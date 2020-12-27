#include "Cache.h"

void Cache::addEntry(char * url)
{
	cacheEntry cacheEntry;
	cacheEntry.url = url;
	cacheEntry.isFull = false;

	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	entries.push_front(cacheEntry);
}

void Cache::addChunk(char * url, messageChunk chunk)
{
	//push chunk
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry& it: entries) {
		if (strcmp(it.url, url) == 0) {
			it.chunks.push(chunk);
			break;
		}
	}

	//notify listeners
	std::unique_lock<std::mutex> listenersLocker(listenersMutex);
	for (listenerEntry listenerEntry : listeners) {
		if (strcmp(listenerEntry.url, url) == 0) {
			listenerEntry.listener->notify(chunk);
		}
	}
}

bool Cache::contains(char * url){
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry& it: entries) {
		if (strcmp(it.url, url) == 0) {
			return true;
		}
	}
	return false;
}

std::queue<messageChunk> Cache::getChunks(char * url)
{
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry& it: entries) {
		if (strcmp(it.url, url) == 0) {
			return it.chunks;
		}
	}

	return std::queue<messageChunk>();
}

bool Cache::entryIsFull(char * url)
{
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry& it: entries) {
		if (strcmp(it.url, url) == 0) {
			return it.isFull;
		}
	}

	return false;
}

void Cache::makeEntryFull(char * url)
{
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry& ref: entries) {
		if (strcmp(ref.url, url) == 0) {
			ref.isFull = true;
			return;
		}
	}
}

void Cache::listenToUrl(char * url, CacheReader * listener)
{
	listenerEntry listenerEntry;
	listenerEntry.url = url;
	listenerEntry.listener = listener;

	std::unique_lock<std::mutex> listenersLocker(listenersMutex);
	listeners.push_back(listenerEntry);
}

void Cache::stopListening(CacheReader * listener)
{
	std::unique_lock<std::mutex> listenersLocker(listenersMutex);
	for (int i = 0; i < listeners.size(); i++) {
		if (listeners[i].listener == listener) {
			listeners.erase(listeners.begin() + i);
		}
	}
}

void Cache::clear()
{
	std::unique_lock<std::mutex> entriesLocker(entriesMutex);
	for (cacheEntry cacheEntry : entries) {
		delete[] cacheEntry.url;
		while (!cacheEntry.chunks.empty()) {
			delete[] cacheEntry.chunks.front().buf;
			delete[] cacheEntry.url;
			cacheEntry.chunks.pop();
		}
	}
}

Cache::Cache()
{
}


Cache::~Cache()
{
}
