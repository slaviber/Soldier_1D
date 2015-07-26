#include "Items.h"

map<const type_info*, int> ItemResources::textures;
map<int, int> ItemResources::e_nums;

const int ItemResources::getTextureID(const type_info* ti) {
	return textures.at(ti);
}

void ItemResources::addTextureID(int newID, const type_info* ti, int e_num) {
	textures[ti] = e_nums[e_num] = newID;
}

int ItemResources::getTextureIDByEnum(int e_num){
	return e_nums[e_num];
}

Item::Item(int x):x(x){
	uid++;
}

int Item::getItemX(){
	return this->x;
}

void Item::setItemX(int x){
	this->x=x;
}

unsigned int Item::uid=0;

SpawnPoint::SpawnPoint(int x):Item(x){
	stats.insert(pair<string,int>("Team",0));
}

map<string,int> SpawnPoint::getStats(){
	return stats;
}