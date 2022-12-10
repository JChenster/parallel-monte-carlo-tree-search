#include "fake.h"
	
bool FakePosition::is_terminal() const {
	return false;
}

// Returns payoff of state (assuming it is terminal)
int FakePosition::payoff() const {
	return 0;
}

// Returns whose turn it is (player 0 or 1)
int FakePosition::whose_turn() const {
	return 0;
}

// Returns vector of possible moves to make
vector<Position*> FakePosition::possible_moves() const {
	return vector<Position*>();
}

// Make a move and returns the resulting position
Position* FakePosition::make_move(Move* move) const {
	FakePosition* new_fp = new FakePosition();
	return new_fp;
}


