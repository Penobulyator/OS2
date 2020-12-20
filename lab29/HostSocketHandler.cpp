#include "HostSocketHandler.h"
#include "HostSocketHandler.h"

HostSocketHandler::HostSocketHandler(TcpSocket * clientSocket, TcpSocket * hostSocket, Cache *cache, HttpProxy * proxy)
{
	this->clientSocket = clientSocket;
	this->hostSocket = hostSocket;
	this->proxy = proxy;
	this->cache = cache;

	state = WAITING_FOR_RESPONCE;
}

HostSocketHandler::~HostSocketHandler()
{
}

void HostSocketHandler::recvChunk() {
	//read
	char *buf = new char[CHUNK_SIZE];
	int length = hostSocket->_read(buf, CHUNK_SIZE);

	//buf[length] = '\0';
	//std::cout << buf;

	if (length == 0) {
		proxy->closeSession(hostSocket);
		return;
	}

	//add chunk to message queue
	messageChunk chunk;
	chunk.buf = buf;
	chunk.length = length;
	messageQueue.push(chunk);

	//add chunk to cache
	cache->addChunk(url, chunk);
}

void HostSocketHandler::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.back();
		messageQueue.pop();

		int length = clientSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			proxy->closeSession(clientSocket);
			return;
		}
	}
}

void HostSocketHandler::handle(PollResult pollResult)
{
	int fd = pollResult.fd;
	int revents = pollResult.revents;

	switch (state)
	{
	case WAITING_FOR_RESPONCE:
		if (fd == hostSocket->fd) {
			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(hostSocket);
			}
			else if (revents & POLLIN) {
				//we have a new request to read
				proxy->gotNewResponce(this);
				recvChunk();

				//change state
				state = READING_RESPONCE;
			}
		}
		break;

	case READING_RESPONCE:
		if (fd == hostSocket->fd) {

			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(hostSocket);
			}
			else if (revents & POLLIN) {

				//we can read a chunk from client
				recvChunk();

				//for client socket to be ready for write
				proxy->changeEvents(clientSocket, POLLIN | POLLHUP | POLLOUT);
			}
		}
		else if (fd == clientSocket->fd) {
			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(clientSocket);
			}
			else if (revents & POLLOUT) {

				//we can send a chunk to host
				sendChunk();

				//if queue is empty, don't wait for client socket to be ready for write
				if (messageQueue.empty()) {
					proxy->changeEvents(clientSocket, POLLIN | POLLHUP);
				}
			}
		}
		break;
	}
}

void HostSocketHandler::finishReadingResponce()
{
	if (url != NULL) {
		//notify cache that entry with current url is full
		cache->makeEntryFull(url);
	}
}



void HostSocketHandler::waitForNextResponce(char *url)
{
	this->url = url;

	cache->addEntry(url);

	state = WAITING_FOR_RESPONCE;
}
