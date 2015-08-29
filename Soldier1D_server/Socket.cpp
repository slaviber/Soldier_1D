#include "Socket.h"
#include <iostream>

using namespace std;

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

ClientManager::ClientEvent ClientManager::pollClientEvent(){
	client_event_mutex.lock();
	if (client_events.size()){
		ClientEvent ce = client_events.front();
		client_events.pop();
		client_event_mutex.unlock();
		return ce;
	}
	client_event_mutex.unlock();
	return ClientEvent{ ClientEvents::NONE, 0, "" };
}

void ClientManager::addServerEvent(ClientEvent e){
	server_events.push(e);
}

void ClientManager::addClientEvent(ClientEvent e){
	client_event_mutex.lock();
	client_events.push(e);
	client_event_mutex.unlock();
}

ClientManager::ClientEvent ClientManager::pollServerEvent(){
	server_event_mutex.lock();
	if (server_events.size()){
		ClientEvent ce = server_events.front();
		server_events.pop();
		server_event_mutex.unlock();
		return ce;
	}
	server_event_mutex.unlock();
	return ClientEvent{ ClientEvents::NONE, 0, "" };
}

queue<ClientManager::ClientEvent> ClientManager::client_events;
queue<ClientManager::ClientEvent> ClientManager::server_events;
mutex ClientManager::client_event_mutex;
mutex ClientManager::server_event_mutex;

SocketManager::Client::Client(TCPsocket s){
	this->s = s;
	id = uid++;
}
char& SocketManager::Client::operator [](const int index){
	return buf[index];
}
TCPsocket& SocketManager::Client::operator *(){
	return s;
}
char* SocketManager::Client::operator &(){
	return buf;
}
unsigned int SocketManager::Client::operator ()(){
	return id;
}
int SocketManager::Client::operator +(){
	return buflen;
}
int SocketManager::Client::operator -(){
	return -buflen;
}
int& SocketManager::Client::operator +=(int i){
	buflen += i;
	return buflen;
}
int& SocketManager::Client::operator -=(int i){
	buflen -= i;
	return buflen;
}

void SocketManager::mainSocketThread(){
	IPaddress ipaddress;
	TCPsocket tcpsock;
	TCPsocket new_tcpsock;
	SDLNet_SocketSet set;
	ClientManager client_manager;
	map<unsigned int, Client> sockets;
	int numused = 0;
	int result;

	try{

		if (SDLNet_Init() == -1)throw Error(("SDLNet_Init: " + string(SDL_GetError())).c_str());

		if (SDLNet_ResolveHost(&ipaddress, NULL, 7777) == -1)throw Error(("SDLNet_ResolveHost: " + string(SDL_GetError())).c_str());

		if (!(tcpsock = SDLNet_TCP_Open(&ipaddress)))throw Error(("SDLNet_TCP_Open: " + string(SDL_GetError())).c_str());

		if (!(set = SDLNet_AllocSocketSet(SOCKS)))throw Error(("SDLNet_AllocSocketSet: " + string(SDL_GetError())).c_str());

		while (!stop){
			if (numused == SOCKS)continue; //server socketset is full; should be avoided
			new_tcpsock = SDLNet_TCP_Accept(tcpsock);
			if (!new_tcpsock){
				const char* error = SDL_GetError();
				if (strcmp("accept() failed", error)){
					throw Error(("SDLNet_TCP_Accept: " + string(error)).c_str());
				}
			}
			else {
				numused = SDLNet_TCP_AddSocket(set, new_tcpsock);
				if (numused == -1)throw Error(("SDLNet_AddSocket: " + string(SDL_GetError())).c_str());
				Client cl = Client(new_tcpsock);
				if (sockets.insert(pair<unsigned int, Client>(cl(), cl)).second == false)throw Error("Internal server error. Client connected with an existing socket. Aborting. 'sockets.insert(pair<unsigned int, Client>(cl(), cl))'");
				client_manager.addClientEvent({ ClientManager::ClientEvents::CONNECT, cl(), "" });
			}
			if (numused){ //there are clients
				int ringing = SDLNet_CheckSockets(set, 0);
				if (ringing == -1)throw Error(("error in SDLNet_CheckSockets. Possible cause: " + string(SDL_GetError())).c_str());
				else if (ringing){
					map<unsigned int, Client>::iterator vit;
					for (vit = sockets.begin(); vit != sockets.end(); ++vit){
						Client& cl = vit->second;
						if (SDLNet_SocketReady(*cl)){
							result = SDLNet_TCP_Recv(*cl, &cl[+cl], cl.MAXLEN + -cl);
							if (result <= 0) {
								const char* error = SDL_GetError();
								SDLNet_TCP_Close(*cl);
								numused = SDLNet_TCP_DelSocket(set, *cl);
								client_manager.addClientEvent({ ClientManager::ClientEvents::DISCONNECT, cl(), "" });
								sockets.erase(vit);
								if (strcmp("accept() failed", error)){
									string error2 = "";
									if (numused == -1)error2 = SDL_GetError();
									throw Error(("SDLNet_TCP_Recv: " + string(error) + error2 == "" ? "" : " SDLNet_TCP_DelSocket: " + error2).c_str());
								}
								if (numused == -1)throw Error(("SDLNet_TCP_DelSocket: " + string(SDL_GetError())).c_str());
								break;
							}
							int oldlen = +cl;
							cl += result;
							for (int i = oldlen; i < +cl; ++i){
								if (!cl[i]){
									i += 1;
									string chunk(&cl, i);
									memmove(&cl, &cl + i, +cl - i);
									cl -= i;
									client_manager.addClientEvent({ ClientManager::ClientEvents::UPDATE, cl(), chunk });
									oldlen = 0;
									i = oldlen;
								}
							}
						}
					}
				}
			}
			while (1){
				ClientManager::ClientEvent ce = client_manager.pollServerEvent();
				if (ce.event_occured == ClientManager::ClientEvents::NONE)break;
				try{
					Client cl = sockets.at(ce.client_id);
					switch (ce.event_occured){
					case  ClientManager::ClientEvents::UPDATE:
						result = SDLNet_TCP_Send(*cl, ce.message.c_str(), ce.message.length() + 1);
						if (result < ce.message.length() + 1) {
							const char* error = SDL_GetError();
							SDLNet_TCP_Close(*cl);
							numused = SDLNet_TCP_DelSocket(set, *cl);
							string error2 = "";
							if (numused == -1)error2 = SDL_GetError();
							client_manager.addClientEvent({ ClientManager::ClientEvents::DISCONNECT, cl(), "" });
							sockets.erase(cl());
							throw Error(("SDLNet_TCP_Send: " + string(error) + error2 == "" ? "" : " SDLNet_TCP_DelSocket: " + error2).c_str());
						}
						break;
					default: break;
					}
				}
				catch (exception e){
					cout << "server requests manipulation of a nonexistent client. Has the client disconnected? Client cl = sockets.at(ce.client_id); " << e.what() << endl;
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
}
void SocketManager::startManager(SocketManager* self){
	self->start = thread(mainSocketThread);
	self->start.detach();
}

SocketManager::SocketManager(){
	call_once(one_thread, startManager, this);
}

SocketManager::~SocketManager(){
	stop = true;
}

unsigned int SocketManager::Client::uid = 0;
bool SocketManager::stop = false;
