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
	ofstream = new std::ofstream("index.html");

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
	cache->stopListening(this);
	url = NULL;
	ofstream->close();
	delete ofstream;
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
			proxy->changeEvents(writeSocket, POLLHUP | POLLIN);
		}
	}

	return true;
}
