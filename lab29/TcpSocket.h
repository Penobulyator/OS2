#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
class TcpSocket
{
private:
	void resolveHostName(char * hostName, addrinfo **res);

public:
	int fd;

	void _connect(char* hostName, int port);

	int _read(char* buf, int length);

	int _write(char* buf, int length);

	void _close();

	char *hostName;

	TcpSocket();
	TcpSocket(int fd);
	~TcpSocket();

	char* getHostName();
};

