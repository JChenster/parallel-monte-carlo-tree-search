#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

// Hash function that given a vector which represents state of game
// is able to map it to the pointer of ConnectFourPosition
struct pos_hash {
	size_t operator()(vector<int> const& pos_vec) const {
		size_t seed = pos_vec.size();
		for (int i: pos_vec) {
			seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

struct ConnectFourMove: public Move {
	// What column to place chip in
	int col;
	ConnectFourMove(int col): col(col) {}
};

class ConnectFourPosition: public Position {
	private:
		unordered_map<vector<int>, ConnectFourPosition*, pos_hash>* pos_map;
		// Which player's turn it is
		int turn;
		// Vector of ints representing columns. Every 2 bits represents a row
		// 0 means empty, 1 means player 0, 2 means player 1
		vector<int> pos_vec;
	public:
		ConnectFourPosition(unordered_map<vector<int>, ConnectFourPosition*, pos_hash>* pos_map);
		bool is_terminal() const override;
		// Returns payoff of state (assuming it is terminal)
		int payoff() const override;
		// Returns whose turn it is (player 0 or 1)
		int whose_turn() const override;
		// Returns vector of possible moves to make
		vector<Move*> possible_moves() const override;
		// Make a move and returns the resulting position
		Position* make_move(Move* move) const override;
		// For debug
		void print_pos_map();
};

typedef unordered_map<vector<int>, ConnectFourPosition*, pos_hash> pos_map_t;

struct ConnectFourGame: public Game {
	private:
		pos_map_t* pos_map;
	public:
		ConnectFourGame();
		ConnectFourPosition* new_game() const override;
};

#endif

