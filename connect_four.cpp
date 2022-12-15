#include "connect_four.h"
	
bool ConnectFourPosition::is_terminal() const {
	return true;
}

// Returns payoff of state (assuming it is terminal)
int ConnectFourPosition::payoff() const {
	return 0;
}

// Returns whose turn it is (player 0 or 1)
int ConnectFourPosition::whose_turn() const {
	return 0;
}

// Returns vector of possible moves to make
vector<Move*> ConnectFourPosition::possible_moves() const {
	return vector<Move*>();
}

// Make a move and returns the resulting position
Position* ConnectFourPosition::make_move(Move* move) const {
	ConnectFourPosition* new_fp = new ConnectFourPosition();
	return new_fp;
}


