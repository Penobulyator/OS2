#pragma once
#include <iostream>
#include <fstream>
#include <queue>
#include <string> 

#include "Cache.h"
#include "TcpSocket.h"
#include "HttpProxy.h"
class HttpProxy;
struct messageChunk;

class ClientSocketHandler
{
private:
	TcpSocket *clientSocket;
	TcpSocket *hostSocket = NULL;
	HttpProxy *proxy;

	bool parseRequest(char *request);

	void run();

	std::thread *runningThread;

public:
	ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy);
	~ClientSocketHandler();

	void setHostSocket(TcpSocket *hostSocket);

	int getClientFd();
};

