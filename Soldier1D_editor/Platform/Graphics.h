#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <string.h>
#include <list>
#include <iostream>
#include <vector>

using namespace std;

#ifndef _GLIBCXX_GRAPHICS_H
#define _GLIBCXX_GRAPHICS_H

struct RGBA{
	SDL_Color col;
	RGBA(Uint8 R, Uint8 G, Uint8 B, Uint8 A){
		col.r = R;
		col.g = G;
		col.b = B;
		col.a = A;
	}
	void setR(Uint8 r){ col.r = r; }
	void setG(Uint8 g){ col.g = g; }
	void setB(Uint8 b){ col.b = b; }
	void setA(Uint8 a){ col.a = a; }
};

class Error{
	const char* err;
public:
	Error(const char* err);
	const char* getError();
};

class Graphics{
	#define MAX_FONTS 10
	SDL_Window* win;
	SDL_Renderer* ren;
	vector<SDL_Texture*> textures;
	TTF_Font* fonts[MAX_FONTS];
	int current_fonts = 0;
	const int W = 640;
	const int H = 640;
public:
	Graphics();
	void renderScene();
	unsigned int loadTexture(const char* path);
	void applyTexture(SDL_Texture* t, double x, double y, double width, double height);
	int loadFont(const char* font, int size);
	void displayText(int font, const char* text, RGBA color, double x, double y, double w, double h);
	void getTextWH(int font, const char* text, double& w, double& h);
	double getMouseX();
	double getMouseY();
	//double getMouseY();
	SDL_Texture* getTexture(unsigned int id);
	~Graphics();
};

#endif