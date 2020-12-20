#include "ClientSocketHandler.h"

ClientSocketHandler::ClientSocketHandler(TcpSocket * clientSocket, HttpProxy *proxy):
	clientSocket(clientSocket), proxy(proxy)
{
	waitForRequest();
}

ClientSocketHandler::~ClientSocketHandler()
{
}

void ClientSocketHandler::parseRequest(char *request) {
	//find url
	char *urlStart = strstr(request, "http://");
	int urlLength = strstr(urlStart, " ") - urlStart - 7;

	char *url = new char[urlLength + 1];
	memccpy(url, urlStart + 7, 0, urlLength);
	url[urlLength] = '\0';

	//notify proxy about new request
	proxy->gotNewRequest(this, url);
}


void ClientSocketHandler::readFirstRequestChunk() {
	//read
	char *buf = new char[CHUNK_SIZE];
	int length = clientSocket->_read(buf, CHUNK_SIZE);

	//buf[length] = '\0';
	//std::cout << buf;
	
	if (length == 0) {
		proxy->closeSession(clientSocket);
		return;
	}

	parseRequest(buf);

	//add chunk to message queue
	messageChunk chunk;
	chunk.buf = buf;
	chunk.length = length;
	messageQueue.push(chunk);
}

void ClientSocketHandler::recvChunk() {
	//read
	char *buf = new char[CHUNK_SIZE];
	int length = clientSocket->_read(buf, CHUNK_SIZE);

	if (length == 0) {
		proxy->closeSession(clientSocket);
		return;
	}

	//add chunk to message queue
	messageChunk chunk;
	chunk.buf = buf;
	chunk.length = length;
	messageQueue.push(chunk);
}

void ClientSocketHandler::sendChunk()
{
	if (!messageQueue.empty()) {
		messageChunk chunk = messageQueue.back();
		messageQueue.pop();

		int length = hostSocket->_write(chunk.buf, chunk.length);
		if (length == 0) {
			proxy->closeSession(hostSocket);
			return;
		}
	}
}

void ClientSocketHandler::handle(PollResult pollResult)
{
	int fd = pollResult.fd;
	int revents = pollResult.revents;
	switch (state)
	{
	case WAITING_FOR_REQUEST:
		if (fd == clientSocket->fd) {
			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(clientSocket);
			}
			else if (revents & POLLIN) {

				//we have a new request to read
				readFirstRequestChunk();

				state = READING_REQUEST;
			}
		}

		break;

	case READING_REQUEST:
		if (fd == clientSocket->fd) {

			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(clientSocket);
			}
			else if (revents & POLLIN) {

				//we can read a chunk from client
				recvChunk();

				//wait for host socket to be ready for write
				if (hostSocket != NULL)
					proxy->changeEvents(hostSocket, POLLIN | POLLHUP | POLLOUT);
			}
		}
		else if (hostSocket != NULL && fd == hostSocket->fd) {
			if (revents & POLLHUP) {

				//peer closed its end of the channel
				proxy->closeSession(hostSocket);
			}
			else if (revents & POLLOUT) {

				//we can send a chunk to host
				sendChunk();

				//if queue is empty, don't wait for host socket to be ready for write
				if (messageQueue.empty()) {
					proxy->changeEvents(hostSocket, POLLIN | POLLHUP);
				}
			}
		}
		break;
	}
}

void ClientSocketHandler::waitForRequest()
{
	state = WAITING_FOR_REQUEST;
}

void ClientSocketHandler::setHostSocket(TcpSocket * hostSocket)
{
	this->hostSocket = hostSocket;
}
