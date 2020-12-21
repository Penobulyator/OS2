#include "ServerSocket.h"
#include <iostream>
ServerSocket::ServerSocket(int port)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(fd, 10);
}

ServerSocket::~ServerSocket()
{
}

TcpSocket ServerSocket::_accept()
{
	int socket = accept(fd, (struct sockaddr*)NULL, NULL);
	return TcpSocket(socket);
}

void ServerSocket::_close()
{
	close(fd);
}
