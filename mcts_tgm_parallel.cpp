#include <stdlib.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#include "timing.h"
#include "mcts_tgm_parallel.h"

MctsNodeTgmParallel::MctsNodeTgmParallel(Position* p): pos(p), reward(0), visits(0), children(vector<child_info_tgm>()) {}

// Accessor functions
float MctsNodeTgmParallel::get_reward() {
	return reward;
}

int MctsNodeTgmParallel::get_visits() {
	return visits;
}

bool MctsNodeTgmParallel::is_leaf() {
	return children.empty();
}

void MctsNodeTgmParallel::add_child(MctsNodeTgmParallel* new_child) {
	children.push_back(make_pair(new_child, make_pair(0.0, 0)));
}

void MctsNodeTgmParallel::inc_reward(float delta) {
	reward += delta;
}

void MctsNodeTgmParallel::inc_visits(float delta) {
	visits += delta;
}

void MctsNodeTgmParallel::expand(pos_map_tgm_t* pos_map) {
	// Get next possible moves
	vector<Move*> next_moves = pos->possible_moves();
	for (Move* move: next_moves) {
		// See subsequent positions and either locate corresponding node already in tree
		// or insert into tree
		Position* new_pos = pos->make_move(move);
		if (pos_map->find(new_pos->get_vec()) == pos_map->end()) {
			MctsNodeTgmParallel* new_child = new MctsNodeTgmParallel(new_pos);
			pos_map->insert(make_pair(new_pos->get_vec(), new_child));
		}
		// Add subsequent node as child of current node
		this->add_child(pos_map->find(new_pos->get_vec())->second);
	}
}

float MctsNodeTgmParallel::calc_ucb2_child(child_info_tgm child){
	int edge_visits = child.second.second;
	MctsNodeTgmParallel* child_node = child.first;
	// If node has never been visited before
	if (edge_visits == 0 || child_node->get_visits() == 0) {
		return INFINITY;
	}
	float exploit = (float) child_node->get_reward() / (float) child_node->get_visits();
	float explore = sqrt(UCB_CONSTANT * log(this->get_visits()) / edge_visits);
	// Player 0 views results favorably while player 1 wants to negate it
	if (child_node->pos->whose_turn() == 0) {
		return exploit + explore;
	} 
	return -1.0 * exploit + explore;
}

// Calculate UCB for each node
// Return the node that maximizes UCB
MctsNodeTgmParallel* MctsNodeTgmParallel::select_child(unsigned int* seed) {
	float max_ucb = -INFINITY;
	vector<MctsNodeTgmParallel*> optimal_children;
	for (child_info_tgm child: this->children) {
		float child_ucb = this->calc_ucb2_child(child);
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
MctsNodeTgmParallel* MctsNodeTgmParallel::select_first_child() {
	return this->children[0].first;
}

MctsAgentTgmParallel::MctsAgentTgmParallel() {
	omp_init_lock(&tree_mutex);
}

// time_limit is in seconds
pair<Move*,int> MctsAgentTgmParallel::best_move(Position* p, float time_limit) {
	double wc_time, cpu_time;
	timing(&wc_time, &cpu_time);
	double start = wc_time;

	// Look up node in search tree or create new one
	MctsNodeTgmParallel* pos_node;
	if (pos_map.find(p->get_vec()) != pos_map.end()) {
		pos_node = pos_map.find(p->get_vec())->second;
	} else {
		pos_node = new MctsNodeTgmParallel(p);
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
			MctsNodeTgmParallel* leaf_node = pos_node;
			vector<MctsNodeTgmParallel*> path;
			path.push_back(pos_node);

			// Only allow one thread access
			omp_set_lock(&tree_mutex);
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
				MctsNodeTgmParallel* playout_node;
				if (leaf_node->get_visits() == 0) {
					playout_node = leaf_node;
				} else {
					leaf_node->expand(&pos_map);
					playout_node = leaf_node->select_first_child();
					path.push_back(playout_node);
				}
				curr_pos = playout_node->pos; 	
			}
			// Done reading and writing to tree

			// Rollout phase can be done without access to tree
			while (!curr_pos->is_terminal()) {
				vector<Move*> poss_moves = curr_pos->possible_moves();
				Move* next_move = poss_moves[rand_r(&seed) % poss_moves.size()];
				curr_pos = curr_pos->make_move(next_move);
			}
			// Now at terminal state
			rollout_reward = curr_pos->payoff();
			my_iterations++;

			// Need access to tree again
			omp_set_lock(&tree_mutex);
			// Back propagate
			for (int i = 0; i < path.size(); i++) {
				MctsNodeTgmParallel* node = path[i];
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
			}
			// Done with tree
			omp_unset_lock(&tree_mutex);

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
		MctsNodeTgmParallel* next_node = pos_map.find(next_pos->get_vec())->second;	
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

void MctsAgentTgmParallel::reset() {
	for (auto it: pos_map) {
		// Delete node
		delete it.second;
	}
	pos_map.clear();
}

