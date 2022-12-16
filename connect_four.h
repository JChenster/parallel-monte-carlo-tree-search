#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define COLS (7)
#define ROWS (6)

// Hash function that given a vector which represents state of game
// is able to map it to the pointer of ConnectFourPosition
struct pos_hash {
	size_t operator()(vector<int> const& pos_vec) const {
		size_t seed = pos_vec.size();
		for (int i = 0; i < pos_vec.size(); i++) {
			seed ^= (0x9e3779b9 * i) + pos_vec[i];
		}
		return seed;
	}
};

struct ConnectFourMove: public Move {
	// What column to place chip in
	int col;
	ConnectFourMove(int col): col(col) {};
	void print() override;
};

class ConnectFourPosition: public Position {
	private:
		unordered_map<vector<int>, ConnectFourPosition*, pos_hash>* pos_map;
		// Vector of ints representing columns. Every 2 bits represents a row
		// 0 means empty, 1 means player 0, 2 means player 1
		// Last element is whose turn it is
		vector<int> pos_vec;
		// Helper functions
		int get_slot(int col, int row);
		int check_winner();
	public:
		ConnectFourPosition(unordered_map<vector<int>, ConnectFourPosition*, pos_hash>* pos_map, vector<int> pos_vec);
		bool is_terminal() override;
		// Returns payoff of state (assuming it is terminal)
		float payoff() override;
		// Returns whose turn it is (player 0 or 1)
		int whose_turn() const override;
		// Returns vector of possible moves to make
		vector<Move*> possible_moves() override;
		// Make a move and returns the resulting position
		Position* make_move(Move* move) override;
		// Helper functions
		// For debug
		void print() override;
		void print_pos_map();
};

typedef unordered_map<vector<int>, ConnectFourPosition*, pos_hash> pos_map_t;

struct ConnectFourGame: public Game {
	private:
		pos_map_t* pos_map;
	public:
		ConnectFourGame();
		Position* new_game() override;
};

#endif

