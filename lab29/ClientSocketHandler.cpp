#include "ClientSocketHandler.h"

ClientSocketHandler::ClientSocketHandler(TcpSocket * clientSocket, HttpProxy *proxy):
	clientSocket(clientSocket), proxy(proxy)
{

}

ClientSocketHandler::~ClientSocketHandler()
{
}

bool ClientSocketHandler::parseRequest(char *request) {
	//check if it's a GET request
	if (strstr(request, "GET") == request) {
		//find url
		char *urlStart = strstr(request, "http://") + 7;

		char *url = new char[2048];
		int i;
		for (i = 0; urlStart[i] != ' '; i++) {
			url[i] = urlStart[i];
		}
		url[i] = '\0';

		//notify proxy about new request
		proxy->gotNewRequest(this, url);
		return true;
	}
	else if (strstr(request, "CONNECT") == request) {
		return false;
	}

	return true;
}

bool ClientSocketHandler::recvChunk() {
	//read
	char *buf = new char[MAX_CHUNK_SIZE];
	int length = clientSocket->_read(buf, MAX_CHUNK_SIZE);

	if (length == 0) {
		return false;
	}


	//add chunk to message queue
	messageChunk chunk;
	chunk.buf = buf;
	chunk.length = length;
	messageQueue.push(chunk);


	return parseRequest(buf);
}

bool ClientSocketHandler::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.front();
		messageQueue.pop();

		int length = hostSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			return false;
		}
	}
	return true;
}

bool ClientSocketHandler::handle(PollResult pollResult)
{
	int fd = pollResult.fd;
	int revents = pollResult.revents;

	if (revents & POLLHUP) {

		//peer closed its end of the channel
		return false;
	}

	if (fd == clientSocket->fd) {
		if (revents & POLLIN) {

			//we can read a chunk from client
			return recvChunk();
		}
	}
	else if (hostSocket != NULL && fd == hostSocket->fd) {
		if (revents & POLLOUT) {

			//we can send a chunk to host
			return sendChunk();
		}
	}

	return true;
}

void ClientSocketHandler::setHostSocket(TcpSocket * hostSocket)
{
	this->hostSocket = hostSocket;
}

int ClientSocketHandler::getClientFd()
{
	return clientSocket->fd;
}
