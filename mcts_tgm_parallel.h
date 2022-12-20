#ifndef MCTS_TGM_H
#define MCTS_TGM_H

#include <omp.h>

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNodeTgmParallel {
	private:
		float reward;
		int visits;
	public:
		Position* pos;
		vector<pair<MctsNodeTgmParallel*, pair<float, int>>> children;
		// Functions
		MctsNodeTgmParallel(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNodeTgmParallel* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNodeTgmParallel*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNodeTgmParallel*, pair<float, int>> child);
		MctsNodeTgmParallel* select_child(unsigned int* seed);
		MctsNodeTgmParallel* select_first_child();
};

typedef pair<MctsNodeTgmParallel*, pair<float,int>> child_info_tgm;
typedef unordered_map<vector<int>, MctsNodeTgmParallel*, pos_hash> pos_map_tgm_t;

class MctsAgentTgmParallel: public Agent {
	private:
		pos_map_tgm_t pos_map;
		omp_lock_t tree_mutex;
	public:
		MctsAgentTgmParallel();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif

