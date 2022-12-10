#ifndef MCTS_H
#define MCTS_H

#include <vector>
using namespace std;

#include "position.h"

// Node in computation tree to represent positions
class MctsNode {
	private:
		Position* pos;
		int reward;
		int visits;
		vector<MctsNode*> children;

	public:
		MctsNode(Position* p);
		Position* get_pos();
		int get_reward();
		int get_visits();
		vector<MctsNode*> get_children();
		bool is_leaf();
		int get_player();
		void add_child(MctsNode* new_child);
		void inc_reward(int delta);
		void inc_visits(int delta);
		MctsNode* expand();
		float calc_ucb2_child(MctsNode* child);
		MctsNode* select_child();
		MctsNode* select_first_child();
};

class MctsAgent {
	public:
		MctsAgent();
		int best_move(Position* p);
};

#endif


