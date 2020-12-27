#include "CacheReader.h"

CacheReader::CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy)
{
	this->cache = cache;
	this->writeSocket = writeSocket;
	this->proxy = proxy;
	this->url = NULL;
}


CacheReader::~CacheReader()
{
}

void CacheReader::sendChunk(messageChunk chunk)
{
	int length = writeSocket->_write(chunk.buf, chunk.length);
	if (length == 0) {
		this->terminate();
	}
}

void CacheReader::terminate()
{
	proxy->closeSession(writeSocket);
	std::terminate();
}

void CacheReader::read(char * url)
{
	this->url = url;
	std::queue<messageChunk> chunksAvalible = cache->getChunks(url);
	while (!chunksAvalible.empty()) {
		messageChunk messageChunk = chunksAvalible.front();
		chunksAvalible.pop();

		sendChunk(messageChunk);
	}

	if (cache->entryIsFull(url)) {
		std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url  << " (entry for this URL is full)" << std::endl;
	}
	else {
		std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url << " (entry for this URL isn't full)" << std::endl;
		cache->listenToUrl(url, this);
	}

	exit = false;
	std::unique_lock<std::mutex> locker(exitMutex);
	while(!exit)
		exitCond.wait(locker);
	std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " has finished reading " << url << std::endl;
}

void CacheReader::stopRead()
{
	if (url != NULL) {
		cache->stopListening(this);
		url = NULL;
		exitCond.notify_one();
	}
}

bool CacheReader::isReading()
{
	return url != NULL;
}

void CacheReader::notify(messageChunk chunk){
	
}