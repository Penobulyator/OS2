#pragma once
#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

#include <queue>
#include <pthread.h>
class HttpProxy;
class Cache;
struct messageChunk;

enum HostSocketHandlerState
{
	WAITING_FOR_RESPONCE,
	READING_RESPONCE
};

class HostSocketHandler
{
private:
	TcpSocket *clientSocket;
	TcpSocket *hostSocket;

	HttpProxy *proxy;

	HostSocketHandlerState state;

	Cache *cache;

	std::thread *thread = NULL;

	char* url;

	void recvChunk();

	void terminate();
public:
	HostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket, Cache *cache, HttpProxy *proxy);
	~HostSocketHandler();

	//
	// returns false if session should be closed
	//
	void run();

	void finishReadingResponce();

	void waitForNextResponce(char *url);
};

