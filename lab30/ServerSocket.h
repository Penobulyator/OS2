#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>

#include "TcpSocket.h"

class ServerSocket
{
public:
	int fd;

	ServerSocket(int port);
	~ServerSocket();

	TcpSocket _accept();

	void _close();
};

