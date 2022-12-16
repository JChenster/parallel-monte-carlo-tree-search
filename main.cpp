#include <iostream>
#include <vector>
using namespace std;

#include "mcts_serial.h"
#include "connect_four.h"

void my_test() {
	MctsAgentSerial agent = MctsAgentSerial();
	ConnectFourGame game = ConnectFourGame();
	ConnectFourPosition* init_pos = (ConnectFourPosition*) game.new_game();
	init_pos->print();
	init_pos->print_pos_map();
	vector<Move*> next_moves = init_pos->possible_moves();
	for (auto it: next_moves) {
		cout << "\t";
		((ConnectFourMove*) it)->print();
	}
	ConnectFourPosition* curr_pos = (ConnectFourPosition*) init_pos->make_move(next_moves[3]);
	curr_pos->print();
	curr_pos->print_pos_map();
	init_pos->print_pos_map();
	
	ConnectFourMove* best_move = (ConnectFourMove*) agent.best_move(init_pos, 0.001);
	cout << "Best move: " << best_move << endl;
	best_move->print();
	//curr_pos->print_pos_map();

}

int main() {

}

