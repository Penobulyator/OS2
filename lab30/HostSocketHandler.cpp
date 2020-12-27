#include "HostSocketHandler.h"
#include "HostSocketHandler.h"

HostSocketHandler::HostSocketHandler(TcpSocket * clientSocket, TcpSocket * hostSocket, Cache *cache, HttpProxy * proxy)
{
	this->clientSocket = clientSocket;
	this->hostSocket = hostSocket;
	this->proxy = proxy;
	this->cache = cache;
	this->url = NULL;

	this->runningThread = new std::thread(&HostSocketHandler::run, this);
}

HostSocketHandler::~HostSocketHandler()
{
}

void HostSocketHandler::run() {
	while (true) {
		//read
		char *buf = new char[MAX_CHUNK_SIZE];
		int length = hostSocket->_read(buf, MAX_CHUNK_SIZE);

		if (length <= 0) {
			proxy->closeSession(hostSocket);
			return;
		}

		//add chunk to cache
		messageChunk chunk;
		chunk.buf = buf;
		chunk.length = length;
		cache->addChunk(url, chunk);

		//write
		length = clientSocket->_write(buf, length);
		if (length <= 0) {
			proxy->closeSession(clientSocket);
			return;
		}
	}
}

void HostSocketHandler::finishReadingResponce()
{
	if (url != NULL) {
		//notify cache that entry with current url is full
		cache->makeEntryFull(url);

		std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " has finished reading " << url << std::endl;
	}
}

void HostSocketHandler::waitForNextResponce(char *url)
{
	this->url = url;

	std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " starting to read " << url << std::endl;

	cache->addEntry(url);
}
