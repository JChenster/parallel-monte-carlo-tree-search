#ifndef MCTS_LP_H
#define MCTS_LP_H

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNodeLeafParallel {
	private:
		float reward;
		int visits;
	public:
		Position* pos;
		vector<pair<MctsNodeLeafParallel*, pair<float, int>>> children;
		// Functions
		MctsNodeLeafParallel(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNodeLeafParallel* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNodeLeafParallel*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNodeLeafParallel*, pair<float, int>> child);
		MctsNodeLeafParallel* select_child();
		MctsNodeLeafParallel* select_first_child();
};

typedef pair<MctsNodeLeafParallel*, pair<float,int>> child_info_lp;
typedef unordered_map<vector<int>, MctsNodeLeafParallel*, pos_hash> pos_map_lp_t;

class MctsAgentLeafParallel: public Agent {
	private:
		pos_map_lp_t pos_map;
	public:
		MctsAgentLeafParallel();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif


