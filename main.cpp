#include <iostream>
using namespace std;

#include "mcts_serial.h"
#include "fake.h"

int main() {
	MctsAgentSerial agent = MctsAgentSerial();
	FakePosition pos = FakePosition();
	cout << agent.best_move(&pos, 5.0) << endl;
}


