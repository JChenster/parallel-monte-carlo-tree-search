#include "connect_four.h"

void ConnectFourMove::print() {
	cout << "ConnectFourMove(" << col << ")" << endl;
}

// Returns 0 if slot is empty, 1 if has player 0 chip, 2 if has player 1 chip
int ConnectFourPosition::get_slot(int col, int row) {
	return (pos_vec[col] & (3 << (2*row))) >> (2*row);
}

ConnectFourPosition::ConnectFourPosition(vector<int> pos_vec): 
	pos_vec(pos_vec) {}

// Returns -1 if no winner
// Otherwise, 0 or 1 for which player wins
int ConnectFourPosition::check_winner() {
	for (int player = 0; player <= 1; player++) {
		// Horizontal
		for (int c = 0; c < COLS-3; c++) {
			for (int r = 0; r < ROWS; r++) {
				if (
					this->get_slot(c, r) == player+1 &&
					this->get_slot(c+1, r) == player+1 &&
					this->get_slot(c+2, r) == player+1 &&
					this->get_slot(c+3, r) == player+1
				){
					return player;
				}
			}
		}
		// Vertical
		for (int c = 0; c < COLS; c++) {
			for (int r = 0; r < ROWS-3; r++) {
				if (
					this->get_slot(c, r) == player+1 &&
					this->get_slot(c, r+1) == player+1 &&
					this->get_slot(c, r+2) == player+1 &&
					this->get_slot(c, r+3) == player+1
				){
					return player;
				}
			}
		}
		// Positive slope diagonal
		for (int c = 0; c < COLS-3; c++) {
			for (int r = 0; r < ROWS-3; r++) {
				if (
					this->get_slot(c, r) == player+1 &&
					this->get_slot(c+1, r+1) == player+1 &&
					this->get_slot(c+2, r+1) == player+1 &&
					this->get_slot(c+3, r+1) == player+1
				){
					return player;
				}
			}
		}
		// Negative slope diagonals
		for (int c = 0; c < COLS-3; c++) {
			for (int r = 3; r < ROWS; r++) {
				if (
					this->get_slot(c, r) == player+1 &&
					this->get_slot(c+1, r-1) == player+1 &&
					this->get_slot(c+2, r-2) == player+1 &&
					this->get_slot(c+3, r-3) == player+1
				){
					return player;
				}
			}
		}
	}
	// No winner
	return -1;
}

// Hash function for a position
vector<int> ConnectFourPosition::get_vec() {
	return pos_vec;
}

bool ConnectFourPosition::is_terminal() {
	// Winner exists
	if (this->check_winner() != -1) {
		return true;
	}
	// End of game if board is full
	for (int c = 0; c < COLS; c++) {
		// Go from top down
		for (int r = ROWS-1; r >= 0; r--) {
			// Empty slot so not end of game yet
			if (get_slot(c, r) == 0) {
				return false;
			}
		}
	}
	return true;
}

// Returns payoff of state (assuming it is terminal)
float ConnectFourPosition::payoff() {
	int check_win = this->check_winner();
	// Perspective of player 0
	if (check_win == 0) {
		return 1;
	} else if (check_win == 1) {
		return 0;
	}
	// Tie
	return 0.5;
}

// Returns whose turn it is (player 0 or 1)
int ConnectFourPosition::whose_turn() const {
	return pos_vec[COLS];
}

// Returns vector of possible moves to make
vector<Move*> ConnectFourPosition::possible_moves() {
	vector<Move*> moves;
	for (int c = 0; c < COLS; c++) {
		// Empty at least at very top
		if(this->get_slot(c, ROWS-1) == 0) {
			ConnectFourMove* new_move = new ConnectFourMove(c);
			moves.push_back(new_move);
		}
	}
	return moves;
}

// Make a move and returns the resulting position
Position* ConnectFourPosition::make_move(Move* move) {
	// Must explicitly cast to ConnectFourMove
	ConnectFourMove* cfmove = (ConnectFourMove*) move;
	// Copy current position vec
	vector<int> new_vec = pos_vec;
	// Switch whose turn it is
	new_vec[COLS] = 1 - this->whose_turn();
	int new_val = new_vec[cfmove->col];
	// Modify it
	for (int r = 0; r < ROWS; r++) {
		if (get_slot(cfmove->col, r) == 0) {
			new_val |= ((this->whose_turn() + 1) << (2*r));
			// No more modifications
			break;
		}
	}
	new_vec[cfmove->col] = new_val;
	ConnectFourPosition* new_pos = new ConnectFourPosition(new_vec);
	return new_pos;
}

void ConnectFourPosition::print() {
	cout << "Turn: " << this->whose_turn() << endl;
	for (int r = ROWS-1; r >= 0; r--) {
		for (int c = 0; c < COLS; c++) {
			cout << this->get_slot(c, r) << " ";
		}
		cout << endl;
	}
}

ConnectFourGame::ConnectFourGame() {}

Position* ConnectFourGame::new_game() {
	vector<int> new_pos_vec = vector<int> (COLS+1, 0);
	ConnectFourPosition* init_pos = new ConnectFourPosition(new_pos_vec);	
	return init_pos;
}

