#pragma once
#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

#include <queue>
#include <thread>
class HttpProxy;
class Cache;
struct messageChunk;

class HostSocketHandler
{
private:
	TcpSocket *clientSocket;
	TcpSocket *hostSocket;

	HttpProxy *proxy;

	Cache *cache;

	char* url;

	std::thread *runningThread;

	void run();
public:
	HostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket, Cache *cache, HttpProxy *proxy);
	~HostSocketHandler();

	void finishReadingResponce();

	void waitForNextResponce(char *url);
};

