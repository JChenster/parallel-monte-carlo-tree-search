#include <string.h>

#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

#include "game.h"
#include "mcts_serial.h"
#include "connect_four.h"
#include "mcts_leaf_parallel.h"
#include "mcts_root_parallel.h"

class RandomAgent: public Agent {
	pair<Move*,int> best_move(Position* pos, float time_limit) override {
		vector<Move*> poss_moves = pos->possible_moves();
		Move* rand_move = poss_moves[rand() % poss_moves.size()];
		return make_pair(rand_move, 0);
	}

	void reset() override {}
};

void compare_agents(Game* game, Agent* a1, Agent* a2, int test_games, float epsilon, float time_limit) {
	float p0_wins = 0;
	pair<int, int> a1_iter = make_pair(0, 0);
	pair<int, int> a2_iter = make_pair(0, 0);
	for (int i = 0; i < test_games; i++) {
		Position* pos = game->new_game();
		while (!pos->is_terminal()) {
			float r = (float) rand() / RAND_MAX;
			Move* move;
			if (r < epsilon) {
				// Use strategy here
				if (pos->whose_turn() == 0) {
					pair<Move*, int> res = a1->best_move(pos, time_limit);
					move = res.first;
					a1_iter.first += res.second;
					a1_iter.second++;
				} else {
					pair<Move*, int> res = a2->best_move(pos, time_limit);
					move = res.first;
					a2_iter.first += res.second;
					a2_iter.second++;
				}
			} else {
				// Otherwise random
				vector<Move*> poss_moves = pos->possible_moves();
				move = poss_moves[rand() % poss_moves.size()];
			}
			// Move to next position
			pos = pos->make_move(move);
		}
		p0_wins += pos->payoff();
		// Reset agent cache
		a1->reset();
		a2->reset();
	}
	float p0_win_rate = p0_wins / test_games;
	printf("Player 0 win rate across %d games: %f\n", test_games, p0_win_rate);
	float a1_avg_iter = (float) a1_iter.first / a1_iter.second;	
	float a2_avg_iter = (float) a2_iter.first / a2_iter.second;
	printf("Agent 1 Average MCTS Iterations: %f\n", a1_avg_iter);
	printf("Agent 2 Average MCTS Iterations: %f\n", a2_avg_iter);
}

int main(int argc, char* argv[]) {
	if (argc != 6) {
		cout << "Usage: <Agent 1> <Agent 2> <Test games> <Epsilon> <Time limit>" << endl;
		cout << "Valid agents are random, serial, leaf_parallel" << endl;
		exit(-1);
	}
	
	Game* connect_four = new ConnectFourGame();
	
	// Initialize hyper-parameters
	int test_games = atoi(argv[3]);
	float epsilon = atof(argv[4]);
	float time_limit = atof(argv[5]);
	cout << "Simulating " << test_games << " games" << endl;
	cout << "Epsilon: " << epsilon << endl;
	cout << "Time limit for each MCTS run: " << time_limit << endl;
	
	// Initialize agents from command line
	Agent* agents[2];
	for (int a = 0; a < 2; a++) {
		cout << "Player " << a << ": ";
		if (!strcmp(argv[1+a], "random")) {
			agents[a] = new RandomAgent();
			cout << "Random" << endl;
		} else if (!strcmp(argv[1+a], "serial")) {
			agents[a] = new MctsAgentSerial();
			cout << "Serial MCTS" << endl;
		} else if (!strcmp(argv[1+a], "leaf")) {
			agents[a] = new MctsAgentLeafParallel();
			cout << "Leaf Parallel MCTS" << endl;
		} else if (!strcmp(argv[1+a], "root")) {
			agents[a] = new MctsAgentRootParallel();
			cout << "Root Parallel MCTS" << endl;
		} else {
			cout << "Invalid input: " << argv[1+a];
			exit(-1);
		}
	}

	compare_agents(connect_four, agents[0], agents[1], test_games, epsilon, time_limit);
}

