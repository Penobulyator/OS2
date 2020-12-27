#pragma once
#include <queue>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

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

	bool queueHasData = false;
	std::mutex queueMutex;
	std::condition_variable queueCondVar;
	std::queue<messageChunk> messageQueue;

	std::thread *runningThread;

	void run();

public:
	CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);
	~CacheReader();

	void read(char *url);

	void stopRead();

	bool isReading();

	void notify(messageChunk chunk);
};

