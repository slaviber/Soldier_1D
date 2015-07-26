#include "InputLoop.h"
#include <sstream>

int Console::inputLoop(void* target){
	while (1){
		string input;
		getline(cin, input);
		stringstream ss(input);
		string token;
		vector<string> elems;
		while (ss >> token)elems.push_back(token);
		Console& c = *reinterpret_cast<Console*>(target);
		c.parseInput(elems);

	}
	return 0;
}