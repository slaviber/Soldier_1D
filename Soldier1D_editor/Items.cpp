#include "Items.h"

map<const type_info*, int> ItemResources::textures;
map<int, int> ItemResources::e_nums;
vector<Item*(*)(int)> ItemResources::item_types;
unsigned int ItemResources::last_item = 0;

const int ItemResources::getTextureID(const type_info* ti) {
	return textures.at(ti);
}

int ItemResources::getTextureIDByEnum(int e_num){
	return e_nums[e_num];
}

unsigned int ItemResources::getLastItem(){
	return last_item;
}

Item::Item(int x):x(x),id(uid++){
	
}

int Item::getItemX(){
	return this->x;
}

void Item::setItemX(int x){
	this->x=x;
}

map<string,int> Item::getStats(){
	return stats;
}

bool Item::updateStat(string stat, int value){
	map<string, int>::iterator it;
	it = stats.find(stat);
	if (it == stats.end())return false;
	stats[stat] = value;
	return true;
}

unsigned int Item::uid=0;

unsigned int Item::getUID(){
	return this->id;
}

SpawnPoint::SpawnPoint(int x):Item(x){
	stats.insert(pair<string,int>("Team",0));
}

string SpawnPoint::getName(){
	return "Spawn Point";
}

Base::Base(int x):Item(x){
	stats.insert(pair<string,int>("Team",0));
	stats.insert(pair<string,int>("Range",0));
}

string Base::getName(){
	return "Team Base";
}

Flag::Flag(int x):Item(x){
	stats.insert(pair<string,int>("Team",0));
}

string Flag::getName(){
	return "Team Flag";
}

Ammo::Ammo(int x):Item(x){
	stats.insert(pair<string,int>("Quantity",0));
	stats.insert(pair<string,int>("Weapon_type",0));
}

string Ammo::getName(){
	return "Ammo";
}

Weapon::Weapon(int x):Item(x){
	stats.insert(pair<string,int>("Weapon_type",0));
	stats.insert(pair<string,int>("Ammo_clip",0));
	stats.insert(pair<string,int>("Durability",0));
}

string Weapon::getName(){
	return "Weapon";
}
