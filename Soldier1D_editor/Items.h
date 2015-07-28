#include <iostream>
#include <vector>
#include <map>
#include <typeinfo>
#include <array>

using namespace std;

#ifndef _GLIBCXX_ITEMS_H
#define _GLIBCXX_ITEMS_H

enum ITEMS { SPAWN_POINT = 0, LAST_ITEM };

class Item;

class ItemResources {
	static map<const type_info*, int> textures;
	static map<int, int> e_nums;
public:
	static const int getTextureID(const type_info* ti);
	template<typename T> static void addTextureID(int newID, int e_num);
	static int getTextureIDByEnum(int e_num);
	static array<Item*(*)(int), LAST_ITEM> item_types;
};

class Item{
	int x=0;
	static unsigned int uid;
	const unsigned int id;
public:
	Item(int x);
	int getItemX();
	void setItemX(int x);
	unsigned int getUID();
	virtual map<string,int> getStats() = 0;
	virtual string getName() = 0;
};

class SpawnPoint: public Item{
	map<string,int> stats;
public:
	SpawnPoint(int x);
	map<string,int> getStats();
	string getName();
};

template<typename T> Item * createInstance(int x) { return new T(x); }

template<class T> void ItemResources::addTextureID(int newID, int e_num) {
	textures[&typeid(T)] = e_nums[e_num] = newID;
	item_types[e_num] = &createInstance<T>;
}


#endif