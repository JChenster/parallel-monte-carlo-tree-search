# Parallel Monte Carlo Tree Search

## Introduction
Monte Carlo Tree Search (MCTS) is a search algorithm utilized in artificial intelligence to pick an optimal move given a starting position in two-player turn-based games with perfect information. The algorithm works by running as many iterations as it can in a certain amount of time in which it explores the game tree in an asymmetric fashion and simulates as many games as it can. 
MCTS is composed of several phases in each iteration: tree traversal, leaf expansion, leaf rollout, and backpropagation. In my project, I aim to parallelize MCTS using C++ using OpenMP. I aim to answer the questions of how much better parallel MCTS is over serial MCTS and if it makes a difference to parallelize different phases of the algorithm such as tree traversal, leaf rollout, or the iterations. 
I derive inspiration for parallelization approaches from an existing paper, [Parallel Monte-Carlo Tree Search (Chaslot et al.)](https://dke.maastrichtuniversity.nl/m.winands/documents/multithreadedMCTS2.pdf)

## Code
The way that I utilize classes allows me to run my programs on any games provided that C++ code is written that implements the abstract `Move` and `Position` classes in my `game.h` file. My `connect_four.cpp` and `connect_four.h` are an example.

I also wrote a class for MCTS agents. I wrote my serial program and various parallel implementations in different files: `mcts_serial.cpp`, `mcts_leaf_parallel.cpp`, `mcts_root_parallel.cpp`, `mcts_tgm_parallel.cpp`, `mcts_tnm_parallel.cpp`. 

I wrote a main.cpp program that takes in the following command line arguments:

```./mcts_connect_four <Agent 1> <Agent 2> <Test games> <Epsilon> <Time limit>```

where valid agents are `serial`, `leaf`, `root`, `tgm`, `tnm` to represent my different implementations for MCTS agents as well as random which is a benchmark agent that simply picks a random move given a position.

The main program serves as a testing program for the effectiveness of each MCTS agent. It simulates `<Test games>` number of games of Connect Four where for each move, the agent whose turn it is plays does MCTS for `<Time limit>` number of seconds with probability `<Epsilon>` and plays randomly otherwise. `<Epsilon>` is chosen to be a number between 0 and 1, typically on the smaller side, so we allow agents to pick moves randomly a large percentage of the time, resulting in more positions that can be reached (which an agent may have otherwise avoided), thus testing our agent’s decision making abilities in different positions.

My code is compiled with a Makefile and I have Bash test scripts for different implementations, which are intended to be used with a Slurm task manager. I use `g++` compiler on the `C++11` standard with flag `-fopenmp` to use OpenMP.


###  Leaf Rollout Parallelization
We can observe that in the rollout phase, the game tree is not used at all. We simply take the current position and pick random moves until we reach a terminal state, interacting mostly with the class that represents a game as opposed to the game tree.
The leaf parallelization approach takes advantage of this by parallelizing the rollout process by having multiple threads conduct rollout from the current position.

### Root Tree Generation Parallelization
This parallelization approach circumvents the issue of multiple threads accessing the same game tree by having each thread build its own game tree. Then once the allotted time period for MCTS is up, we aggregate the expected values calculated in all the game trees for neighbors of the roots in order to obtain collective expected values and choose the best move according to that. It is called root parallelization because we only need to synchronize the expected values of the children of the root of the subtree we are interested in since we only have to make a decision about what move to make at that particular position.

### Tree Global Mutex (TGM) Parallelization
In the previous two parallelization approaches, we did not take advantage of mutexes aka. locks, which are one of the powerful tools we learned about this semester. Naturally, in order to make multiple threads accessing the game tree safe, we can use a lock on the game tree. Recall that the phases of one MCTS iteration are traversal, expansion, rollout, and backpropagation in that order. The game tree is read and written to in the traversal, expansion, and backpropagation phases so we place a lock during the traversal and expansion phases and then unlock it during rollout. We pick up the lock again after backpropagation and then release it again. 

### Tree Node Mutex (TNM) Parallelization
The major inefficiency in having a global mutex for our game tree is that oftentimes, a thread will only be working with a small portion of the game tree, traversing down paths and positions that may be entirely disjoint from what another thread is doing. It would be nice therefore for multiple threads to access the game tree as long as they are operating on different nodes. In this approach, I initialized an OpenMP lock for every single node. Whenever a thread reads or writes to a node, it locks it and unlocks it when done. In order to make this possible, I had to rewrite a lot of code in order to prevent deadlocks. I ensured that each thread will only attempt to gain access to a lock when it currently holds no locks. This ensures that no thread is too greedy. 
