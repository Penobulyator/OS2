#pragma once
#include <queue>
#include <vector>
#include <iostream>
#include <condition_variable>
#include <mutex>

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

	bool exit = 0;
	std::mutex exitMutex;
	std::condition_variable exitCond;

	void sendChunk(messageChunk chunk);

	void terminate();

public:
	CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);
	~CacheReader();

	void read(char *url);

	void stopRead();

	bool isReading();

	void notify(messageChunk chunk);
};

