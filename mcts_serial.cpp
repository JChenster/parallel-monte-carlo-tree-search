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

// Assumes that caller has at least one child
MctsNode* MctsNode::select_first_child() {
	return this->get_children()[0].first;
}

MctsAgentSerial::MctsAgentSerial() {
	cout << "Creating agent" << endl;
}

// time_limit is in seconds
Move* MctsAgentSerial::best_move(Position* p, double time_limit) {
	auto start = chrono::steady_clock::now(); 
	
	// Look up node in search tree or create new one
	MctsNode* pos_node;
	if (pos_map.find(p) != pos_map.end()) {
		pos_node = pos_map.find(p)->second;
	} else {
		pos_node = new MctsNode(p);
		pos_map.insert(make_pair(p, pos_node));
	}
	
	long int elapsed = 0;
	double time_limit_ms = time_limit * 1000;
	// Continue search algorithm while time_limit is not complete
	while (elapsed < time_limit_ms) {
		// Start at base node
		MctsNode* leaf_node = pos_node;
		vector<MctsNode*> path;
		path.push_back(pos_node);

		// Traverse tree until we reach a leaf by picking child with highest UCB
		while (!leaf_node->is_leaf()) {
			leaf_node = leaf_node->select_child();
			path.push_back(leaf_node);
		}

		// We have now reached leaf
		// If game over, we have reached terminal node
		Position* final_pos = leaf_node->get_pos();
		// If not game over, then we need to expand and rollout
		if (!leaf_node->get_pos()->is_terminal()) {
			// Get the node to rollout from
			MctsNode* playout_node;
			if (leaf_node->get_visits() == 0) {
				playout_node = leaf_node;
			} else {
				leaf_node->expand(pos_map);
				playout_node = leaf_node->select_first_child();
				path.push_back(playout_node);
			}
			Position* curr_pos = playout_node->get_pos(); 
			// Rollout
			while (!curr_pos->is_terminal()) {
				vector<Move*> poss_moves = curr_pos->possible_moves();
				Move* next_move = poss_moves[rand() % poss_moves.size()];
				curr_pos = curr_pos->make_move(next_move);
			}
			final_pos = curr_pos;
		}

		// Back propagate
		for (int i = 0; i < path.size(); i++) {
			MctsNode* node = path[i];
			Position* node_pos = node->get_pos();
			node->inc_visits(1);
			if (i != 0) {
				node->inc_reward(final_pos->payoff());
				// Update edge reward and visits
				for (child_info child: node->get_children()) {
					if (child.first == path[i-1]) {
						child.second.first += final_pos->payoff();
						child.second.second += 1;
					}
				}
			}
		}

		auto curr = chrono::steady_clock::now();
		elapsed = chrono::duration_cast<chrono::milliseconds>(curr - start).count();
	}

	// Choose best action
	double max_ratio = -INFINITY;
	Move* best_move = NULL;
	for (Move* move: p->possible_moves()) {
		Position* next_pos = p->make_move(move);
		MctsNode* next_node = pos_map.find(next_pos)->second;
		if (next_node->get_visits() == 0) {
			continue;
		}
		double curr_ratio = (double) next_node->get_reward() / (double) next_node->get_visits();
		// Player 1 wants least number of wins for player 0
		if (p->whose_turn() == 1) {
			curr_ratio *= -1.0;
		}
		if (curr_ratio > max_ratio) {
			max_ratio = curr_ratio;
			best_move = move;
		}
	}
	return best_move;
}

