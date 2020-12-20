#include "HttpProxy.h"

int main(int argc, char** argv){
	HttpProxy proxy(atoi(argv[1]));
	proxy.start();
}