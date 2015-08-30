#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include "SDL_net.h"

using namespace std;

#ifndef _GLIBCXX_Socket_H
#define _GLIBCXX_Socket_H

class Error{
	const char* err;
public:
	Error(const char* err);
	const char* getError();
};


namespace ClientManager{

	enum ClientEvents{ CONNECT = 0, DISCONNECT, UPDATE, NONE };
	
	struct ClientEvent{
		ClientEvents event_occured;
		unsigned int client_id;
		string message;
	};

	ClientEvent pollClientEvent();
	void addServerEvent(ClientEvent e);
	void addClientEvent(ClientEvent e);
	ClientEvent pollServerEvent();

	namespace{
		queue<ClientEvent> client_events;
		queue<ClientEvent> server_events;
		mutex client_event_mutex;
		mutex server_event_mutex;
	}
}

namespace SocketManager{
	namespace{
		thread start;
		once_flag one_thread;
		bool stop = false;
		const int SOCKS = 16;
		class Client;

		TCPsocket server_socket;
		SDLNet_SocketSet set;
		map<unsigned int, Client> sockets;
		int num_clients = 0;

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
			Client(TCPsocket s);
			char& operator [](const int index);
			TCPsocket& operator *();
			char* operator &();
			unsigned int operator ()();
			int operator +();
			int operator -();
			int& operator +=(int i);
			int& operator -=(int i);

		};

		void mainSocketThread();
		void startManager();
		void initSocketSystem();
		void deinitSocketSystem();
		void processServerEvents();
		void acceptClients();
		void processClientEvents();
		void extractUpdateBlocks(int last_message_len, Client& cl);
	}
	void startSocketManager();
	void stopSocketManager();
}

#endif