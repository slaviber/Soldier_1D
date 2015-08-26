#include <iostream>
#include <thread>
#include <vector>
#include <memory>
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


void client(TCPsocket new_tcpsock){

	int result;
	char msg[MAXLEN];
	while (1){
		result = SDLNet_TCP_Recv(new_tcpsock, msg, MAXLEN);
		if (result <= 0) {
			const char* error = SDL_GetError();
			SDLNet_TCP_Close(new_tcpsock);
			cout << "SDLNet_TCP_Recv: " << error << endl;
			return;
		}
		cout << "received " << endl;
		for (int i = 0; i < result; ++i){
			cout << msg[i];
		}
		cout << endl;
	}

}

int main(int argc, char** argv){
	IPaddress ipaddress;
	TCPsocket tcpsock;	
	TCPsocket new_tcpsock;
	vector<shared_ptr<thread>> threads;

	try{

		if (SDL_Init(0) == -1)throw Error(("SDL_Init: " + string(SDL_GetError())).c_str());

		if (SDLNet_Init() == -1)throw Error(("SDLNet_Init: " + string(SDL_GetError())).c_str());

		if (SDLNet_ResolveHost(&ipaddress, NULL, 7777) == -1)throw Error(("SDLNet_ResolveHost: " + string(SDL_GetError())).c_str());

		if (!(tcpsock = SDLNet_TCP_Open(&ipaddress)))throw Error(("SDLNet_TCP_Open: " + string(SDL_GetError())).c_str());



		//SDLNet_SocketSet set;
		//if (!(set = SDLNet_AllocSocketSet(16)))throw Error(("SDLNet_AllocSocketSet: " + string(SDL_GetError())).c_str());






		while (1){
			new_tcpsock = SDLNet_TCP_Accept(tcpsock);
			if (!new_tcpsock) {
				//cout << "SDLNet_TCP_Accept: " + string(SDL_GetError()) << endl;
			}
			//int numused;
			//numused = SDLNet_UDP_AddSocket(set, new_tcpsock);
			//if (numused == -1)throw Error(("SDLNet_UDP_AddSocket: " + string(SDL_GetError())).c_str());
			else {
				threads.push_back(shared_ptr<thread>(new thread(client, new_tcpsock)));
				cout << threads.size() << endl;
			}
			SDL_Delay(1);
		}




		//int result;
		//char msg[MAXLEN];
		//while (1){
		//	result = SDLNet_TCP_Recv(new_tcpsock, msg, MAXLEN);
		//	if (result <= 0) {
		//		cout << "SDLNet_TCP_Recv: " + string(SDL_GetError()) << endl;
		//		SDLNet_TCP_Close(new_tcpsock);
		//		break;
		//	}
		//	cout << "received " << endl;
		//	for (int i = 0; i < result; ++i){
		//		cout << msg[i];
		//	}
		//	cout << endl;
		//}

	}
	catch (Error& e){
		cout << e.getError() << endl;
	}


	for (auto i : threads){
		i->join();
	}

	SDLNet_TCP_Close(tcpsock);
	SDLNet_Quit();
	SDL_Quit();

	return 0;
}

