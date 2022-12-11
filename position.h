#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <vector>
using namespace std;

struct Move {
	virtual ~Move() = default;	
};

// Abstract class
class Position {
	public:
		// Returns if is terminal state
		virtual bool is_terminal() const = 0;
		// Returns payoff of state (assuming it is terminal)
		virtual int payoff() const = 0;
		// Returns whose turn it is (player 0 or 1)
		virtual int whose_turn() const = 0;
		// Returns vector of possible moves to make
		virtual vector<Move*> possible_moves() const = 0;
		// Make a move and returns the resulting position
		virtual Position* make_move(Move* move) const = 0;
};

#endif


