#include <iostream>
#include "Graphics.h"
#include "InputLoop.h"
#include "SDL_thread.h"
#include "Map.h"
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

void Game::loop(){
	if ((thread_g = SDL_CreateThread(Console::inputLoop, "commands", (void *)this)) == NULL) throw Error(SDL_GetError());
	main_font = display->loadFont("Fonts/ARIALUNI.ttf", 12);
	display->loadTexture("Textures/bgr.png");
	while (!quit) {
		double main_w, main_h;
		display->renderScene();
		//display->applyTexture(display->getTexture(0), (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x, 0.45, (0.1 / pow(2, zoom))*map->getMapSize(), 0.1);
		for (int i = 0; i < map->getMapSize(); ++i){
			double blocksize = (0.1 / pow(2, zoom));
			double blockpos = (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x / pow(2, zoom) + blocksize*i;
			display->applyTexture(display->getTexture(0), blockpos, 0.45, blocksize, 0.1);
		}
		//display->applyTexture(display->getTexture(0), (0.5 - ((0.1 / pow(2, zoom))*map->getMapSize()) / 2.0) + mappos_x, 0.45, (0.1 / pow(2, zoom))*map->getMapSize(), 0.1);
		double mx = display->getMouseX();
		mx -= 0.5;
		mx -= mappos_x / pow(2, zoom);
		mx *= pow(2, zoom);
		mx *= 10;
		if (mx < -0.5)mx -= 1;
		int block_pos = mx+0.5;
		string pos = "X=" + to_string(block_pos);

		display->getTextWH(main_font, pos.c_str(), main_w, main_h);
		display->displayText(main_font, pos.c_str(), RGBA(0, 255, 0, 0), 0.9, 0.9, main_w, main_h);
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