#pragma once
#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

#include <queue>
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

	std::queue<messageChunk> messageQueue;

	HostSocketHandlerState state;

	Cache *cache;

	char* url;

	bool recvChunk();
	bool sendChunk();
public:
	HostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket, Cache *cache, HttpProxy *proxy);
	~HostSocketHandler();

	//
	// returns false if session should be closed
	//
	bool handle(PollResult pollResult);

	void finishReadingResponce();

	void waitForNextResponce(char *url);
};

