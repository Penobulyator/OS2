#pragma once
#include <map>
#include <list>
#include <vector>
#include "CacheReader.h"
#define CHUNK_SIZE 333

class CacheReader;

typedef struct messageChunk {
	char *buf;
	int length;
}messageChunk;

typedef struct cacheEntry {
	char *url;
	std::vector<messageChunk> chunks;
	bool isFull;
}cacheEntry;

typedef struct listenerEntry {
	char *url;
	CacheReader *listener;
}waiterEntry;

class Cache
{
private:
	std::list<cacheEntry> entries;
	
	std::vector<listenerEntry> listeners;

public:

	void addEntry(char* url);

	void addChunk(char* url, messageChunk chunk);

	bool contains(char* url);

	std::vector<messageChunk> getChunks(char* url);

	bool entryIsFool(char* url);

	void makeEntryFull(char* url);

	void listenToUrl(char* url, CacheReader *listener);

	void stopListening(CacheReader *listener);

	Cache();
	~Cache();
};

