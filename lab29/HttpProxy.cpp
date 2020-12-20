#include "HttpProxy.h"
#define SOCKET_EVENTS POLLIN | POLLHUP | POLLOUT
HttpProxy::HttpProxy(int listenPort):
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

	std::cout << "Adding client socket, fd = " << clientSocket->fd << std::endl;

	fcntl(clientSocket->fd, F_SETFL, O_NONBLOCK);
	poller.addFd(clientSocket->fd, SOCKET_EVENTS);

	//create proxy entry
	ProxyEntry proxyEntry;
	proxyEntry.clinetSocket = clientSocket;
	proxyEntry.clientSocketHandler = new ClientSocketHandler(clientSocket, this);
	proxyEntry.hostSocket = NULL;
	proxyEntry.hostSocketHandler = NULL;
	proxyEntry.cacheReader = new CacheReader(cache, clientSocket, this);

	proxyEntries.push_back(proxyEntry);
}

TcpSocket* HttpProxy::connectToHost(char * hostName)
{
	TcpSocket *socket = new TcpSocket();
	socket->_connect(hostName, 80);

	fcntl(socket->fd, F_SETFL, O_NONBLOCK);
	poller.addFd(socket->fd, SOCKET_EVENTS);

	return socket;
}

void HttpProxy::start()
{
	fcntl(serverSocket.fd, F_SETFL, O_NONBLOCK);
	poller.addFd(serverSocket.fd, POLLIN | POLLPRI);

	while(true) {
		poller._poll();
		std::vector<PollResult> pollResults = poller.getPollResult();

		for (int i = 0; i < pollResults.size(); i++) {

			if (pollResults[i].fd == serverSocket.fd && pollResults[i].fd & (POLLIN | POLLPRI)) {
				acceptClient();
			}
			else{

				for (ProxyEntry proxyEnrty : proxyEntries) {
					if (proxyEnrty.clinetSocket->fd == pollResults[i].fd || (proxyEnrty.hostSocket != NULL && proxyEnrty.hostSocket->fd == pollResults[i].fd)) {
						proxyEnrty.clientSocketHandler->handle(pollResults[i]);

						if (proxyEnrty.hostSocket != NULL) {
							proxyEnrty.hostSocketHandler->handle(pollResults[i]);
						}

						if (proxyEnrty.cacheReader->isReading()) {
							proxyEnrty.cacheReader->handle(pollResults[i]);
						}
					}

				}
			}
		}
	}
}

void HttpProxy::addHostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket)
{
	fcntl(hostSocket->fd, F_SETFL, O_NONBLOCK);
	poller.addFd(hostSocket->fd, SOCKET_EVENTS);

	for (int i = 0; i < proxyEntries.size(); i++) {
		if (proxyEntries[i].clinetSocket == clientSocket) {
			std::cout << "Adding host socket, fd = " << hostSocket->fd << std::endl;
			proxyEntries[i].hostSocket = hostSocket;
			proxyEntries[i].hostSocketHandler = new HostSocketHandler(clientSocket, hostSocket, cache, this);
			break;
		}
	}
}

void HttpProxy::closeSession(TcpSocket *socket)
{

	for (int i = 0; i < proxyEntries.size(); i++) {
		if (proxyEntries[i].clinetSocket == socket || proxyEntries[i].hostSocket == socket) {

			std::cout << "Closing client socket, fd = " << proxyEntries[i].clinetSocket->fd << std::endl;

			poller.removeFd(proxyEntries[i].clinetSocket->fd);
			proxyEntries[i].clinetSocket->_close();
			delete proxyEntries[i].clinetSocket;
			delete proxyEntries[i].clientSocketHandler;

			if (proxyEntries[i].hostSocket != NULL) {
				std::cout << "Closing host socket, fd = " << proxyEntries[i].hostSocket->fd << std::endl;
				poller.removeFd(proxyEntries[i].hostSocket->fd);
				proxyEntries[i].hostSocket->_close();
				delete proxyEntries[i].hostSocket;
				proxyEntries[i].hostSocketHandler->finishReadingResponce();
				delete proxyEntries[i].hostSocketHandler;
			}

			delete proxyEntries[i].cacheReader;

			proxyEntries.erase(proxyEntries.begin() + i);
			break;
		}
	}
}

void HttpProxy::gotNewRequest(ClientSocketHandler *clientSocketHandler, char *url)
{
	std::cout << "Got request for " << url << std::endl;
	if (!cache->contains(url)) {

		for (int i = 0; i < proxyEntries.size(); i++) {

			if (proxyEntries[i].clientSocketHandler == clientSocketHandler) {

				//create new hostSocketHandler if necessary
				if (proxyEntries[i].hostSocket == NULL) {

					//get host from url
					int hostNameLength = strstr(url, "/") - url;
					char *hostName = new char[hostNameLength + 1];
					memccpy(hostName, url, 0, hostNameLength);
					hostName[hostNameLength] = '\0';

					//open hostSocket
					TcpSocket *hostSocket = connectToHost(hostName);
					delete[] hostName;
					clientSocketHandler->setHostSocket(hostSocket);
					std::cout << "Adding host socket, fd = " << hostSocket->fd << ", hostname = " << hostName << std::endl;

					proxyEntries[i].hostSocket = hostSocket;
					proxyEntries[i].hostSocketHandler = new HostSocketHandler(proxyEntries[i].clinetSocket, proxyEntries[i].hostSocket, cache, this);
				}

				//stop response reading
				if (proxyEntries[i].cacheReader->isReading()) {
					proxyEntries[i].cacheReader->stopRead();
				}
				else {
					proxyEntries[i].hostSocketHandler->finishReadingResponce();
					proxyEntries[i].hostSocketHandler->waitForNextResponce(url);
				}
				break;
			}
		}
	}
	else {
		std::cout << "Data for " << url << " is in cache" << std::endl;
		for (int i = 0; i < proxyEntries.size(); i++) {
			if (proxyEntries[i].clientSocketHandler == clientSocketHandler) {
				proxyEntries[i].cacheReader->read(url);
			}
		}
	}
}

void HttpProxy::gotNewResponce(HostSocketHandler *hostSocketHandler)
{
	for (ProxyEntry proxyEntry : proxyEntries) {
		if (proxyEntry.hostSocketHandler == hostSocketHandler) {
			proxyEntry.clientSocketHandler->waitForRequest();
			break;
		}
	}
}

void HttpProxy::changeEvents(TcpSocket *socket, int events)
{
	poller.changeEvents(socket->fd, events);
}
