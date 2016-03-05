#include "Socket.h"
#include <iostream>
#include <string.h>

using namespace std;

Error::Error(const char* err) : err(err){}

const char* Error::getError(){ return err; }

namespace ClientManager{

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
		server_event_mutex.lock();
		server_events.push(e);
		server_event_mutex.unlock();
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

}

namespace SocketManager{

	Client::Client(TCPsocket s){
		this->s = s;
		id = uid++;
	}
	char& Client::operator [](const int index){
		return buf[index];
	}
	TCPsocket& Client::operator *(){
		return s;
	}
	char* Client::operator &(){
		return buf;
	}
	unsigned int Client::operator ()(){
		return id;
	}
	int Client::operator +(){
		return buflen;
	}
	int Client::operator -(){
		return -buflen;
	}
	int& Client::operator +=(int i){
		buflen += i;
		return buflen;
	}
	int& Client::operator -=(int i){
		buflen -= i;
		return buflen;
	}

	void initSocketSystem(){
		IPaddress ipaddress;
		if (SDLNet_Init() == -1)throw Error(("SDLNet_Init: " + string(SDL_GetError())).c_str());
		if (SDLNet_ResolveHost(&ipaddress, NULL, 7777) == -1)throw Error(("SDLNet_ResolveHost: " + string(SDL_GetError())).c_str());
		if (!(server_socket = SDLNet_TCP_Open(&ipaddress)))throw Error(("SDLNet_TCP_Open: " + string(SDL_GetError())).c_str());
		if (!(set = SDLNet_AllocSocketSet(SOCKS)))throw Error(("SDLNet_AllocSocketSet: " + string(SDL_GetError())).c_str());
	}

	void deinitSocketSystem(){
		SDLNet_FreeSocketSet(set);
		SDLNet_TCP_Close(server_socket);
		SDLNet_Quit();
	}

	void processServerEvents(){
		while (1){
			ClientManager::ClientEvent ce = ClientManager::pollServerEvent();
			if (ce.event_occured == ClientManager::ClientEvents::NONE)break;
			try{
				Client cl = sockets.at(ce.client_id);
				int result = 0;
				switch (ce.event_occured){
				case  ClientManager::ClientEvents::UPDATE:
					result = SDLNet_TCP_Send(*cl, ce.message.c_str(), ce.message.length() + 1);
					if (result < ce.message.length() + 1) {
						const char* error = SDL_GetError();
						SDLNet_TCP_Close(*cl);
						num_clients = SDLNet_TCP_DelSocket(set, *cl);
						string error2 = "";
						if (num_clients == -1)error2 = SDL_GetError();
						ClientManager::addClientEvent({ ClientManager::ClientEvents::DISCONNECT, cl(), "" });
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
	}

	void acceptClients(){
		if (num_clients == SOCKS){
			cout << "socketset full!" << endl; //server socketset is full; should be avoided
			return;
		}
		TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);
		if (!client_socket){
			const char* error = SDL_GetError();
			if (strcmp("accept() failed", error)){
				throw Error(("SDLNet_TCP_Accept: " + string(error)).c_str());
			}
		}
		else {
			num_clients = SDLNet_TCP_AddSocket(set, client_socket);
			if (num_clients == -1)throw Error(("SDLNet_AddSocket: " + string(SDL_GetError())).c_str());
			Client cl = Client(client_socket);
			if (sockets.insert(pair<unsigned int, Client>(cl(), cl)).second == false)throw Error("Internal server error. Client connected with an existing socket. Aborting. 'sockets.insert(pair<unsigned int, Client>(cl(), cl))'");
			ClientManager::addClientEvent({ ClientManager::ClientEvents::CONNECT, cl(), "" });
		}
	}

	void extractUpdateBlocks(int last_message_len, Client& cl){
		int oldlen = +cl;
		cl += last_message_len;
		for (int i = oldlen; i < +cl; ++i){
			if (!cl[i]){
				i += 1;
				string chunk(&cl, i);
				memmove(&cl, &cl + i, +cl - i);
				cl -= i;
				ClientManager::addClientEvent({ ClientManager::ClientEvents::UPDATE, cl(), chunk });
				oldlen = 0;
				i = oldlen;
			}
		}
	}

	void processClientEvents(){
		if (num_clients){ //there are clients
			int ringing = SDLNet_CheckSockets(set, 0);
			if (ringing == -1)throw Error(("error in SDLNet_CheckSockets. Possible cause: " + string(SDL_GetError())).c_str());
			else if (ringing){
				map<unsigned int, Client>::iterator vit;
				for (vit = sockets.begin(); vit != sockets.end(); ++vit){
					Client& cl = vit->second;
					if (SDLNet_SocketReady(*cl)){
						int result = SDLNet_TCP_Recv(*cl, &cl[+cl], cl.MAXLEN + -cl);
						if (result <= 0) {
							const char* error = SDL_GetError();
							SDLNet_TCP_Close(*cl);
							num_clients = SDLNet_TCP_DelSocket(set, *cl);
							ClientManager::addClientEvent({ ClientManager::ClientEvents::DISCONNECT, cl(), "" });
							sockets.erase(vit);
							if (strcmp("accept() failed", error)){
								string error2 = "";
								if (num_clients == -1)error2 = SDL_GetError();
								throw Error(("SDLNet_TCP_Recv: " + string(error) + error2 == "" ? "" : " SDLNet_TCP_DelSocket: " + error2).c_str());
							}
							if (num_clients == -1)throw Error(("SDLNet_TCP_DelSocket: " + string(SDL_GetError())).c_str());
							break;
						}
						SocketManager::extractUpdateBlocks(result, cl);
					}
				}
			}
		}
	}

	void mainSocketThread(){
		try{
			SocketManager::initSocketSystem();
			while (!stop){
				SocketManager::acceptClients();
				SocketManager::processClientEvents();
				SocketManager::processServerEvents();
				SDL_Delay(1);
			}
		}
		catch (Error& e){
			cout << e.getError() << endl;
		}
		SocketManager::deinitSocketSystem();
	}
	void startManager(){
		start = thread(SocketManager::mainSocketThread);
		start.detach();
	}

	void startSocketManager(){
		call_once(one_thread, SocketManager::startManager);
	}

	void stopSocketManager(){
		stop = true;
	}

	unsigned int Client::uid = 0;

}
