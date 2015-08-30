#include <iostream>
#include "SDL_net.h"
#include "SDL.h"

#include "Socket.h"
using namespace std;

int main(int argc, char** argv){
	if (SDL_Init(0) == -1)throw Error(("SDL_Init: " + string(SDL_GetError())).c_str());
	SocketManager::startSocketManager();
	while (1){
		ClientManager::ClientEvent ce = ClientManager::pollClientEvent();
		switch (ce.event_occured){
		case ClientManager::ClientEvents::CONNECT:
			cout << "connection with id: " << ce.client_id << endl;
			break;
		case ClientManager::ClientEvents::DISCONNECT:
			cout << "connection lost with id: " << ce.client_id << endl;
			break;
		case  ClientManager::ClientEvents::UPDATE:
			cout << "received message with id: " << ce.client_id << " : " << ce.message << endl;
			break;
		default: SDL_Delay(10);
		}
	}
	SocketManager::stopSocketManager();
	SDL_Quit();

return 0;
}