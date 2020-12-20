#include "HttpProxy.h"

int main(int argc, char** argv){
	HttpProxy proxy(10000);
	proxy.start();
}