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

bool HostSocketHandler::recvChunk() {
	//read
	char *buf = new char[MAX_CHUNK_SIZE];
	int length = hostSocket->_read(buf, MAX_CHUNK_SIZE);

	if (length == 0) {
		return false;
	}

	//add chunk to message queue
	messageChunk chunk;
	chunk.buf = buf;
	chunk.length = length;
	messageQueue.push(chunk);

	//add chunk to cache
	cache->addChunk(url, chunk);

	return true;
}

bool HostSocketHandler::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.front();
		messageQueue.pop();

		int length = clientSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			return false;
		}
	}
	return true;
}

//
// returns false if session should be closed
//
bool HostSocketHandler::handle(PollResult pollResult)
{
	int fd = pollResult.fd;
	int revents = pollResult.revents;

	switch (state)
	{
	case WAITING_FOR_RESPONCE:
		if (fd == hostSocket->fd) {
			if (revents & POLLHUP) {

				//peer closed its end of the channel
				return false;
			}
			else if (revents & POLLIN) {
				//we have a new request to read

				//change state
				state = READING_RESPONCE;
				proxy->gotNewResponce(this);

				return recvChunk();

			}
		}
		break;

	case READING_RESPONCE:
		if (fd == hostSocket->fd) {

			if (revents & POLLHUP) {

				//peer closed its end of the channel
				return false;
			}
			else if (revents & POLLIN) {
		
				//we can read a chunk from client
				return recvChunk();

			}
		}
		else if (fd == clientSocket->fd) {
			if (revents & POLLHUP) {

				return false;
			}
			else if (revents & POLLOUT) {

				//we can send a chunk to host
				return sendChunk();
			}
		}
	}

	return true;
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

	state = WAITING_FOR_RESPONCE;
}
