#include <iostream>
using namespace std;

#include "mcts_serial.h"
#include "fake.h"

int main() {
	MctsAgent agent = MctsAgent();
	FakePosition pos = FakePosition();
	cout << agent.best_move(&pos) << endl;
}


