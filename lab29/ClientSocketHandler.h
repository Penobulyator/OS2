#pragma once
#include <iostream>
#include <queue>

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

	ClinetSocketHandlerState state;

	std::queue<messageChunk> messageQueue;

	void readFirstRequestChunk();
	void recvChunk();

	void sendChunk();

	void parseRequest(char *request);

public:
	ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy);
	~ClientSocketHandler();

	void handle(PollResult pollResult);

	void waitForRequest();

	void setHostSocket(TcpSocket *hostSocket);
};

