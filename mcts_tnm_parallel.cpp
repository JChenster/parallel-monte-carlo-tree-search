#include <stdlib.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#include "timing.h"
#include "mcts_tnm_parallel.h"

MctsNodeTnmParallel::MctsNodeTnmParallel(Position* p): 
	pos(p), reward(0), visits(0), children(vector<child_info_tnm>()), am_leaf(true) {
	omp_init_lock(&node_mutex);
}

void MctsNodeTnmParallel::lock() {
	omp_set_lock(&node_mutex);
}

void MctsNodeTnmParallel::unlock() {
	omp_unset_lock(&node_mutex);
}

// Accessor functions
float MctsNodeTnmParallel::get_reward() {
	return reward;
}

int MctsNodeTnmParallel::get_visits() {
	return visits;
}

bool MctsNodeTnmParallel::is_leaf() {
	return am_leaf;
}

void MctsNodeTnmParallel::add_child(MctsNodeTnmParallel* new_child) {
	children.push_back(make_pair(new_child, make_pair(0.0, 0)));
}

void MctsNodeTnmParallel::inc_reward(float delta) {
	reward += delta;
}

void MctsNodeTnmParallel::inc_visits(float delta) {
	visits += delta;
}

void MctsNodeTnmParallel::expand(pos_map_tnm_t* pos_map) {
	// Get next possible moves
	vector<Move*> next_moves = pos->possible_moves();
	for (Move* move: next_moves) {
		// See subsequent positions and either locate corresponding node already in tree
		// or insert into tree
		Position* new_pos = pos->make_move(move);
		if (pos_map->find(new_pos->get_vec()) == pos_map->end()) {
			MctsNodeTnmParallel* new_child = new MctsNodeTnmParallel(new_pos);
			pos_map->insert(make_pair(new_pos->get_vec(), new_child));
		}
		// Add subsequent node as child of current node
		this->add_child(pos_map->find(new_pos->get_vec())->second);
	}
	// Update
	am_leaf = false;
}

float MctsNodeTnmParallel::calc_ucb2_child(child_info_tnm child, int parent_visits){
	int edge_visits = child.second.second;	
	MctsNodeTnmParallel* child_node = child.first;
	// Lock child node
	child_node->lock();

	// If node has never been visited before
	if (edge_visits == 0 || child_node->get_visits() == 0) {
		child_node->unlock();	
		return INFINITY;
	}
	float exploit = (float) child_node->get_reward() / (float) child_node->get_visits();
	float explore = sqrt(UCB_CONSTANT * log(parent_visits) / edge_visits);
	// Player 0 views results favorably while player 1 wants to negate it
	if (child_node->pos->whose_turn() == 0) {
		return exploit + explore;
	}
	// Done with child node
	child_node->unlock();

	return -1.0 * exploit + explore;
}

// Calculate UCB for each node
// Return the node that maximizes UCB
MctsNodeTnmParallel* MctsNodeTnmParallel::select_child(unsigned int* seed) {
	float max_ucb = -INFINITY;
	vector<MctsNodeTnmParallel*> optimal_children;
	this->lock();
	int my_visits = this->get_visits();
	vector<child_info_tnm> curr_children = this->children;
	this->unlock();

	for (child_info_tnm child: curr_children) {
		float child_ucb = this->calc_ucb2_child(child, my_visits);
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

	int rand_idx = rand_r(seed) % optimal_children.size();
	return optimal_children[rand_idx];
}

// Assumes that caller has at least one child
MctsNodeTnmParallel* MctsNodeTnmParallel::select_first_child() { 
	MctsNodeTnmParallel* ret = this->children[0].first;
	return ret;
}

MctsAgentTnmParallel::MctsAgentTnmParallel() {}

// time_limit is in seconds
pair<Move*,int> MctsAgentTnmParallel::best_move(Position* p, float time_limit) {
	double wc_time, cpu_time;
	timing(&wc_time, &cpu_time);
	double start = wc_time;

	// Look up node in search tree or create new one
	MctsNodeTnmParallel* pos_node;
	if (pos_map.find(p->get_vec()) != pos_map.end()) {
		pos_node = pos_map.find(p->get_vec())->second;
	} else {
		pos_node = new MctsNodeTnmParallel(p);
		pos_map.insert(make_pair(p->get_vec(), pos_node));
	}

	int iterations = 0;
	#pragma omp parallel \
		shared(start, time_limit, iterations, pos_node) \
		private(wc_time, cpu_time) \
		default(none)
	{
		// Each thread get its own seed to generate random numbers with
		unsigned int seed = omp_get_thread_num();		
		
		double elapsed = 0.0;
		int my_iterations = 0;
		
		// Continue search algorithm while time_limit is not complete
		while (elapsed < time_limit) {
			// Start at base node
			MctsNodeTnmParallel* leaf_node = pos_node;
			vector<MctsNodeTnmParallel*> path;
			path.push_back(pos_node);

			// Traverse tree until we reach a leaf by picking child with highest UCB
			while (!leaf_node->is_leaf()) {
				leaf_node = leaf_node->select_child(&seed);
				path.push_back(leaf_node);
			}

			float rollout_reward;
			int rollout_visits = 1;

			Position* curr_pos;
			// We have now reached leaf
			// If game over, we have reached terminal node
			if (leaf_node->pos->is_terminal()) {
				curr_pos = leaf_node->pos;
			}
			// If not game over, then we need to expand and rollout
			else {
				// Get the node to rollout from
				MctsNodeTnmParallel* playout_node;
				leaf_node->lock();
				if (leaf_node->get_visits() == 0) {
					playout_node = leaf_node;
				} else {
					leaf_node->expand(&pos_map);
					playout_node = leaf_node->select_first_child();
					path.push_back(playout_node);
				}
				leaf_node->unlock();
				curr_pos = playout_node->pos; 	
			}

			// Rollout phase can be done without access to tree
			while (!curr_pos->is_terminal()) {
				vector<Move*> poss_moves = curr_pos->possible_moves();
				Move* next_move = poss_moves[rand_r(&seed) % poss_moves.size()];
				curr_pos = curr_pos->make_move(next_move);
			}
			// Now at terminal state
			rollout_reward = curr_pos->payoff();
			my_iterations++;

			// Back propagate
			for (int i = 0; i < path.size(); i++) {
				MctsNodeTnmParallel* node = path[i];
				node->lock();
				//printf("Path[%d] = %p\n", i, node);
				Position* node_pos = node->pos;
				node->inc_visits(rollout_visits);
				node->inc_reward(rollout_reward);
				// Update edge info
				if (i != path.size() - 1) {
					// Update edge reward and visits
					for (int j = 0; j < node->children.size(); j++) {
						// This is the edge we traversed
						if (node->children[j].first == path[i+1]) {
							node->children[j].second.first += rollout_reward;
							node->children[j].second.second += rollout_visits;
							break;
						}
					}
				}
				node->unlock();
			}

			// Update elapsed time
			timing(&wc_time, &cpu_time);
			elapsed = wc_time - start;
		}
		#pragma omp atomic update
		iterations += my_iterations;
	}

	// Parallel section finished
	// Can now serially safely access tree results
	// Choose best action
	float max_ratio = -INFINITY;
	Move* best_move = NULL;
	for (Move* move: p->possible_moves()) {
		Position* next_pos = p->make_move(move);
		MctsNodeTnmParallel* next_node = pos_map.find(next_pos->get_vec())->second;	
		// printf("%p: (%f, %d)\n", next_node, next_node->get_reward(), next_node->get_visits());
		// move->print();
		if (next_node->get_visits() == 0) {
			continue;
		}
		float curr_ratio = (float) next_node->get_reward() / (float) next_node->get_visits();
		// Player 1 wants least number of wins for player 0
		if (p->whose_turn() == 1) {
			curr_ratio *= -1.0;
		}
		if (curr_ratio > max_ratio) {
			max_ratio = curr_ratio;
			best_move = move;
		}
	}
	return make_pair(best_move, iterations);
}

void MctsAgentTnmParallel::reset() {
	for (auto it: pos_map) {
		// Delete node
		delete it.second;
	}
	pos_map.clear();
}

