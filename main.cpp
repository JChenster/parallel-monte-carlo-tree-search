#include <iostream>
using namespace std;

#include "mcts_serial.h"
#include "connect_four.h"

int main() {
	MctsAgentSerial agent = MctsAgentSerial();
	pos_map_t pos_map;
	ConnectFourPosition pos = ConnectFourPosition(&pos_map);
	cout << agent.best_move(&pos, 5.0) << endl;
}


