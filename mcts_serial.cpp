#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#include "mcts_serial.h"

MctsNode::MctsNode(Position* p): pos(p), reward(0), visits(0), children(vector<child_info>()) {}

// Accessor functions
Position* MctsNode::get_pos() {
	return pos;
}

int MctsNode::get_reward() {
	return reward;
}

int MctsNode::get_visits() {
	return visits;
}

vector<child_info> MctsNode::get_children() {
	return children;
}

bool MctsNode::is_leaf() {
	return children.empty();
}

void MctsNode::add_child(MctsNode* new_child) {
	children.push_back(make_pair(new_child, make_pair(0, 0)));
}

void MctsNode::inc_reward(int delta) {
	reward += delta;
}

void MctsNode::inc_visits(int delta) {
	visits += delta;
}

void MctsNode::expand(unordered_map<Position*, MctsNode*>& pos_map) {
	// Get next possible moves
	vector<Move*> next_moves = pos->possible_moves();
	for (Move* move: next_moves) {
		// See subsequent positions and either locate corresponding node already in tree
		// or insert into tree
		Position* new_pos = pos->make_move(move);
		if (pos_map.find(new_pos) == pos_map.end()) {
			MctsNode* new_child = new MctsNode(new_pos);
			pos_map.insert(make_pair(new_pos, new_child));
		}
		// Add subsequent node as child of current node
		this->add_child(pos_map.find(new_pos)->second);
		// Initialize edge between current node and subsequent node
	}
}

double MctsNode::calc_ucb2_child(child_info child){
	int edge_visits = child.second.second;
	MctsNode* child_node = child.first;
	// If node has never been visited before
	if (edge_visits == 0 || child_node->get_visits() == 0) {
		return INFINITY;
	}
	double exploit = (double) child_node->get_reward() / (double) child_node->get_visits();
	double explore = sqrt(UCB_CONSTANT * log(this->get_visits()) / edge_visits);
	// Player 0 views results favorably while player 1 wants to negate it
	if (child_node->get_pos()->whose_turn() == 0) {
		return exploit + explore;
	} 
	return -1.0 * exploit + explore;
}

// Calculate UCB for each node
// Return the node that maximizes UCB
MctsNode* MctsNode::select_child() {
	double max_ucb = -INFINITY;
	vector<MctsNode*> optimal_children;
	for (child_info child: this->get_children()) {
		double child_ucb = this->calc_ucb2_child(child);
		if (child_ucb == INFINITY) {
			return child.first;
		}
		if (child_ucb > max_ucb) {
			max_ucb = child_ucb;
			optimal_children.clear();
		}
		if (child_ucb == max_ucb) {
			optimal_children.push_back(child.first);
		}
	}
	int rand_idx = rand() % optimal_children.size();
	return optimal_children[rand_idx];
}

MctsAgent::MctsAgent() {
	cout << "Creating agent" << endl;
}

int MctsAgent::best_move(Position* p) {
	return 0;	
}

