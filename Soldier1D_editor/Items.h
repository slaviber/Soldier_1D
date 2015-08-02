#include <iostream>
#include <vector>
#include <map>
#include <typeinfo>
#include <array>

using namespace std;

#ifndef _GLIBCXX_ITEMS_H
#define _GLIBCXX_ITEMS_H

class Item;

class ItemResources {
	static map<const type_info*, int> textures;
	static map<int, int> e_nums;
	static unsigned int last_item;
public:
	static const int getTextureID(const type_info* ti);
	template<typename T> static void addTextureID(int newID);
	static int getTextureIDByEnum(int e_num);
	static vector<Item*(*)(int)> item_types;
	static unsigned int getLastItem();
};

class Item{
	int x=0;
	static unsigned int uid;
	const unsigned int id;
protected:
	map<string,int> stats;
public:
	Item(int x);
	int getItemX();
	void setItemX(int x);
	unsigned int getUID();
	map<string,int> getStats();
	virtual string getName() = 0;
	bool updateStat(string, int);
};

class SpawnPoint: public Item{
public:
	SpawnPoint(int x);
	string getName();
};

class Base: public Item{
public:
	Base(int x);
	string getName();
};

class Flag: public Item{
public:
	Flag(int x);
	string getName();
};

class Ammo: public Item{
public:
	Ammo(int x);
	string getName();
};

class Weapon: public Item{
public:
	Weapon(int x);
	string getName();
};

template<typename T> Item * createInstance(int x) { return new T(x); }

template<class T> void ItemResources::addTextureID(int newID) {

	textures.insert(std::pair<const type_info*, int>(&typeid(T), newID));
	e_nums[last_item] = newID;
	item_types.push_back(&createInstance<T>);
	last_item++;
}


#endif