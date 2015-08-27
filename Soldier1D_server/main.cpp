#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include<array>
#include "SDL_net.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"


using namespace std;


class Error{
	const char* err;
public:
	Error(const char* err);
	const char* getError();
};

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }


#define MAXLEN 1024
#define SOCKS 16

int main(int argc, char** argv){
	IPaddress ipaddress;
	TCPsocket tcpsock;	
	TCPsocket new_tcpsock;
	SDLNet_SocketSet set;
	vector<TCPsocket> sockets;
	int numused = 0;
	int result;
	char msg[MAXLEN];

	try{

		if (SDL_Init(0) == -1)throw Error(("SDL_Init: " + string(SDL_GetError())).c_str());

		if (SDLNet_Init() == -1)throw Error(("SDLNet_Init: " + string(SDL_GetError())).c_str());

		if (SDLNet_ResolveHost(&ipaddress, NULL, 7777) == -1)throw Error(("SDLNet_ResolveHost: " + string(SDL_GetError())).c_str());

		if (!(tcpsock = SDLNet_TCP_Open(&ipaddress)))throw Error(("SDLNet_TCP_Open: " + string(SDL_GetError())).c_str());

		if (!(set = SDLNet_AllocSocketSet(SOCKS)))throw Error(("SDLNet_AllocSocketSet: " + string(SDL_GetError())).c_str());

		while (1){
			new_tcpsock = SDLNet_TCP_Accept(tcpsock);
			if (!new_tcpsock);
			else {
				numused = SDLNet_TCP_AddSocket(set, new_tcpsock);
				if (numused == -1)throw Error(("SDLNet_AddSocket: " + string(SDL_GetError())).c_str());
				else cout << numused << endl;
				sockets.push_back(new_tcpsock);
			}
			if (numused){ //there are clients
				int ringing = SDLNet_CheckSockets(set, 0);
				if (ringing == -1)throw Error(("error in SDLNet_CheckSockets. Possible cause: " + string(SDL_GetError())).c_str());
				else if (ringing){
					vector<TCPsocket>::iterator vit;
					for (vit = sockets.begin(); vit != sockets.end(); ++vit){
						if (SDLNet_SocketReady(*vit)){
							result = SDLNet_TCP_Recv(*vit, msg, MAXLEN);
							if (result <= 0) {
								const char* error = SDL_GetError();
								SDLNet_TCP_Close(*vit);
								numused = SDLNet_TCP_DelSocket(set, *vit);
								sockets.erase(vit);
								cout << "SDLNet_TCP_Recv: " << error << endl;
								if (numused == -1)throw Error(("SDLNet_TCP_DelSocket: " + string(SDL_GetError())).c_str());
								break;
							}
							cout << "received " << endl;
							for (int i = 0; i < result; ++i){
								cout << msg[i];
							}
							cout << endl;
						}
					}
				}
			}
			SDL_Delay(1);
		}
	}
	catch (Error& e){
		cout << e.getError() << endl;
	}

	SDLNet_FreeSocketSet(set);
	SDLNet_TCP_Close(tcpsock);
	SDLNet_Quit();
	SDL_Quit();

	return 0;
}

// TODO: class-based server logic with separate client buffers