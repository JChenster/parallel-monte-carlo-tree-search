#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

#include "game.h"
#include "mcts_serial.h"
#include "connect_four.h"


#define TEST_GAMES (100)
#define EPSILON (0.15)
#define TIME_LIMIT (1.0)

class RandomAgent: public Agent {
	Move* best_move(Position* pos, float time_limit) override {
		vector<Move*> poss_moves = pos->possible_moves();
		return poss_moves[rand() % poss_moves.size()];
	}

	void reset() override {}
};

void compare_agents(Game* game, Agent* a1, Agent* a2) {
	float p0_wins = 0;
	for (int i = 0; i < TEST_GAMES; i++) {
		Position* pos = game->new_game();
		while (!pos->is_terminal()) {
			float r = (float) rand() / RAND_MAX;
			Move* move;
			if (r < EPSILON) {
				// Use strategy here
				if (pos->whose_turn() == 0) {
					move = a1->best_move(pos, TIME_LIMIT);
				} else {
					move = a2->best_move(pos, TIME_LIMIT);
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
}

int main() {
	ConnectFourGame* connect_four = new ConnectFourGame();
	MctsAgentSerial* mcts_serial_agent = new MctsAgentSerial();
	RandomAgent* rand_agent = new RandomAgent();
	cout << "Player 1: Serial MCTS" << endl;
	cout << "Player 2: Random" << endl;
	cout << "Epsilon: " << EPSILON << endl;
	compare_agents(connect_four, mcts_serial_agent, rand_agent);
}

