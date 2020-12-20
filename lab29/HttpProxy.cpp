#include "HttpProxy.h"
#define SOCKET_EVENTS POLLIN | POLLHUP | POLLOUT
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

	return socket;
}

void HttpProxy::start()
{
	fcntl(serverSocket.fd, F_SETFL, O_NONBLOCK);
	poller.addFd(serverSocket.fd, POLLIN | POLLPRI);

	while (true) {
		poller._poll();
		std::vector<PollResult> pollResults = poller.getPollResult();

		for (PollResult pollresult : pollResults) {

			if (pollresult.fd == serverSocket.fd && pollresult.fd & (POLLIN | POLLPRI)) {
				acceptClient();
			}
			else {

				for (int i = 0; i < proxyEntries.size(); i++) {
					ProxyEntry proxyEnrty = proxyEntries[i];
					if (proxyEnrty.clinetSocket->fd == pollresult.fd || (proxyEnrty.hostSocket != NULL && proxyEnrty.hostSocket->fd == pollresult.fd)) {
						int keepConnection;

						keepConnection = proxyEnrty.clientSocketHandler->handle(pollresult);
						if (!keepConnection) {
							closeSession(i);
							break;
						}


						if (proxyEnrty.hostSocket != NULL) {
							keepConnection = proxyEnrty.hostSocketHandler->handle(pollresult);
							if (!keepConnection) {
								closeSession(i);
								break;
							}
						}

						if (proxyEnrty.cacheReader->isReading()) {
							keepConnection = proxyEnrty.cacheReader->handle(pollresult);
							if (!keepConnection) {
								closeSession(i);
								break;
							}
						}
					}

				}
			}
		}
	}
}

void HttpProxy::closeSession(int proxyEntryIndex)
{

	ProxyEntry proxyEntry = proxyEntries[proxyEntryIndex];

	std::cout << "Closing client socket: fd = " << proxyEntry.clinetSocket->fd << std::endl;
	poller.removeFd(proxyEntry.clinetSocket->fd);
	proxyEntry.clinetSocket->_close();
	delete proxyEntry.clinetSocket;
	delete proxyEntry.clientSocketHandler;

	if (proxyEntry.hostSocket != NULL) {
		std::cout << "Closing host socket: fd = " << proxyEntry.hostSocket->fd << std::endl;
		poller.removeFd(proxyEntry.hostSocket->fd);
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
	std::cout << "Got request for " << url << std::endl;
	if (!cache->contains(url)) {

		for (ProxyEntry &proxyEntry : proxyEntries) {
			if (proxyEntry.clientSocketHandler == clientSocketHandler) {

				//create new hostSocketHandler if necessary
				if (proxyEntry.hostSocket == NULL) {

					//get host from url
					int hostNameLength = strstr(url, "/") - url;
					char *hostName = new char[hostNameLength + 1];
					memccpy(hostName, url, 0, hostNameLength);
					hostName[hostNameLength] = '\0';

					//open hostSocket
					TcpSocket *hostSocket = connectToHost(hostName);
					fcntl(hostSocket->fd, F_SETFL, O_NONBLOCK);
					poller.addFd(hostSocket->fd, SOCKET_EVENTS);
					std::cout << "Adding host socket: fd = " << hostSocket->fd << ", hostname = " << hostName << ", pair client socket = " << proxyEntry.clinetSocket->fd << std::endl;
					delete[] hostName;

					clientSocketHandler->setHostSocket(hostSocket);
					proxyEntry.hostSocket = hostSocket;

					proxyEntry.hostSocketHandler = new HostSocketHandler(proxyEntry.clinetSocket, proxyEntry.hostSocket, cache, this);
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