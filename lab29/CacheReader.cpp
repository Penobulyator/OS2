#include "CacheReader.h"

bool CacheReader::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.front();
		messageQueue.pop();

		int length = writeSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			return false;
		}
	}
	return true;
}


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

void CacheReader::read(char * url)
{
	this->url = url;
	std::list<messageChunk> chunks = cache->getChunks(url);
	for (messageChunk messageChunk : chunks) {
		messageQueue.push(messageChunk);
	}
	proxy->changeEvents(writeSocket, POLLHUP | POLLIN | POLLOUT);

	if (cache->entryIsFull(url)) {
		std::cout << "Cache entry for " << url << " is full, sending data from cache" << std::endl;
	}
	else {
		std::cout << "Cache entry for " << url << " isn't full, listening to this URL" << std::endl;
		cache->listenToUrl(url, this);
	}
}

void CacheReader::stopRead()
{
	if (url != NULL) {
		cache->stopListening(this);
		std::cout << "Cache reader has finished reading " << url << std::endl;
		url = NULL;
	}
}

bool CacheReader::isReading()
{
	return url != NULL;
}

void CacheReader::notify(messageChunk chunk){

	messageQueue.push(chunk);
	proxy->changeEvents(writeSocket, POLLHUP | POLLIN | POLLOUT);
}

bool CacheReader::handle(PollResult pollResult)
{
	if (pollResult.fd != writeSocket->fd)
		return true;

	if (pollResult.revents & POLLHUP) {
		return false;
	}
	else if (pollResult.revents & POLLOUT) {
		if (!sendChunk())
			return false;

		if (messageQueue.empty()) {
			if (cache->entryIsFull(url)) {
				stopRead();
			}
			proxy->changeEvents(writeSocket, POLLHUP | POLLIN);
		}
	}

	return true;
}
