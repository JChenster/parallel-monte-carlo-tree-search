CC=g++ -fopenmp
FLAGS=-O2 -std=c++11 -g

BINARIES=mcts_connect_four

timing.o: timing.cpp timing.h
	$(CC) $(FLAGS) -c timing.cpp

mcts_serial.o: mcts_serial.cpp mcts_serial.h game.h
	$(CC) $(FLAGS) -c $<

mcts_leaf_parallel.o: mcts_leaf_parallel.cpp mcts_leaf_parallel.h game.h
	$(CC) $(FLAGS) -c $<

mcts_root_parallel.o: mcts_root_parallel.cpp mcts_root_parallel.h game.h
	$(CC) $(FLAGS) -c $<

mcts_tgm_parallel.o: mcts_tgm_parallel.cpp mcts_tgm_parallel.h game.h
	$(CC) $(FLAGS) -c $<

mcts_tnm_parallel.o: mcts_tnm_parallel.cpp mcts_tnm_parallel.h game.h
	$(CC) $(FLAGS) -c $<


mcts_connect_four: main.cpp connect_four.cpp connect_four.h timing.o mcts_serial.o mcts_leaf_parallel.o mcts_root_parallel.o mcts_tgm_parallel.o mcts_tnm_parallel.o
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm $(BINARIES) *.o *gch 2> /dev/null

