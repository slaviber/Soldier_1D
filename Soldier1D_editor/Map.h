#include <iostream>
#include <vector>
using namespace std;

#ifndef _GLIBCXX_Map_H
#define _GLIBCXX_Map_H
class Map{
	unsigned int size = 1;
public:
	void changeMapSize(unsigned int);
	unsigned int getMapSize();
};
#endif