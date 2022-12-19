#ifndef MCTS_LP_H
#define MCTS_LP_H

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNode {
	private:
		float reward;
		int visits;
	public:
		Position* pos;
		vector<pair<MctsNode*, pair<float, int>>> children;
		// Functions
		MctsNode(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNode* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNode*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNode*, pair<float, int>> child);
		MctsNode* select_child();
		MctsNode* select_first_child();
};

typedef pair<MctsNode*, pair<float,int>> child_info;
typedef unordered_map<vector<int>, MctsNode*, pos_hash> pos_map_t;

class MctsAgentLeafParallel: public Agent {
	private:
		pos_map_t pos_map;
	public:
		MctsAgentLeafParallel();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif


