#include "Items.h"

map<const type_info*, int> ItemResources::textures;
map<int, int> ItemResources::e_nums;
array<Item*(*)(int), LAST_ITEM> ItemResources::item_types;

const int ItemResources::getTextureID(const type_info* ti) {
	return textures.at(ti);
}

int ItemResources::getTextureIDByEnum(int e_num){
	return e_nums[e_num];
}

Item::Item(int x):x(x),id(uid++){
	
}

int Item::getItemX(){
	return this->x;
}

void Item::setItemX(int x){
	this->x=x;
}

unsigned int Item::uid=0;

unsigned int Item::getUID(){
	return this->id;
}

SpawnPoint::SpawnPoint(int x):Item(x){
	stats.insert(pair<string,int>("Team",0));
}

map<string,int> SpawnPoint::getStats(){
	return stats;
}

string SpawnPoint::getName(){
	return "Spawn Point";
}