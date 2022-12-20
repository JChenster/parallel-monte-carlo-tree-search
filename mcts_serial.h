#ifndef MCTS_SERIAL_H
#define MCTS_SERIAL_H

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNodeSerial {
	private:
		float reward;
		int visits;
	public:
		Position* pos;
		vector<pair<MctsNodeSerial*, pair<float, int>>> children;
		// Functions
		MctsNodeSerial(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNodeSerial* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNodeSerial*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNodeSerial*, pair<float, int>> child);
		MctsNodeSerial* select_child();
		MctsNodeSerial* select_first_child();
};

typedef pair<MctsNodeSerial*, pair<float,int>> child_info;
typedef unordered_map<vector<int>, MctsNodeSerial*, pos_hash> pos_map_t;

class MctsAgentSerial: public Agent {
	private:
		pos_map_t pos_map;
	public:
		MctsAgentSerial();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif

