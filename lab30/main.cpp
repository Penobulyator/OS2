#include "HttpProxy.h"
#include <csignal>

HttpProxy *proxy;

void sigHandler(int signal) {
	proxy->clear();
	delete proxy;
	exit(0);
}

int main(int argc, char** argv){
	proxy = new HttpProxy(atoi(argv[1]));
	std::signal(SIGINT, sigHandler);
	proxy->start();
	return 0;
}