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

	bool recvFirstRequestChunk();
	bool recvChunk();

	bool sendChunk();

	bool parseRequest(char *request);


public:
	ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy);
	~ClientSocketHandler();

	//
	// returns false if session should be closed
	//
	bool handle(PollResult pollResult);

	void waitForRequest();

	void setHostSocket(TcpSocket *hostSocket);
};

