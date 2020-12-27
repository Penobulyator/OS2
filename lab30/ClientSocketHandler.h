#pragma once
#include <iostream>
#include <fstream>
#include <queue>
#include <string> 
#include <pthread.h>

#include "Cache.h"
#include "TcpSocket.h"
#include "HttpProxy.h"
class HttpProxy;
struct messageChunk;

enum ClinetSocketHandlerState
{
	WAITING_FOR_REQUEST,
	READING_REQUEST
};

class ClientSocketHandler
{
private:
	TcpSocket *clientSocket;
	TcpSocket *hostSocket = NULL;
	HttpProxy *proxy;

	std::thread *thread = NULL;

	ClinetSocketHandlerState state;

	void recvFirstRequestChunk();
	void recvChunk();

	void parseRequest(char *request);

	void terminate();
public:
	ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy);
	~ClientSocketHandler();

	void run();

	void startThread();

	void waitForRequest();

	void setHostSocket(TcpSocket *hostSocket);

	int getClientFd();
};

