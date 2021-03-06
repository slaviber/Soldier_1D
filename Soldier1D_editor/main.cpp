#include <iostream>
#include "Graphics.h"
#include "InputLoop.h"
#include "SDL_thread.h"
#include "Map.h"
#include "Items.h"
#include <complex>

using namespace std;

/*int reverse(int x, int bits)
{
	x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
	x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
	x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
	x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
	x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);
	return x >> (32 - bits);
}*/

class Game : public Console{
	Graphics *display;
	Map* map = new Map();
	bool quit;
	SDL_Event event;
	SDL_Thread *thread_g;
	int main_font;
	int head_font;
	float mappos_x = 0;
	int zoom = 0;
	int curr_selection = 0;
	bool left_clicked = false;
	vector<Item*> reference_point;
	unsigned int last_time_mil;
	bool update_bg = false;
public:
	Game();
	void loop();
	void parseInput(vector<string>);
	int getBlockPos(double x);
	~Game();
};

Game::Game(){
	try{
		if (SDL_Init(SDL_INIT_EVERYTHING))throw Error(SDL_GetError());
		display = new Graphics();
		last_time_mil = display->Time();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}
	quit = 0;
}

Game::~Game(){
	delete display;
	SDL_Quit();
}

int Game::getBlockPos(double x){
	double mx = x;
	mx -= 0.5;
	mx -= mappos_x / pow(2, zoom);
	mx *= pow(2, zoom);
	mx *= 10;
	if (mx < -0.5)mx -= 1;
	return mx + 0.5;
}

void Game::loop(){
	if ((thread_g = SDL_CreateThread(Console::inputLoop, "commands", (void *)this)) == NULL) throw Error(SDL_GetError());
	main_font = display->loadFont("Fonts/ARIALUNI.TTF", 12);
	head_font = display->loadFont("Fonts/ARIALUNI.TTF", 16);
	int map_bgr = display->loadTexture("Textures/bgr.png");
	int gray_bgr = display->loadTexture("Textures/gray_bgr.png");
	int orange = display->loadTexture("Textures/orange.png");
	int tiles = display->loadTexture("Textures/tiles.png");
	int selected = display->loadTexture("Textures/selected.png");
	int selected_white = display->loadTexture("Textures/selected_white.png");
	int not_so_black = display->loadTexture("Textures/not_so_black.png");
	ItemResources::addTextureID<SpawnPoint>(display->loadTexture("Textures/spawn_style1.png"));
	ItemResources::addTextureID<Base>(display->loadTexture("Textures/TeamBase_style1.png"));
	ItemResources::addTextureID<Flag>(display->loadTexture("Textures/Flag_style1.png"));
	ItemResources::addTextureID<Ammo>(display->loadTexture("Textures/Ammo_style1.png"));
	ItemResources::addTextureID<Weapon>(display->loadTexture("Textures/Weapon_style1.png"));
	ItemResources::addTextureID<Health>(display->loadTexture("Textures/MedKit_style1.png"));
	ItemResources::addTextureID<Bush>(display->loadTexture("Textures/Bush_style1.png"));
	ItemResources::addTextureID<Fence>(display->loadTexture("Textures/Fence_style1.png"));
	ItemResources::addTextureID<Bunker>(display->loadTexture("Textures/Bunker_style1.png"));
	//ItemResources::addTextureID<BFG>(display->loadTexture("Textures/BFG_style1.png"));
	ItemResources::addTextureID<Truck>(display->loadTexture("Textures/Truck_style1.png"));
	ItemResources::addTextureID<BTR>(display->loadTexture("Textures/APC_style1.png"));
	ItemResources::addTextureID<Tank>(display->loadTexture("Textures/Tank_style1.png"));

	unsigned char bg[16]{255, 255, 255, 254, 255, 255, 255, 254, 254, 255, 254, 255, 255, 255, 254, 255};
	map->setBackground(bg);
	map_bgr = display->loadBackground(map->getBackground());

	for (int i = 0; i < ItemResources::getLastItem(); ++i){
		reference_point.push_back(ItemResources::item_types[i](0));
	}

	while (!quit) {
		double main_w, main_h;
		display->renderScene();



		display->applyTexture(display->getTexture(gray_bgr), 0, 0.2, 1, 0.1);
		int map_min_block = -(map->getMapSize() / 2);
		int map_max_block = +(map->getMapSize() / 2);
		int draw_begin = getBlockPos(0) < map_min_block ? 0 : getBlockPos(0) - map_min_block - 1;
		draw_begin = draw_begin < 0 ? 0 : draw_begin;
		int draw_end = getBlockPos(1) > map_max_block ? map->getMapSize() : getBlockPos(1) + map_max_block + 1;
		draw_end = draw_end > map->getMapSize() ? map->getMapSize() : draw_end;

		int selection_x = display->getMouseX() * 10;
		int selection_y = display->getMouseY() * 10;

		double blocksize = (0.1 / pow(2, zoom));
		double block_part = (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x / pow(2, zoom);
		for (int i = draw_begin; i < draw_end; ++i){
			double blockpos = block_part + blocksize*i;
			display->applyTexture(display->getTexture(map_bgr), blockpos, 0.2, blocksize, 0.1);
			if (i == getBlockPos(display->getMouseX()) + (map->getMapSize() / 2) && selection_y == 2){
				if (curr_selection < ItemResources::getLastItem())display->applyTexture(display->getTexture(ItemResources::getTextureIDByEnum(curr_selection)), blockpos, 0.2, blocksize, 0.1);
				else display->applyTexture(display->getTexture(selected_white), blockpos, 0.2, blocksize, 0.1);
			}
		}

		double stat_text_pos = 0.405;
		int text_counter = 0;

		for (int i = 0; i < map->items.size(); i++){
			Item& curr_item = *map->items.at(i);
			int x = curr_item.getItemX();
			if (x >= draw_begin - map_max_block && x <= draw_end - map_max_block){
				display->applyTexture(display->getTexture(ItemResources::getTextureID(&typeid(curr_item))), block_part + blocksize*(x + map_max_block), 0.2, blocksize, 0.1);
			}
			if (getBlockPos(display->getMouseX()) == x && selection_y == 2){
				++text_counter;
				RGBA c1 = *new RGBA(0, 255, 0, 0);
				RGBA c2 = *new RGBA(255, 255, 0, 0);

				RGBA text_color = text_counter % 2 ? c1 : c2;
				string item_name = curr_item.getName();
				double text_w, text_h;
				display->getTextWH(head_font, item_name.c_str(), text_w, text_h);
				display->displayText(head_font, item_name.c_str(), text_color, 0.01, stat_text_pos, text_w, text_h);

				double temp_h = text_h;
				double temp_w = text_w;

				string uid_string = " UID: " + to_string(curr_item.getUID());

				display->getTextWH(main_font, uid_string.c_str(), text_w, text_h);
				display->displayText(main_font, uid_string.c_str(), text_color, 0.01 + temp_w, stat_text_pos + (temp_h - text_h), text_w, text_h);

				temp_w += text_w;

				uid_string = " / ";

				display->getTextWH(head_font, uid_string.c_str(), text_w, text_h);
				display->displayText(head_font, uid_string.c_str(), text_color, 0.01 + temp_w, stat_text_pos, text_w, text_h);

				std::map<string, int> stats = curr_item.getStats();

				for (std::map<string, int>::iterator it = stats.begin(); it != stats.end(); ++it){
					uid_string = it->first + ": " + to_string(it->second) + " ";

					temp_w += text_w;

					display->getTextWH(main_font, uid_string.c_str(), text_w, text_h);
					display->displayText(main_font, uid_string.c_str(), text_color, 0.01 + temp_w, stat_text_pos + text_h/2.0 -(temp_h-text_h)/2.0, text_w, text_h);
				}

				stat_text_pos += text_h*0.8;

				display->applyTexture(display->getTexture(not_so_black), 0, stat_text_pos + 0.01, 1, 0.002);
			}

		}

		string pos = "X=" + to_string(getBlockPos(display->getMouseX()));

		display->getTextWH(main_font, pos.c_str(), main_w, main_h);
		display->displayText(main_font, pos.c_str(), RGBA(0, 255, 0, 0), 0.8, 0.9, main_w, main_h);

		display->applyTexture(display->getTexture(gray_bgr), 0, 0.3, 1, 0.1);

		int block_min = getBlockPos(0);
		int block_max = getBlockPos(1);
		int visible_size = block_max - block_min;
		int block_avg = (block_min + block_max) / 2;
		double visible_part = (double)visible_size / (double)map->getMapSize();
		double offset_part = (double)block_avg / (double)map->getMapSize();

		if (visible_part < 0.01) visible_part = 0.01;

		display->applyTexture(display->getTexture(orange), offset_part + 0.5 - visible_part / 2, 0.3, visible_part, 0.1);

		display->applyTexture(display->getTexture(tiles), 0, 0, 1, 0.2);

		for (int i = 0; i < ItemResources::getLastItem(); i++){
			int grid_x = i % 10;
			int grid_y = i / 10;

			int texture = ItemResources::getTextureIDByEnum(i);

			display->applyTexture(display->getTexture(texture), grid_x*0.1, grid_y*0.1, 0.1, 0.1);
		}

		display->applyTexture(display->getTexture(selected), (curr_selection % 10)*0.1, (curr_selection / 10)*0.1, 0.1, 0.1);

		if (selection_y < 2)display->applyTexture(display->getTexture(selected_white), selection_x*0.1, selection_y*0.1, 0.1, 0.1);


		int hovered_item = selection_y * 10 + selection_x;

		if (hovered_item < ItemResources::getLastItem()){
			string item = reference_point[hovered_item]->getName();
			display->getTextWH(main_font, item.c_str(), main_w, main_h);
			display->displayText(main_font, item.c_str(), RGBA(0, 255, 0, 0), 0.8, 0.95, main_w, main_h);
		}

		if (selection_y == 2){
			if (getBlockPos(display->getMouseX()) >= map_min_block && getBlockPos(display->getMouseX()) <= map_max_block)
				if (curr_selection < ItemResources::getLastItem()){
					string item = "Will place " + reference_point[curr_selection]->getName();
					display->getTextWH(main_font, item.c_str(), main_w, main_h);
					display->displayText(main_font, item.c_str(), RGBA(0, 255, 0, 0), 0.8, 0.95, main_w, main_h);
				}
		}

		if (display->getLeftClick())left_clicked = true;
		if (!display->getLeftClick() && left_clicked){
			left_clicked = false;
			if (hovered_item < 20){
				curr_selection = hovered_item;
			}

			if (selection_y == 2){
				if (getBlockPos(display->getMouseX()) >= map_min_block && getBlockPos(display->getMouseX()) <= map_max_block)
					if (curr_selection < ItemResources::getLastItem())
						map->items.push_back(unique_ptr<Item>(ItemResources::item_types[curr_selection](getBlockPos(display->getMouseX()))));
			}
		}


		if (update_bg){
			update_bg = false;
			display->changeBackground(map->getBackground());
		}

		if (last_time_mil + 600000 < display->Time()){
			last_time_mil = display->Time();
			map->saveMap("autosaved.sdm");
		}

		double mouse_center_x;
		if (SDL_PollEvent(&event)){
			switch (event.type){
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym){
				case SDLK_LEFT:
					if (getBlockPos(1) <= map_max_block)mappos_x -= 0.05*pow(2, zoom);
					break;
				case SDLK_RIGHT:
					if (getBlockPos(0) >= map_min_block)mappos_x += 0.05*pow(2, zoom);
					break;
				case SDLK_UP:
					if (zoom < 10)zoom++;
					break;
				case SDLK_DOWN:
					if (zoom > 0)zoom--;
					break;
				case SDLK_RCTRL:
					mouse_center_x = (display->getMouseX() - 0.5)*pow(2, zoom);
					if (mappos_x - mouse_center_x > map_min_block / 10.0 && mappos_x - mouse_center_x < map_max_block / 10.0)
						mappos_x -= mouse_center_x;
					break;
				}
			}
		}
		SDL_Delay(10);
	}
}

void Game::parseInput(vector<string> input){
	try{
		if (input[0] == "mapsize" && input.size() == 2){
			unsigned int size = atoi(input[1].c_str());
			if (size == 0)cout << "Warning! Size can't be zero!" << endl;
			else{
				map->changeMapSize(size);
				mappos_x = 0;
			}
		}

		else if (input[0] == "stat" && input.size() == 4){
			unsigned int uid = stoul(input[1], nullptr, 0);
			int value = stoi(input[3], nullptr, 0);

			bool found = false;
			for (int i = 0; i < map->items.size(); ++i){
				if (map->items.at(i)->getUID() == uid){
					found = true;
					if (!map->items.at(i)->updateStat(input[2], value))cout << "wrong stat!" << endl;
					break;
				}
			}
			if (!found) cout << "invalid item uid!" << endl;
		}

		else if (input[0] == "delete" && input.size() == 2){
			unsigned int uid = stoul(input[1], nullptr, 0);

			bool found = false;
			for (int i = 0; i < map->items.size(); ++i){
				if (map->items.at(i)->getUID() == uid){
					found = true;
					map->items.erase(map->items.begin() + i);
					break;
				}
			}
			if (!found) cout << "invalid item uid!" << endl;
		}

		else if (input[0] == "save" && input.size() == 2){
			map->saveMap(input[1]);
		}

		else if (input[0] == "load" && input.size() == 2){
			map->readMap(input[1]);
			update_bg = true;
		}

		else if (input[0] == "copy" && input.size() == 3){
			unsigned int uid = stoul(input[1], nullptr, 0);
			int x = stoul(input[2], nullptr, 0);

			bool found = false;
			for (int i = 0; i < map->items.size(); ++i){
				if (map->items.at(i)->getUID() == uid){
					found = true;
					Item& old = *map->items.at(i);
					int item_id = ItemResources::getItemID(&typeid(old));
					Item* new_item = ItemResources::item_types[item_id](x);
					std::map<string, int> old_stats = old.getStats();
					vector<int> stats;
					for (auto &it : old_stats){
						stats.push_back(it.second);
					}
					new_item->updateStat(stats);
					map->items.push_back(unique_ptr<Item>(new_item));
					break;
				}
			}
			if (!found) cout << "invalid item uid!" << endl;
		}

		else if (input[0] == "quit" && input.size() == 1){
			quit = true;
		}

		else if (input[0] == "background" && input.size() == 17){
			unsigned char bytes[16];
			for (int i = 0; i < 16; ++i){
				bytes[i] = (unsigned char)stoul(input[i+1], nullptr, 0);
			}
			map->setBackground(bytes);
			update_bg = true;


		}

		else cout << "wrong command" << endl;
	}
	catch (Error e){
		cout << e.getError() << endl;
	}
	catch (exception e){
		cout << e.what() << endl;
	}
}

int main(int argc, char** argv){
	Game &g = *new Game();
	try{
		g.loop();
	}
	catch (Error e){
		cout << e.getError() << endl;
	}
	catch (exception& e){
		cout << e.what() << endl;
	}

	return 0;
}
