#include <omp.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#include "timing.h"
#include "mcts_leaf_parallel.h"

#define ROLLOUTS (20)

MctsNodeLeafParallel::MctsNodeLeafParallel(Position* p): pos(p), reward(0), visits(0), children(vector<child_info_lp>()) {}

// Accessor functions
float MctsNodeLeafParallel::get_reward() {
	return reward;
}

int MctsNodeLeafParallel::get_visits() {
	return visits;
}

bool MctsNodeLeafParallel::is_leaf() {
	return children.empty();
}

void MctsNodeLeafParallel::add_child(MctsNodeLeafParallel* new_child) {
	children.push_back(make_pair(new_child, make_pair(0, 0)));
}

void MctsNodeLeafParallel::inc_reward(float delta) {
	reward += delta;
}

void MctsNodeLeafParallel::inc_visits(float delta) {
	visits += delta;
}

void MctsNodeLeafParallel::expand(pos_map_lp_t* pos_map) {
	// Get next possible moves
	vector<Move*> next_moves = pos->possible_moves();
	for (Move* move: next_moves) {
		// See subsequent positions and either locate corresponding node already in tree
		// or insert into tree
		Position* new_pos = pos->make_move(move);
		if (pos_map->find(new_pos->get_vec()) == pos_map->end()) {
			MctsNodeLeafParallel* new_child = new MctsNodeLeafParallel(new_pos);
			pos_map->insert(make_pair(new_pos->get_vec(), new_child));
		}
		// Add subsequent node as child of current node
		this->add_child(pos_map->find(new_pos->get_vec())->second);
	}
}

float MctsNodeLeafParallel::calc_ucb2_child(child_info_lp child){
	int edge_visits = child.second.second;
	MctsNodeLeafParallel* child_node = child.first;
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
MctsNodeLeafParallel* MctsNodeLeafParallel::select_child() {
	float max_ucb = -INFINITY;
	vector<MctsNodeLeafParallel*> optimal_children;
	for (child_info_lp child: this->children) {
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
	int rand_idx = rand() % optimal_children.size();
	return optimal_children[rand_idx];
}

// Assumes that caller has at least one child
MctsNodeLeafParallel* MctsNodeLeafParallel::select_first_child() {
	return this->children[0].first;
}

MctsAgentLeafParallel::MctsAgentLeafParallel() {}

// time_limit is in seconds
pair<Move*,int> MctsAgentLeafParallel::best_move(Position* p, float time_limit) {
	double wc_time, cpu_time;
	timing(&wc_time, &cpu_time);
	double start = wc_time;

	// Look up node in search tree or create new one
	MctsNodeLeafParallel* pos_node;
	if (pos_map.find(p->get_vec()) != pos_map.end()) {
		pos_node = pos_map.find(p->get_vec())->second;
	} else {
		pos_node = new MctsNodeLeafParallel(p);
		pos_map.insert(make_pair(p->get_vec(), pos_node));
	}

	int iterations = 0;
	double elapsed = 0.0;
	// Continue search algorithm while time_limit is not complete
	while (elapsed < time_limit) {
		// Start at base node
		MctsNodeLeafParallel* leaf_node = pos_node;
		vector<MctsNodeLeafParallel*> path;
		path.push_back(pos_node);

		// Traverse tree until we reach a leaf by picking child with highest UCB
		while (!leaf_node->is_leaf()) {
			leaf_node = leaf_node->select_child();
			path.push_back(leaf_node);
		}

		// Shared variable for rollout reward and visits that all threads access
		float rollout_reward;
		int rollout_visits = 1;

		// We have now reached leaf
		// If game over, we have reached terminal node
		if (leaf_node->pos->is_terminal()) {
			rollout_reward = leaf_node->pos->payoff();
			iterations++;
		}
		// If not game over, then we need to expand and rollout
		else {
			// Get the node to rollout from
			MctsNodeLeafParallel* playout_node;
			if (leaf_node->get_visits() == 0) {
				playout_node = leaf_node;
			} else {
				leaf_node->expand(&pos_map);
				playout_node = leaf_node->select_first_child();
				path.push_back(playout_node);
			}
			
			Position* curr_pos = playout_node->pos; 
			
			// ** BEGIN PARALLEL SECTION **
			// Rollout
			#pragma omp parallel \
				shared(rollout_reward) \
				firstprivate(curr_pos) \
				default(none)
			{
				// Each thread should get its own random seed
				srand(int(time(NULL)) ^ omp_get_thread_num());
				// Parrallelize leaf rollouts here
				// Do dynamic scheduling because rollout may be different complexity
				#pragma omp for schedule(dynamic)
				for (int r = 0; r < ROLLOUTS; r++) {
					// Rollout
					while (!curr_pos->is_terminal()) {
						vector<Move*> poss_moves = curr_pos->possible_moves();
						Move* next_move = poss_moves[rand() % poss_moves.size()];
						curr_pos = curr_pos->make_move(next_move);
					}
					//printf("%d\n", omp_get_thread_num());
					// Now at terminal state
					#pragma omp atomic update
					rollout_reward += 1;
				}
			}
			// ** END PARALLEL SECTION **
			
			rollout_visits = ROLLOUTS;
			// Count multiple rollouts as multiple iterations
			iterations += ROLLOUTS;
		}

		// Back propagate
		for (int i = 0; i < path.size(); i++) {
			MctsNodeLeafParallel* node = path[i];
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
	/*
	cout << "MCTS pos_map size: " << pos_map.size() << endl;
	for (auto it: pos_map) {
		printf("(%f, %d)\n", it.second->get_reward(), it.second->get_visits());
		it.second->pos->print();
	}
	*/

	// Choose best action
	float max_ratio = -INFINITY;
	Move* best_move = NULL;
	for (Move* move: p->possible_moves()) {
		Position* next_pos = p->make_move(move);
		MctsNodeLeafParallel* next_node = pos_map.find(next_pos->get_vec())->second;	
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

void MctsAgentLeafParallel::reset() {
	for (auto it: pos_map) {
		// Delete node
		delete it.second;
	}
	pos_map.clear();
}

