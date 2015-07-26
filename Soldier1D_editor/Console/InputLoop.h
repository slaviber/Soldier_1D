#include <iostream>
#include <vector>
using namespace std;

#ifndef _GLIBCXX_InputLoop_H
#define _GLIBCXX_InputLoop_H

class Console{
public:
	static int inputLoop(void*);
	virtual void parseInput(vector<string>) = 0;
};

#endif