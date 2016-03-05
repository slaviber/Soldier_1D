#include "Map.h"
#include <fstream>
#include <lua.hpp>

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
	out.open(mapfile.c_str());
	out << "--This is an auto generated Soldier 1D map file--" << endl;
	out << "mapsize = " << size << endl;
	out << "background = '";
	for (int i = 0; i < 16; ++i){
		out << "\\" << (int)background[i];
	}
	out << "'" << endl;
	for (int i = 0; i < items.size(); ++i){
		Item& curr_item = *items.at(i);
		int id = ItemResources::getItemID(&typeid(curr_item));
		int X = curr_item.getItemX();
		out << "Item{" << endl;
		out << "\tid = " << id << "," << endl;
		out << "\tX = " << X << "," << endl;
		map<string, int> stats = items.at(i)->getStats();
		for (auto& it : stats){
			out << "\t" << it.first << " = " << it.second << "," << endl;
		}
		out << "}" << endl;
	}
}

static void stackDump(lua_State *L) {
	int i = lua_gettop(L);
	printf(" ----------------  Stack Dump ----------------\n");
	while (i) {
		int t = lua_type(L, i);
		switch (t) {
		case LUA_TSTRING:
			cout << i << " " << lua_tostring(L, i) << endl;
			break;
		case LUA_TBOOLEAN:
			cout << i << " " << lua_toboolean(L, i) << endl;
			break;
		case LUA_TNUMBER:
			cout << i << " " << lua_tonumber(L, i) << endl;
			break;
		default: cout << i << " " << lua_typename(L, t) << endl; break;
		}
		i--;
	}
	printf("--------------- Stack Dump Finished ---------------\n");
}

void Map::readMap(string mapfile){

	size = 1;
	items.clear();
	lua_State *L = luaL_newstate();
	string parser = "Items = {} \
		function Item(item) Items[#Items + 1] = item end";
	int error = luaL_loadbuffer(L, parser.c_str(), parser.length(), "mapfile parser") || lua_pcall(L, 0, 0, 0);
	if (!error){
		error = luaL_dofile(L, mapfile.c_str());
		if (error)goto A;
	}
	else{
	A:
		cout << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
		lua_close(L);
		return;
	}
	lua_getglobal(L, "mapsize");
	if (!lua_isinteger(L, -1)) Error("damaged mapfile - missing mapsize");
	size = lua_tointeger(L, -1);
	lua_pop(L, 1);
	lua_getglobal(L, "background");
	if (!lua_isstring(L, -1)) Error("damaged mapfile - missing backgound");
	memcpy(background, lua_tolstring(L, -1, NULL), 16);
	lua_pop(L, 1);
	lua_getglobal(L, "Items");
	if (!lua_istable(L, -1))Error("damaged mapfile - missing Item holding structure");
	int counter = 0;
	while (1){
		counter++;
		int result = lua_geti(L, -1, counter);
		if (result == LUA_TNIL)break;
		if (result != LUA_TTABLE) Error((string("damaged mapfile - Item holding structure corrupted near entry ") + to_string(counter)).c_str());
		lua_getfield(L, -1, "id");
		if (!lua_isinteger(L, -1)) Error((string("damaged mapfile - Item holding structure id missing near entry ") + to_string(counter)).c_str());
		int id = lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, -1, "X");
		if (!lua_isinteger(L, -1)) Error((string("damaged mapfile - Item holding structure X missing near entry ") + to_string(counter)).c_str());
		int x = lua_tointeger(L, -1);
		lua_pop(L, 1);
		items.push_back(unique_ptr<Item>(ItemResources::item_types[id](x)));
		map<string, int> stats = (*items.back()).getStats();
		vector<int> newstats;
		for (auto i : stats){
			result = lua_getfield(L, -1, i.first.c_str());
			if (result == LUA_TNIL)continue;
			if (result != LUA_TNUMBER)Error((string("damaged mapfile - Item has noninteger stat " + i.first + " near entry ") + to_string(counter)).c_str());
			newstats.push_back(lua_tointeger(L, -1));
			lua_pop(L, 1);
			
		}
		(*items.back()).updateStat(newstats);
		lua_pop(L, 1);
	}
	lua_close(L);
}
