#include <iostream>
#include "Graphics.h"
#include "InputLoop.h"
#include "SDL_thread.h"
#include "Map.h"
#include "Items.h"
#include <complex>

using namespace std;

class Game : public Console{
	Graphics *display;
	Map* map = new Map();
	bool quit;
	SDL_Event event;
	SDL_Thread *thread_g;
	int main_font;
	float mappos_x = 0;
	int zoom = 0;
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
		}
		catch(Error e){
			cout<<e.getError()<<endl;
		}
		quit=0;
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
	return mx+0.5;
}

void Game::loop(){
	if ((thread_g = SDL_CreateThread(Console::inputLoop, "commands", (void *)this)) == NULL) throw Error(SDL_GetError());
	main_font = display->loadFont("Fonts/ARIALUNI.ttf", 12);
	int map_bgr = display->loadTexture("Textures/bgr.png");
	int gray_bgr = display->loadTexture("Textures/gray_bgr.png");
	int orange = display->loadTexture("Textures/orange.png");
	int tiles = display->loadTexture("Textures/tiles.png");
	ItemResources::addTextureID(display->loadTexture("Textures/spawn.png"), &typeid(SpawnPoint), SPAWN_POINT);
	
	
	while (!quit) {
		double main_w, main_h;
		display->renderScene();
		
		
		//display->applyTexture(display->getTexture(0), (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x, 0.45, (0.1 / pow(2, zoom))*map->getMapSize(), 0.1);
		display->applyTexture(display->getTexture(gray_bgr), 0, 0.2, 1, 0.1);
		for (int i = 0; i < map->getMapSize(); ++i){
			double blocksize = (0.1 / pow(2, zoom));
			double blockpos = (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x / pow(2, zoom) + blocksize*i;
			display->applyTexture(display->getTexture(map_bgr), blockpos, 0.2, blocksize, 0.1);
		}
		//display->applyTexture(display->getTexture(0), (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x, 0.45, (0.1 / pow(2, zoom))*map->getMapSize(), 0.1);
		string pos = "X=" + to_string(getBlockPos(display->getMouseX()));

		display->getTextWH(main_font, pos.c_str(), main_w, main_h);
		display->displayText(main_font, pos.c_str(), RGBA(0, 255, 0, 0), 0.9, 0.9, main_w, main_h);
		
		display->applyTexture(display->getTexture(gray_bgr), 0, 0.3, 1, 0.1);
		
		int block_min = getBlockPos(0);
		int block_max =	getBlockPos(1);
		int visible_size = block_max-block_min;
		int block_avg = (block_min+block_max)/2;
		double visible_part = (double)visible_size/(double)map->getMapSize();
		double offset_part = (double)block_avg/(double)map->getMapSize();
		
		if(visible_part<0.01) visible_part=0.01;
		
		display->applyTexture(display->getTexture(orange), offset_part+0.5-visible_part/2, 0.3, visible_part, 0.1);
		
		display->applyTexture(display->getTexture(tiles), 0, 0, 1, 0.2);
	
		for(int i=0; i<LAST_ITEM; i++){
			int grid_x = i%10;
			int grid_y = i/10;
			
			int texture = ItemResources::getTextureIDByEnum(i);
			
			display->applyTexture(display->getTexture(texture), grid_x*0.1, grid_y*0.1, 0.1, 0.1);
		}
	
		if (SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT: 
					quit=1;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym){
					case SDLK_LEFT:
						mappos_x -= 0.1*pow(2, zoom);
						break;
					case SDLK_RIGHT:
						mappos_x += 0.1*pow(2, zoom);
						break;
					case SDLK_UP:
						zoom++;
						break;
					case SDLK_DOWN:
						if(zoom > 0)zoom--;
						break;
					case SDLK_RCTRL:
						mappos_x -= (display->getMouseX() - 0.5)*pow(2, zoom);
						break;
				}
			}
		}
		//cout << display->getMouseX() << endl;
		SDL_Delay(10);
	}
}

void Game::parseInput(vector<string> input){
	//for (int i = 0; i < input.size(); ++i){
	//	cout << input[i] << endl;
	//}
	if (input[0] == "mapsize"){
		unsigned int size = atoi(input[1].c_str());
		if (size == 0)cout << "Warning! Size can't be zero!" << endl;
		else{
			map->changeMapSize(size);
		}
	}
}

int main(int argc, char** argv){
	Game &g = *new Game();
	try{
		g.loop();
	}
	catch(Error e){
		cout<<e.getError()<<endl;
	}
	catch (exception& e){
		cout << e.what() << endl;
	}
	
	return 0;
}