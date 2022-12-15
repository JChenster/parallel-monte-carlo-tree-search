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
		Position* pos;
		int reward;
		int visits;
		vector<pair<MctsNode*, pair<int, int>>> children;
		
	public:
		MctsNode(Position* p);
		Position* get_pos();
		int get_reward();
		int get_visits();
		vector<pair<MctsNode*, pair<int, int>>> get_children();
		bool is_leaf();
		int get_player();
		void add_child(MctsNode* new_child);
		void inc_reward(int delta);
		void inc_visits(int delta);
		void expand(unordered_map<Position*, MctsNode*>& pos_map);
		double calc_ucb2_child(pair<MctsNode*, pair<int, int>> child);
		MctsNode* select_child();
		MctsNode* select_first_child();
};

typedef pair<MctsNode*, pair<int, int>> child_info;

class MctsAgentSerial {
	private:
		unordered_map<Position*, MctsNode*> pos_map;
	public:
		MctsAgentSerial();
		Move* best_move(Position* p, double time_limit);
};

#endif


