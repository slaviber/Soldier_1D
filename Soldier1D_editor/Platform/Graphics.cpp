#include "Graphics.h"

using namespace std;

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

Graphics::Graphics(){
	if ((win = SDL_CreateWindow("Soldier 1D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI)) == nullptr)throw Error(SDL_GetError());
	if ((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == nullptr)throw Error(SDL_GetError());	
	if (TTF_Init())throw Error(TTF_GetError());
	for (int i = 0; i < 256; ++i)
	{
		palette[i].r = ((i >> 0) & 7) << 5;
		palette[i].g = ((i >> 3) & 7) << 5;
		palette[i].b = ((i >> 6) & 3) << 6;
	}
}

Graphics::~Graphics(){
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	TTF_Quit();
}

void Graphics::renderScene() {
	SDL_RenderPresent(ren);
	if (SDL_RenderClear(ren))throw Error(SDL_GetError());
}

unsigned int Graphics::loadTexture(const char* path){
	SDL_Texture* tex;
	if ((tex = IMG_LoadTexture(ren, path)) == nullptr)throw Error(IMG_GetError());
	textures.push_back(tex);
	return textures.size()-1;
}

unsigned int Graphics::loadBackground(unsigned char bg[16]){
	SDL_Surface* srf = SDL_CreateRGBSurfaceFrom(bg, 16, 1, 8, 16, 0, 0, 0, 0);
	SDL_SetPaletteColors(srf->format->palette, palette, 0, 256);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, srf);
	SDL_FreeSurface(srf);
	textures.push_back(tex);
	map_background = textures.size() - 1;
	return textures.size() - 1;

}

void Graphics::changeBackground(unsigned char bg[16]){
	SDL_Surface* srf = SDL_CreateRGBSurfaceFrom(bg, 16, 1, 8, 16, 0, 0, 0, 0);
	SDL_SetPaletteColors(srf->format->palette, palette, 0, 256);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, srf);
	SDL_FreeSurface(srf);
	textures.at(map_background) = tex;
}

void Graphics::applyTexture(SDL_Texture* t, double x, double y, double width, double height){
	SDL_Rect dst;
	SDL_Rect src;
	int W, H;
	if (SDL_QueryTexture(t, NULL, NULL, &src.w, &src.h))throw Error(SDL_GetError());
	SDL_GetWindowSize(win, &W, &H);
	dst.x=x*W;
	dst.y=y*H;
	dst.w=width*W;
	dst.h=height*H;
	src.x=0;
	src.y=0;
	if (SDL_RenderCopyEx(ren, t, &src, &dst, 0, NULL, SDL_FLIP_NONE))throw Error(SDL_GetError());
}

SDL_Texture* Graphics::getTexture(unsigned int id){
	if(id>textures.size()-1) throw new Error("Wrong size");
	return textures.at(id);
}

int Graphics::loadFont(const char* font, int size){
	if (current_fonts == MAX_FONTS)throw Error("font limit reached");
	TTF_Font *Font;
	if ((Font = TTF_OpenFont(font, size)) == nullptr)throw Error(TTF_GetError());
	fonts[current_fonts] = Font;
	return current_fonts++;
}

void Graphics::displayText(int font, const char* text, RGBA color, double x, double y, double w, double h){
	SDL_Surface *surf;
	SDL_Texture *texture;
	if ((surf = TTF_RenderUTF8_Blended(fonts[font], text, color.col)) == nullptr)throw Error(TTF_GetError());
	if ((texture = SDL_CreateTextureFromSurface(ren, surf)) == 0)throw Error(SDL_GetError());
	SDL_FreeSurface(surf);
	applyTexture(texture, x, y, w, h);
	SDL_DestroyTexture(texture);
}

void Graphics::getTextWH(int font, const char* text, double& w, double& h){
	int wi, hi;
	if (TTF_SizeUTF8(fonts[font], text, &wi, &hi))throw Error(TTF_GetError());

	w = wi / (double)H;
	h = hi / (double)H;
}

double Graphics::getMouseX(){
	int x;
	SDL_GetMouseState(&x, NULL);
	return (double)x/(double)H;
}

double Graphics::getMouseY(){
	int y;
	SDL_GetMouseState(NULL, &y);
	return (double)y / (double)W;
}

bool Graphics::getLeftClick(){
	return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1);
}

unsigned int Graphics::Time(){
	return SDL_GetTicks();
}