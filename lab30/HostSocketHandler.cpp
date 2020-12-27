#include "HostSocketHandler.h"
#include "HostSocketHandler.h"

HostSocketHandler::HostSocketHandler(TcpSocket * clientSocket, TcpSocket * hostSocket, Cache *cache, HttpProxy * proxy)
{
	this->clientSocket = clientSocket;
	this->hostSocket = hostSocket;
	this->proxy = proxy;
	this->cache = cache;
	this->url = NULL;

	state = WAITING_FOR_RESPONCE;
}

HostSocketHandler::~HostSocketHandler()
{
}

void HostSocketHandler::recvChunk() {
	//read
	char *buf = new char[MAX_CHUNK_SIZE];
	int length = hostSocket->_read(buf, MAX_CHUNK_SIZE);

	if (length <= 0) {
		this->terminate();
	}

	//add chunk to cache
	messageChunk chunk;
	cache->addChunk(url, chunk);

	//write
	length = clientSocket->_write(buf, MAX_CHUNK_SIZE);

	if (length <= 0) {
		this->terminate();
	}
}

void HostSocketHandler::terminate()
{
	proxy->closeSession(clientSocket);
	std::terminate();
}

//
// returns false if session should be closed
//
void HostSocketHandler::run()
{
	while (true)
	{
		switch (state)
		{
		case WAITING_FOR_RESPONCE:
			state = READING_RESPONCE;
			proxy->gotNewResponce(this);
			recvChunk();
			break;

		case READING_RESPONCE:
			recvChunk();
			break;
		}
	}
}

void HostSocketHandler::finishReadingResponce()
{
	if (url != NULL) {
		//notify cache that entry with current url is full
		cache->makeEntryFull(url);

		std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " has finished reading " << url << std::endl;

		//delete thread;
	}
}

void HostSocketHandler::waitForNextResponce(char *url)
{
	this->url = url;

	std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " starting to read " << url << std::endl;

	cache->addEntry(url);


	state = WAITING_FOR_RESPONCE;

	thread = new std::thread(&HostSocketHandler::run, this);
	thread->detach();

}
