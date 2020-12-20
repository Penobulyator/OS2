#pragma once
#include <queue>
#include <vector>
#include <fstream>

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

	void sendChunk();

	std::ofstream *ofstream;
public:
	CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);
	~CacheReader();

	void read(char *url);

	void stopRead();

	bool isReading();

	void notify(messageChunk chunk);

	void handle(PollResult pollresult);
};

