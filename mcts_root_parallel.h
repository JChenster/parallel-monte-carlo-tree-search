#ifndef MCTS_RP_H
#define MCTS_RP_H

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNodeRootParallel {
	private:
		float reward;
		int visits;
	public:
		Position* pos;
		vector<pair<MctsNodeRootParallel*, pair<float, int>>> children;
		// Functions
		MctsNodeRootParallel(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNodeRootParallel* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNodeRootParallel*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNodeRootParallel*, pair<float, int>> child);
		MctsNodeRootParallel* select_child(unsigned int* seed);
		MctsNodeRootParallel* select_first_child();
};

typedef pair<MctsNodeRootParallel*, pair<float,int>> child_info_rp;
typedef unordered_map<vector<int>, MctsNodeRootParallel*, pos_hash> pos_map_rp_t;

class MctsAgentRootParallel: public Agent {
	public:
		MctsAgentRootParallel();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif

