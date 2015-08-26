#include <iostream>
#include "SDL_net.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"


using namespace std;

int main(int argc, char** argv){
	IPaddress ipaddress;
	TCPsocket tcpsock;	
	
	if(SDL_Init(0)==-1) {
		printf("SDL_Init: %s\n", SDL_GetError());
		exit(1);
	}
	if(SDLNet_Init()==-1) {
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}
	SDLNet_ResolveHost(&ipaddress, NULL, 1337);
	
	tcpsock=SDLNet_TCP_Open(&ipaddress);
		if(!tcpsock) {
		printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(2);
	}
	
	TCPsocket new_tcpsock;
	while(1){
		new_tcpsock=SDLNet_TCP_Accept(tcpsock);
		if(!new_tcpsock) {
			//printf("SDLNet_TCP_Accept: %s\n", SDLNet_GetError());
		}
		else {
			cout<<"Noooo"<<endl;
			break;
		}
	}

	#define MAXLEN 1024
	int result;
	char msg[MAXLEN];
	while(1){
		result=SDLNet_TCP_Recv(new_tcpsock,msg,MAXLEN);
		if(result<=0) {
			// An error may have occured, but sometimes you can just ignore it
			// It may be good to disconnect sock because it is likely invalid now.
			break;
		}
		cout<<result<<endl;
		printf("Received: \"%s\"\n",msg);
	}
	
	cout<<"GG"<<endl;
	SDLNet_TCP_Close(tcpsock);
	SDLNet_Quit();
	SDL_Quit();
	return 0;
}

