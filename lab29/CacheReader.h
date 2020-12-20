#pragma once
#include <queue>
#include <vector>
#include <iotream>

#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

class HttpProxy;
class Cache;
struct messageChunk;

class CacheReader
{
private:
	Cache *cache;

	TcpSocket *writeSocket;

	HttpProxy *proxy;

	char *url;

	std::queue<messageChunk> messageQueue;

	bool sendChunk();

public:
	CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);
	~CacheReader();

	void read(char *url);

	void stopRead();

	bool isReading();

	void notify(messageChunk chunk);

	//
	// returns false if session should be closed
	//
	bool handle(PollResult pollresult);
};

