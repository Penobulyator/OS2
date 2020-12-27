#pragma once
#include <fcntl.h>
#include <thread>

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
	ServerSocket serverSocket;

	Cache *cache;

	std::vector<ProxyEntry> proxyEntries;


	void closeProxyEntry(int proxyEntryIndex);

	void acceptClient();

public:
	HttpProxy(int listenPort);
	~HttpProxy();

	void start();

	void gotNewRequest(ClientSocketHandler *clientSocketHandler, char *url);

	void gotNewResponce(HostSocketHandler *hostSocketHandler);

	void closeSession(TcpSocket *socket);

	void clear();
};

