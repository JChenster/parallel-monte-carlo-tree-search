#ifndef MCTS_TNM_H
#define MCTS_TNM_H

#include <omp.h>

#include <cstdlib>
#include <unordered_map>
#include <vector>
using namespace std;

#include "game.h"

#define UCB_CONSTANT (2)

// Node in computation tree to represent positions
class MctsNodeTnmParallel {
	private:
		float reward;
		int visits;
		omp_lock_t node_mutex;
		bool am_leaf;
	public:
		Position* pos;
		vector<pair<MctsNodeTnmParallel*, pair<float, int>>> children;
		// Functions
		void lock();
		void unlock();
		MctsNodeTnmParallel(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNodeTnmParallel* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<vector<int>, MctsNodeTnmParallel*, pos_hash>* pos_map);
		float calc_ucb2_child(pair<MctsNodeTnmParallel*, pair<float, int>> child, int parent_visits);
		MctsNodeTnmParallel* select_child(unsigned int* seed);
		MctsNodeTnmParallel* select_first_child();
};

typedef pair<MctsNodeTnmParallel*, pair<float,int>> child_info_tnm;
typedef unordered_map<vector<int>, MctsNodeTnmParallel*, pos_hash> pos_map_tnm_t;

class MctsAgentTnmParallel: public Agent {
	private:
		pos_map_tnm_t pos_map;
	public:
		MctsAgentTnmParallel();
		pair<Move*,int> best_move(Position* p, float time_limit);
		void reset();
};

#endif

