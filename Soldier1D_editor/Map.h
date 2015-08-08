#include <iostream>
#include <vector>
#include <memory>
#include "Items.h"
using namespace std;

#ifndef _GLIBCXX_Map_H
#define _GLIBCXX_Map_H
class Map{
	unsigned int size = 1;
	unsigned char background[16];
public:
	void changeMapSize(unsigned int);
	unsigned int getMapSize();
	vector<unique_ptr<Item>> items;
	void saveMap(string);
	void readMap(string);
	void setBackground(unsigned char[16]);
	unsigned char* getBackground();
};
#endif