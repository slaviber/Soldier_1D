#include "Map.h"
#include <fstream>

void Map::changeMapSize(unsigned int size){
	this->size = size;
}

unsigned int Map::getMapSize(){
	return size;
}

void Map::setBackground(unsigned char bg[16]){
	memcpy(background, bg, 16);
}
unsigned char* Map::getBackground(){
	return background;
}

void Map::saveMap(string mapfile){
	ofstream out;
	out.open(mapfile.c_str(), ios::binary);

	out << "SDM ";

	out.write((char*)(&size), sizeof(int));
	out.write((char*)background, sizeof(char) * 16);

	for (int i = 0; i < items.size(); ++i){

		Item& curr_item = *items.at(i);

		int id = ItemResources::getItemID(&typeid(curr_item));

		out.write((char*)(&id), sizeof(int));

		int X = curr_item.getItemX();

		out.write((char*)(&X), sizeof(int));

		map<string, int> stats = items.at(i)->getStats();;
		for (auto& it : stats){
			out.write((char*)(&it.second), sizeof(int));
		}
	}
}

void Map::readMap(string mapfile){
	size = 1;
	items.clear();
	ifstream in;
	in.open(mapfile.c_str(), ios::binary);

	char magic[4];
	in.read((char*)(&magic), sizeof(char)*4);
	if (!(magic[0] == 'S' && magic[1] == 'D' && magic[2] == 'M' && magic[3] == ' '))throw Error("wrong map file!");

	in.read((char*)(&size), sizeof(unsigned int));
	in.read((char*)(background), sizeof(char) * 16);

	while (!in.eof()){
		int itemid, itemx;
		in.read((char*)(&itemid), sizeof(unsigned int));
		in.read((char*)(&itemx), sizeof(unsigned int));

		if (in.eof())break;

		items.push_back(unique_ptr<Item>(ItemResources::item_types[itemid](itemx)));

		vector<int> stats;
		for (int i = 0; i < (*items.back()).getStats().size(); ++i){
			int val;
			in.read((char*)(&val), sizeof(unsigned int));
			stats.push_back(val);
		}
		(*items.back()).updateStat(stats);
	}
}