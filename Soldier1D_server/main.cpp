#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <map>
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


class ClientManager{

public:
	struct ClientEvent;
private:

	static queue<ClientEvent> client_events;
	static queue<ClientEvent> server_events;
	static mutex client_event_mutex;
	static mutex server_event_mutex;

public:
	enum ClientEvents{CONNECT = 0, DISCONNECT, UPDATE, NONE};
	struct ClientEvent{
		ClientEvents event_occured;
		unsigned int client_id;
		string message;
	};

	ClientEvent pollClientEvent(){
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
	void addServerEvent(ClientEvent e){
		server_events.push(e);
	}
	void addClientEvent(ClientEvent e){
		client_event_mutex.lock();
		client_events.push(e);
		client_event_mutex.unlock();
	}
	ClientEvent pollServerEvent(){
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
};

queue<ClientManager::ClientEvent> ClientManager::client_events;
queue<ClientManager::ClientEvent> ClientManager::server_events;
mutex ClientManager::client_event_mutex;
mutex ClientManager::server_event_mutex;

class SocketManager{

	thread start;
	once_flag one_thread;
	static bool stop;
	static const int SOCKS = 16;

	class Client{

	public:
		static const int MAXLEN = 65536;
	private:

		static unsigned int uid;
		TCPsocket s;
		char buf[MAXLEN];
		int buflen = 0;
		unsigned int id;
	public:
		Client(TCPsocket s){
			this->s = s;
			id = uid++;
		}
		char& operator [](const int index){
			return buf[index];
		}
		TCPsocket& operator *(){
			return s;
		}
		char* operator &(){
			return buf;
		}
		unsigned int operator ()(){
			return id;
		}
		int operator +(){
			return buflen;
		}
		int operator -(){
			return -buflen;
		}
		int& operator +=(int i){
			buflen += i;
			return buflen;
		}
		int& operator -=(int i){
			buflen -= i;
			return buflen;
		}

	};

	static void mainSocketThread(){
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
	static void startManager(SocketManager* self){
		self->start = thread(mainSocketThread);
		self->start.detach();
	}
public:
	SocketManager(){
		call_once(one_thread, startManager, this);
	}

	~SocketManager(){
		stop = true;
	}
};

unsigned int SocketManager::Client::uid = 0;
bool SocketManager::stop = false;

int main(int argc, char** argv){
	if (SDL_Init(0) == -1)throw Error(("SDL_Init: " + string(SDL_GetError())).c_str());
	SocketManager* sm = new SocketManager();
	ClientManager client_manager;
	while (1){
		ClientManager::ClientEvent ce = client_manager.pollClientEvent();
		switch (ce.event_occured){
		case ClientManager::ClientEvents::CONNECT:
			cout << "A client connected to the server with that unique identifier: " << ce.client_id << endl;
			break;
		case ClientManager::ClientEvents::DISCONNECT:
			cout << "A client lost connection to server with that unique identifier: " << ce.client_id << endl;
			break;
		case  ClientManager::ClientEvents::UPDATE:
			cout << "A client sent a message to server with that unique identifier: " << ce.client_id << " and the message is: " << ce.message << endl;
			break;
		default: SDL_Delay(10);
		}
	}
	delete sm;
	SDL_Quit();

return 0;
}