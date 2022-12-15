#include <iostream>
using namespace std;

#include "mcts_serial.h"
#include "connect_four.h"

int main() {
	MctsAgentSerial agent = MctsAgentSerial();
	ConnectFourPosition pos = ConnectFourPosition();
	cout << agent.best_move(&pos, 5.0) << endl;
}


