#include <iostream>
#include <vector>
#include <map>
#include <typeinfo>

using namespace std;

#ifndef _GLIBCXX_ITEMS_H
#define _GLIBCXX_ITEMS_H

class ItemResources {
	static map<const type_info*, int> textures;
	static map<int, int> e_nums;
public:
	static const int getTextureID(const type_info* ti);
	static void addTextureID(int newID, const type_info*, int e_num);
	static int getTextureIDByEnum(int e_num);
};

class Item{
	int x=0;
	static unsigned int uid;
public:
	Item(int x);
	int getItemX();
	void setItemX(int x);
	virtual map<string,int> getStats() = 0;
};

class SpawnPoint: public Item{
	map<string,int> stats;
public:
	SpawnPoint(int x);
	map<string,int> getStats();
};

enum ITEMS {SPAWN_POINT=0, LAST_ITEM};

#endif