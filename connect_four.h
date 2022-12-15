#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include "position.h"

struct ConnectFourMove: public Move {
	int info;
	ConnectFourMove(int info): info(info) {}
};

class ConnectFourPosition: public Position {
	public:
		bool is_terminal() const override;
		// Returns payoff of state (assuming it is terminal)
		int payoff() const override;
		// Returns whose turn it is (player 0 or 1)
		int whose_turn() const override;
		// Returns vector of possible moves to make
		vector<Move*> possible_moves() const override;
		// Make a move and returns the resulting position
		Position* make_move(Move* move) const override;
};

#endif

