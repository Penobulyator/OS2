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
	//check if it's a GET request
	if (strstr(request, "GET") != request) {
		this->terminate();
	}

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
}

void ClientSocketHandler::terminate()
{
	proxy->closeSession(clientSocket);
	std::terminate();
}


void ClientSocketHandler::recvFirstRequestChunk() {
	//read
	char *buf = new char[MAX_CHUNK_SIZE];
	int length = clientSocket->_read(buf, MAX_CHUNK_SIZE);
	
	if (length <= 0) {
		this->terminate();
	}

	//parse
	parseRequest(buf);

	//write
	length = hostSocket->_write(buf, MAX_CHUNK_SIZE);

	if (length <= 0) {
		this->terminate();
	}
}

void ClientSocketHandler::recvChunk() {
	//read
	char *buf = new char[MAX_CHUNK_SIZE];
	int length = clientSocket->_read(buf, MAX_CHUNK_SIZE);

	if (length <= 0) {
		this->terminate();
	}

	//write
	length = hostSocket->_write(buf, MAX_CHUNK_SIZE);

	if (length <= 0) {
		this->terminate();
	}
}

void ClientSocketHandler::run()
{
	while (true)
	{
		switch (state)
		{
		case WAITING_FOR_REQUEST:
			state = READING_REQUEST;

			recvFirstRequestChunk();
			break;

		case READING_REQUEST:
			recvChunk();
			break;
		}
	}
}

void ClientSocketHandler::startThread()
{
	if (thread != NULL)
		delete thread;
	thread = new std::thread(&ClientSocketHandler::run, this);
}

void ClientSocketHandler::waitForRequest()
{
	state = WAITING_FOR_REQUEST;

	//delete thread;
	thread = new std::thread(&ClientSocketHandler::run, this);
	thread->detach();
}

void ClientSocketHandler::setHostSocket(TcpSocket * hostSocket)
{
	this->hostSocket = hostSocket;
}

int ClientSocketHandler::getClientFd()
{
	return clientSocket->fd;
}
