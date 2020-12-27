all: proxy

proxy: main.o ClientSocketHandler.o HostSocketHandler.o HttpProxy.o ServerSocket.o TcpSocket.o Cache.o CacheReader.o
	g++ -std=c++0x main.o ClientSocketHandler.o HostSocketHandler.o HttpProxy.o ServerSocket.o TcpSocket.o Cache.o CacheReader.o -o proxy -lsocket -lnsl

main.o: main.cpp
	g++ -c main.cpp

ClientSocketHandler.o: ClientSocketHandler.cpp
	g++ -c ClientSocketHandler.cpp

HostSocketHandler.o: HostSocketHandler.cpp
	g++ -c HostSocketHandler.cpp

HttpProxy.o: HttpProxy.cpp
	g++ -c HttpProxy.cpp

Cache.o: Cache.cpp
	g++ -c Cache.cpp

CacheReader.o: CacheReader.cpp
	g++ -c CacheReader.cpp

ServerSocket.o: ServerSocket.cpp
	g++ -c ServerSocket.cpp

TcpSocket.o: TcpSocket.cpp
	g++ -c TcpSocket.cpp

clean:
	rm *.o 
