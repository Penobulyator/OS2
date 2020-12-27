#include "HttpProxy.h"
HttpProxy::HttpProxy(int listenPort) :
	serverSocket(listenPort)
{
	cache = new Cache();
}

HttpProxy::~HttpProxy()
{
}

void HttpProxy::acceptClient() {

	//accept
	TcpSocket *clientSocket = new TcpSocket(serverSocket._accept());

	if (clientSocket->fd == -1) {
		perror("accept");
	}

	std::cout << "Adding client socket, fd = " << clientSocket->fd << std::endl;
	std::unique_lock<std::mutex> locker(proxyEntriesMutex);
	//create proxy entry
	ProxyEntry proxyEntry;
	proxyEntry.clinetSocket = clientSocket;
	proxyEntry.clientSocketHandler = new ClientSocketHandler(clientSocket, this);
	proxyEntry.hostSocket = NULL;
	proxyEntry.hostSocketHandler = NULL;
	proxyEntry.cacheReader = new CacheReader(cache, clientSocket, this);

	proxyEntries.push_back(proxyEntry);
}

void HttpProxy::start()
{
	while (true) {	
		acceptClient();
	}
}

void HttpProxy::closeSession(int proxyEntryIndex)
{
	ProxyEntry proxyEntry = proxyEntries[proxyEntryIndex];

	std::cout << "Closing client socket: fd = " << proxyEntry.clinetSocket->fd << std::endl;
	proxyEntry.clinetSocket->_close();
	delete proxyEntry.clinetSocket;
	delete proxyEntry.clientSocketHandler;

	if (proxyEntry.hostSocket != NULL) {
		std::cout << "Closing host socket: fd = " << proxyEntry.hostSocket->fd << std::endl;
		proxyEntry.hostSocket->_close();
		delete proxyEntry.hostSocket;
		proxyEntry.hostSocketHandler->finishReadingResponce();
		delete proxyEntry.hostSocketHandler;
	}

	if (proxyEntry.cacheReader->isReading()) {
		proxyEntry.cacheReader->stopRead();
	}
	delete proxyEntry.cacheReader;

	proxyEntries.erase(proxyEntries.begin() + proxyEntryIndex);
}


void HttpProxy::gotNewRequest(ClientSocketHandler *clientSocketHandler, char *url)
{
	std::unique_lock<std::mutex> locker(proxyEntriesMutex);
	std::cout << "Got request for " << url << " from client socket with fd = " << clientSocketHandler->getClientFd() << std::endl;
	if (true) {
		for (ProxyEntry &proxyEntry : proxyEntries) {
			if (proxyEntry.clientSocketHandler == clientSocketHandler) {

				//get host from url
				int hostNameLength = strstr(url, "/") - url;
				char *hostName = new char[hostNameLength + 1];
				memccpy(hostName, url, 0, hostNameLength);
				hostName[hostNameLength] = '\0';
				if (proxyEntry.hostSocket == NULL) {
					//entry has no host socket, open socket and connect to host

					//open hostSocket
					proxyEntry.hostSocket = new TcpSocket();
					proxyEntry.hostSocket->_connect(hostName, 80);
					std::cout << "Adding host socket: fd = " << proxyEntry.hostSocket->fd << ", hostname = " << hostName << ", pair client socket = " << proxyEntry.clinetSocket->fd << std::endl;
					

					clientSocketHandler->setHostSocket(proxyEntry.hostSocket);

					proxyEntry.hostSocketHandler = new HostSocketHandler(proxyEntry.clinetSocket, proxyEntry.hostSocket, cache, this);
				}
				else if (strcmp(hostName, proxyEntry.hostSocket->getHostName()) != 0) {
					//client requested data from other host, reconnect host socket
					std::cout << "Reconnecting host socket with fd = " << proxyEntry.hostSocket->fd << " from " << proxyEntry.hostSocket->getHostName() << " to " << hostName << std::endl;

					//close old socket
					int oldFd = proxyEntry.hostSocket->fd;
					proxyEntry.hostSocket->_close();
					delete proxyEntry.hostSocket;

					//open new socket
					proxyEntry.hostSocket = new TcpSocket();
					proxyEntry.hostSocket->_connect(hostName, 80);
					clientSocketHandler->setHostSocket(proxyEntry.hostSocket);

					std::cout << "New fd for host socket with fd = " << oldFd << " is " << proxyEntry.hostSocket->fd << std::endl;
				}

				//stop response reading
				if (proxyEntry.cacheReader->isReading()) {
					proxyEntry.cacheReader->stopRead();
				}
				else {
					proxyEntry.hostSocketHandler->finishReadingResponce();
					proxyEntry.hostSocketHandler->waitForNextResponce(url);
				}
				break;
			}
		}
	}
	else {
		std::cout << "Data for " << url << " is in cache" << std::endl;
		for (ProxyEntry &proxyEntry : proxyEntries) {
			if (proxyEntry.clientSocketHandler == clientSocketHandler) {
				proxyEntry.cacheReader->read(url);
			}
		}
	}

}

void HttpProxy::closeSession(TcpSocket * socket)
{
	std::unique_lock<std::mutex> locker(proxyEntriesMutex);
	for (int i = 0; i < proxyEntries.size(); i++) {
		if (proxyEntries[i].clinetSocket == socket || proxyEntries[i].hostSocket == socket) {
			closeSession(i);
			return;
		}
	}
}

void HttpProxy::clear()
{
	std::unique_lock<std::mutex> locker(proxyEntriesMutex);
	for (ProxyEntry proxyEntry : proxyEntries) {
		delete proxyEntry.cacheReader;

		proxyEntry.clinetSocket->_close();
		delete proxyEntry.clinetSocket;
		delete proxyEntry.clientSocketHandler;

		if (proxyEntry.hostSocket != NULL) {
			proxyEntry.hostSocket->_close();
			delete proxyEntry.hostSocket;
			delete proxyEntry.hostSocketHandler;
		}
	}

	serverSocket._close();
	cache->clear();
}
