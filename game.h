#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
using namespace std;

struct Move {
	virtual ~Move() = default;	
	virtual void print() = 0;
};

// Abstract class
class Position {
	public:
		// Returns if is terminal state
		virtual bool is_terminal() = 0;
		// Returns payoff of state (assuming it is terminal)
		virtual float payoff() = 0;
		// Returns whose turn it is (player 0 or 1)
		virtual int whose_turn() const = 0;
		// Returns vector of possible moves to make
		virtual vector<Move*> possible_moves() = 0;
		// Make a move and returns the resulting position
		virtual Position* make_move(Move* move) = 0;
		virtual void print() = 0;
};

class Game {
	public:
		virtual Position* new_game() = 0;	
};

class Agent {
	public:
		virtual Move* best_move(Position* pos, float time_limit) = 0; 
		virtual void reset() = 0;
};

#endif

