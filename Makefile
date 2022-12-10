CC=g++ -fopenmp
FLAGS=-O2 -std=c++11

BINARIES=test

mcts_serial.o: mcts_serial.cpp mcts_serial.h position.h
	$(CC) $(FLAGS) -c mcts_serial.cpp

fake: main.cpp fake.cpp fake.h mcts_serial.o
	$(CC) $(FLAGS) -o fake main.cpp fake.cpp mcts_serial.o

clean:
	rm $(BINARIES) *.o *gch

