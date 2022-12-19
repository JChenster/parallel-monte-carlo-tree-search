#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

#include "game.h"
#include "mcts_serial.h"
#include "connect_four.h"
#include "mcts_leaf_parallel.h"

#define TEST_GAMES (1)
#define EPSILON (0.15)
#define TIME_LIMIT (0.01)

class RandomAgent: public Agent {
	pair<Move*,int> best_move(Position* pos, float time_limit) override {
		vector<Move*> poss_moves = pos->possible_moves();
		Move* rand_move = poss_moves[rand() % poss_moves.size()];
		return make_pair(rand_move, 0);
	}

	void reset() override {}
};

void compare_agents(Game* game, Agent* a1, Agent* a2) {
	float p0_wins = 0;
	pair<int, int> a1_iter = make_pair(0, 0);
	pair<int, int> a2_iter = make_pair(0, 0);
	for (int i = 0; i < TEST_GAMES; i++) {
		Position* pos = game->new_game();
		while (!pos->is_terminal()) {
			float r = (float) rand() / RAND_MAX;
			Move* move;
			if (r < EPSILON) {
				// Use strategy here
				if (pos->whose_turn() == 0) {
					pair<Move*, int> res = a1->best_move(pos, TIME_LIMIT);
					move = res.first;
					a1_iter.first += res.second;
					a1_iter.second++;
				} else {
					pair<Move*, int> res = a2->best_move(pos, TIME_LIMIT);
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
	float p0_win_rate = p0_wins / TEST_GAMES;
	printf("Player 0 win rate across %d games: %f\n", TEST_GAMES, p0_win_rate);
	float a1_avg_iter = (float) a1_iter.first / a1_iter.second;	
	float a2_avg_iter = (float) a2_iter.first / a2_iter.second;
	printf("Agent 1 Average MCTS Iterations: %f\n", a1_avg_iter);
	printf("Agent 2 Average MCTS Iterations: %f\n", a2_avg_iter);
}

int main() {
	ConnectFourGame* connect_four = new ConnectFourGame();
	MctsAgentSerial* mcts_agent = new MctsAgentSerial();
	//MctsAgentLeafParallel* mcts_agent = new MctsAgentLeafParallel();
	RandomAgent* rand_agent = new RandomAgent();
	cout << "Player 1: Serial MCTS" << endl;
	cout << "Player 2: Random" << endl;
	cout << "Epsilon: " << EPSILON << endl;
	compare_agents(connect_four, mcts_agent, rand_agent);
}

