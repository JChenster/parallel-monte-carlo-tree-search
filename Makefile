CC=g++ -fopenmp
FLAGS=-O2 -std=c++11

BINARIES=connect_four

mcts_serial.o: mcts_serial.cpp mcts_serial.h game.h
	$(CC) $(FLAGS) -c mcts_serial.cpp

connect_four: main.cpp connect_four.cpp connect_four.h mcts_serial.o
	$(CC) $(FLAGS) -o connect_four main.cpp connect_four.cpp mcts_serial.o

clean:
	rm $(BINARIES) *.o *gch

