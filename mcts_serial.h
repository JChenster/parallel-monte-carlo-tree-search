#ifndef MCTS_H
#define MCTS_H

#include <bits/stdc++.h>
#include <chrono>
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
		vector<pair<MctsNode*, pair<int, int>>> children;
		// Functions
		MctsNode(Position* p);
		float get_reward();
		int get_visits();
		bool is_leaf();
		int get_player();
		void add_child(MctsNode* new_child);
		void inc_reward(float delta);
		void inc_visits(float delta);
		void expand(unordered_map<Position*, MctsNode*>* pos_map);
		float calc_ucb2_child(pair<MctsNode*, pair<int, int>> child);
		MctsNode* select_child();
		MctsNode* select_first_child();
};

typedef pair<MctsNode*, pair<int, int>> child_info;

class MctsAgentSerial: public Agent {
	private:
		unordered_map<Position*, MctsNode*> pos_map;
	public:
		MctsAgentSerial();
		Move* best_move(Position* p, float time_limit) override;
		void reset() override;
};

#endif


