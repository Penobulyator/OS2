#include "ClientSocketHandler.h"

ClientSocketHandler::ClientSocketHandler(TcpSocket * clientSocket, HttpProxy *proxy):
	clientSocket(clientSocket), proxy(proxy)
{
	this->runningThread = new std::thread(&ClientSocketHandler::run, this);
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
	}
	else if (strstr(request, "CONNECT") == request) {
		return false;
	}

	return true;

}

void ClientSocketHandler::run() {
	while (true) {

		//read
		char *buf = new char[MAX_CHUNK_SIZE];
		int length = clientSocket->_read(buf, MAX_CHUNK_SIZE);
		if (length <= 0) {
			proxy->closeSession(clientSocket);
			return;
		}
		
		//parse
		if (!parseRequest(buf)) {
			proxy->closeSession(clientSocket);
			return;
		}

		//write
		if (hostSocket != NULL) {
			length = hostSocket->_write(buf, length);
			if (length <= 0) {
				proxy->closeSession(hostSocket);
				return;
			}
		}
	}
}

void ClientSocketHandler::setHostSocket(TcpSocket * hostSocket)
{
	this->hostSocket = hostSocket;
}

int ClientSocketHandler::getClientFd()
{
	return clientSocket->fd;
}
