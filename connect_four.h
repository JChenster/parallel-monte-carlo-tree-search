#ifndef CONNECT_FOUR_H
#define CONNECT_FOUR_H

#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define COLS (7)
#define ROWS (6)

struct ConnectFourMove: public Move {
	// What column to place chip in
	int col;
	ConnectFourMove(int col): col(col) {};
	void print() override;
};

class ConnectFourPosition: public Position {
	private:
		// Vector of ints representing columns. Every 2 bits represents a row
		// 0 means empty, 1 means player 0, 2 means player 1
		// Last element is whose turn it is
		vector<int> pos_vec;
		// Helper functions
		int get_slot(int col, int row);
		int check_winner();
	public:
		ConnectFourPosition(vector<int> pos_vec);
		bool is_terminal() override;
		// Returns payoff of state (assuming it is terminal)
		float payoff() override;
		// Returns whose turn it is (player 0 or 1)
		int whose_turn() const override;
		// Returns vector of possible moves to make
		vector<Move*> possible_moves() override;
		// Make a move and returns the resulting position
		Position* make_move(Move* move) override;
		// For hashing
		vector<int> get_vec() override;
		// Helper functions
		// For debug
		void print() override;
};


struct ConnectFourGame: public Game {
	public:
		ConnectFourGame();
		Position* new_game() override;
};

#endif

