#include "CacheReader.h"



void CacheReader::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.back();
		messageQueue.pop();

		int length = writeSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			proxy->closeSession(writeSocket);
			return;
		}
	}
}


CacheReader::CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy)
{
	this->cache = cache;
	this->writeSocket = writeSocket;
	this->proxy = proxy;
}


CacheReader::~CacheReader()
{
}

void CacheReader::read(char * url)
{
	this->url = url;
	if (cache->entryIsFool(url)) {
		std::cout << "Cache entry for " << url << " is full, sending data from cache" << std::endl;
		std::vector<messageChunk> chunks = cache->getChunks(url);
		for (int i = 0; i < chunks.size(); i++) {
			messageQueue.push(chunks[i]);
		}
		proxy->changeEvents(writeSocket, POLLHUP | POLLIN | POLLOUT);
	}
	else {
		std::cout << "Cache entry for " << url << " isn't full, listening to this URL" << std::endl;
		cache->listenToUrl(url, this);
	}
}

void CacheReader::stopRead()
{
	url = NULL;
}

bool CacheReader::isReading()
{
	return url != NULL;
}

void CacheReader::notify() {
	std::vector<messageChunk> chunks = cache->getChunks(url);
	for (int i = 0; i < chunks.size(); i++) {
		messageQueue.push(chunks[i]);
	}
	cache->stopListening(this);
	proxy->changeEvents(writeSocket, POLLHUP | POLLIN | POLLOUT);
}

void CacheReader::handle(PollResult pollResult)
{
	if (pollResult.fd != writeSocket->fd)
		return;

	if (pollResult.revents & POLLHUP) {
		proxy->closeSession(writeSocket);
		return;
	}
	else if (pollResult.revents & POLLOUT) {
		std::cout << "Cache is sending chunk" << std::endl;
		sendChunk();

		if (messageQueue.empty()) {
			proxy->changeEvents(writeSocket, POLLHUP | POLLIN);
		}
	}
}
