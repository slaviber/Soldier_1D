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


class ClientManager{

public:
	struct ClientEvent;
private:

	static queue<ClientEvent> client_events;
	static queue<ClientEvent> server_events;
	static mutex client_event_mutex;
	static mutex server_event_mutex;

public:
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
};

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

	static void mainSocketThread();
	static void startManager(SocketManager* self);
public:
	SocketManager();
	~SocketManager();
};

#endif