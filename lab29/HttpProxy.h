#pragma once
#include <fcntl.h>

#include "Poller.h"
#include "ServerSocket.h"
#include "ClientSocketHandler.h"
#include "HostSocketHandler.h"
#include "CacheReader.h"

class ClientSocketHandler;
class HostSocketHandler;
class Cache;
class CacheReader;

typedef struct ProxyEntry
{
	TcpSocket *clinetSocket;
	TcpSocket *hostSocket;
	
	ClientSocketHandler *clientSocketHandler;
	HostSocketHandler *hostSocketHandler;

	CacheReader *cacheReader;
}ProxyEntry;

class HttpProxy
{

private:
	Poller poller;

	ServerSocket serverSocket;

	Cache *cache;

	std::vector<ProxyEntry> proxyEntries;

	void closeSession(int proxyEntryIndex);

	void acceptClient();

	TcpSocket* connectToHost(char* hostName);

public:
	HttpProxy(int listenPort);
	~HttpProxy();

	void start();

	void gotNewRequest(ClientSocketHandler *clientSocketHandler, char *url);

	void gotNewResponce(HostSocketHandler *hostSocketHandler);

	void changeEvents(TcpSocket *socket, int events);

	void clear();
};

