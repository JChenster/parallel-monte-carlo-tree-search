#include <omp.h>
#include <stdlib.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#include "timing.h"
#include "mcts_root_parallel.h"

MctsNodeRootParallel::MctsNodeRootParallel(Position* p): pos(p), reward(0), visits(0), children(vector<child_info_rp>()) {}

// Accessor functions
float MctsNodeRootParallel::get_reward() {
	return reward;
}

int MctsNodeRootParallel::get_visits() {
	return visits;
}

bool MctsNodeRootParallel::is_leaf() {
	return children.empty();
}

void MctsNodeRootParallel::add_child(MctsNodeRootParallel* new_child) {
	children.push_back(make_pair(new_child, make_pair(0.0, 0)));
}

void MctsNodeRootParallel::inc_reward(float delta) {
	reward += delta;
}

void MctsNodeRootParallel::inc_visits(float delta) {
	visits += delta;
}

void MctsNodeRootParallel::expand(pos_map_rp_t* pos_map) {
	// Get next possible moves
	vector<Move*> next_moves = pos->possible_moves();
	for (Move* move: next_moves) {
		// See subsequent positions and either locate corresponding node already in tree
		// or insert into tree
		Position* new_pos = pos->make_move(move);
		if (pos_map->find(new_pos->get_vec()) == pos_map->end()) {
			MctsNodeRootParallel* new_child = new MctsNodeRootParallel(new_pos);
			pos_map->insert(make_pair(new_pos->get_vec(), new_child));
		}
		// Add subsequent node as child of current node
		this->add_child(pos_map->find(new_pos->get_vec())->second);
	}
}

float MctsNodeRootParallel::calc_ucb2_child(child_info_rp child){
	int edge_visits = child.second.second;
	MctsNodeRootParallel* child_node = child.first;
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
MctsNodeRootParallel* MctsNodeRootParallel::select_child(unsigned int* seed) {
	float max_ucb = -INFINITY;
	vector<MctsNodeRootParallel*> optimal_children;
	for (child_info_rp child: this->children) {
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
MctsNodeRootParallel* MctsNodeRootParallel::select_first_child() {
	return this->children[0].first;
}

MctsAgentRootParallel::MctsAgentRootParallel() {}

// time_limit is in seconds
pair<Move*,int> MctsAgentRootParallel::best_move(Position* p, float time_limit) {
	double wc_time, cpu_time;
	timing(&wc_time, &cpu_time);
	double start = wc_time;
	int iterations = 0;

	// Scores of children that we aggregate from parallel processes
	vector<Move*> poss_moves = p->possible_moves();
	vector<Position*> next_positions;
	for (Move* move: poss_moves) {
		next_positions.push_back(p->make_move(move));
	}
	vector<pair<float, int>> scores = vector<pair<float, int>>(poss_moves.size(), make_pair(0.0, 0));

	#pragma omp parallel \
		shared(p, start, iterations, time_limit, scores, next_positions) \
		private(wc_time, cpu_time) \
		default(none)
	{
		// Each thread needs it own tree
		pos_map_rp_t pos_map;
		MctsNodeRootParallel* pos_node = new MctsNodeRootParallel(p);
		pos_map.insert(make_pair(p->get_vec(), pos_node));

		// Each thread should get its own random seed
		unsigned int seed = omp_get_thread_num();

		double elapsed = 0.0;
		int my_iterations = 0;

		// Continue search algorithm while time_limit is not complete
		while (elapsed < time_limit) {
			// Start at base node
			MctsNodeRootParallel* leaf_node = pos_node;
			vector<MctsNodeRootParallel*> path;
			path.push_back(pos_node);

			// Traverse tree until we reach a leaf by picking child with highest UCB
			while (!leaf_node->is_leaf()) {
				leaf_node = leaf_node->select_child(&seed);
				path.push_back(leaf_node);
			}

			float rollout_reward;
			int rollout_visits = 1;

			// We have now reached leaf
			// If game over, we have reached terminal node
			if (leaf_node->pos->is_terminal()) {
				rollout_reward = leaf_node->pos->payoff();
			}
			// If not game over, then we need to expand and rollout
			else {
				// Get the node to rollout from
				MctsNodeRootParallel* playout_node;
				if (leaf_node->get_visits() == 0) {
					playout_node = leaf_node;
				} else {
					leaf_node->expand(&pos_map);
					playout_node = leaf_node->select_first_child();
					path.push_back(playout_node);
				}

				// Rollout
				Position* curr_pos = playout_node->pos; 	
				while (!curr_pos->is_terminal()) {
					vector<Move*> poss_moves = curr_pos->possible_moves();
					Move* next_move = poss_moves[rand_r(&seed) % poss_moves.size()];
					curr_pos = curr_pos->make_move(next_move);
				}
				// Now at terminal state
				rollout_reward = curr_pos->payoff();			
			}

			my_iterations++;

			// Back propagate
			for (int i = 0; i < path.size(); i++) {
				MctsNodeRootParallel* node = path[i];
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

			// Update elapsed time
			timing(&wc_time, &cpu_time);
			elapsed = wc_time - start;
		}
		// All done with iterations
		#pragma omp atomic update
		iterations += my_iterations;
	
		// Do root synchronization
		for (int i = 0; i < next_positions.size(); i++) {
			// Ensure entry exists
			if (pos_map.find(next_positions[i]->get_vec()) == pos_map.end()) {
				continue;	
			}
			MctsNodeRootParallel* next_node = pos_map.find(next_positions[i]->get_vec())->second;
			#pragma omp critical
			{
				scores[i].first += next_node->get_reward();
				scores[i].second += next_node->get_visits(); 
			}
		}

		// Clear memory used by this thread
		for (auto it: pos_map) {
			delete it.second;
		}
		pos_map.clear();
	}

	// Choose best action
	float max_ratio = -INFINITY;
	Move* best_move = NULL;
	for (int i = 0; i < poss_moves.size(); i++) {
		Move* move = poss_moves[i];
		float curr_ratio = scores[i].first / scores[i].second;
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

void MctsAgentRootParallel::reset() {}

