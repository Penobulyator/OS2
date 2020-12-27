#include "CacheReader.h"

CacheReader::CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy)
{
	this->cache = cache;
	this->writeSocket = writeSocket;
	this->proxy = proxy;
	this->url = NULL;

	//this->runningThread = new std::thread(&CacheReader::run, this);
}

CacheReader::~CacheReader()
{
}

void CacheReader::read(char * url)
{
	this->url = url;
	std::unique_lock<std::mutex> locker(queueMutex);

	std::queue<messageChunk> curChunks = cache->getChunks(url);
	//proxy->changeEvents(writeSocket, POLLHUP | POLLIN | POLLOUT);

	if (cache->entryIsFull(url)) {
		std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url  << " (entry for this URL is full)" << std::endl;
	}
	else {
		std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url << " (entry for this URL isn't full)" << std::endl;
		cache->listenToUrl(url, this);
	}

	queueHasData = true;
	queueCondVar.notify_one();
}

void CacheReader::run()
{
	std::cout << "Cache reader thread with write socket fd = " << writeSocket->fd << " is running " << std::endl;
	while (true)
	{
		//wait untill queue is not empty
		std::cout << "Here0" << std::endl;
		while (!queueHasData) {
			//std::cout << "Here1" << std::endl;
			//queueCondVar.wait(locker);
			//std::cout << "Here2" << std::endl;
		}
		std::cout << "Here3" << std::endl;

		//send chunks from queue
		while (!messageQueue.empty()) {

			messageChunk chunk = messageQueue.front();
			messageQueue.pop();

			int length = writeSocket->_write(chunk.buf, chunk.length);
			if (length == 0) {
				proxy->closeSession(writeSocket);
			}
		}
		queueHasData = false;
	}
}

void CacheReader::stopRead()
{
	if (url != NULL) {
		cache->stopListening(this);
		std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " has finished reading " << url << std::endl;
		url = NULL;
	}
}

bool CacheReader::isReading()
{
	return url != NULL;
}

void CacheReader::notify(messageChunk chunk){

	std::unique_lock<std::mutex> locker(queueMutex);
	messageQueue.push(chunk);
	queueHasData = true;
	queueCondVar.notify_one();
}
