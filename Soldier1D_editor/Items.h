#include <iostream>
#include <vector>
#include <map>
#include <typeinfo>
#include <array>

#include "Platform/Graphics.h" //mainly due to the need of Error()

using namespace std;

#ifndef _GLIBCXX_ITEMS_H
#define _GLIBCXX_ITEMS_H

class Item;

class ItemResources {
	static map<const type_info*, int> textures;
	static map<int, int> e_nums;
	static map<const type_info*, int> ids;
	static unsigned int last_item;
public:
	static const int getTextureID(const type_info* ti);
	template<typename T> static void addTextureID(int newID);
	static int getTextureIDByEnum(int e_num);
	static vector<Item*(*)(int)> item_types;
	static unsigned int getLastItem();
	static int getItemID(const type_info*);
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
	void updateStat(vector<int>);
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

class Health: public Item{
public:
	Health(int x);
	string getName();
};

class Bush: public Item{
public:
	Bush(int x);
	string getName();
};

class Fence: public Item{
public:
	Fence(int x);
	string getName();
};

class Bunker: public Item{
public:
	Bunker(int x);
	string getName();
};

class BFG: public Item{
public:
	BFG(int x);
	string getName();
};

class Truck: public Item{
public:
	Truck(int x);
	string getName();
};

class BTR: public Item{
public:
	BTR(int x);
	string getName();
};

class Tank: public Item{
public:
	Tank(int x);
	string getName();
};


template<typename T> Item * createInstance(int x) { return new T(x); }

template<class T> void ItemResources::addTextureID(int newID) {
	map<const type_info*, int>::iterator it;
	it = textures.find(&typeid(T));
	string error = "class ";
	error += typeid(T).name();
	error += " already registered!";
	if (it != textures.end()){
		string &err = *new string(error);
		throw Error(err.c_str());
	}

	textures[&typeid(T)] = e_nums[last_item] = newID;
	ids[&typeid(T)] = last_item;
	item_types.push_back(&createInstance<T>);
	last_item++;
}


#endif