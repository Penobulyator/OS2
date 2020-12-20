#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "list.h"
#define A_SLEEP_TIME 1
#define B_SLEEP_TIME 2
#define C_SLEEP_TIME 3

#define PRODUSER_COUNT 5

typedef struct produserArg {
	int sleepTime;
	list *srcStoreList; //list<sem_t>
	sem_t *dstStore;
}produserArg;


void* producerThreadFunc(void *arg) {
	produserArg *myProduserArg = (produserArg*)arg;

	while (1) {
		sleep(myProduserArg->sleepTime);
		if (myProduserArg->srcStoreList != NULL)
			for (node *curNode = myProduserArg->srcStoreList->head; curNode != NULL; curNode = curNode->next) {
				sem_wait((sem_t*)curNode->data);
			}
		sem_post(myProduserArg->dstStore);
		printf("Posting in %p\n", myProduserArg->dstStore);
	}

	return NULL;
}

int main() {
	//init stores
	sem_t stores[PRODUSER_COUNT]; // {aStore, bStore, cStore, moduleStore, widgetStore}

	for (int i = 0; i < PRODUSER_COUNT; i++)
		sem_init(&stores[i], 0, 0);
	
	//init list of source stores for module producer
	list moduleSrcStoreList;
	initList(&moduleSrcStoreList);
	push(&moduleSrcStoreList, (void*)&stores[0]);
	push(&moduleSrcStoreList, (void*)&stores[1]);

	//init list of source stores for widget producer
	list widgetSrcStoreList;
	initList(&widgetSrcStoreList);
	push(&widgetSrcStoreList, (void*)&stores[2]);
	push(&widgetSrcStoreList, (void*)&stores[3]);

	//init arguments for producers
	produserArg produserArgs[PRODUSER_COUNT] = {
		{.sleepTime = A_SLEEP_TIME, NULL,.dstStore = &stores[0] },
		{.sleepTime = B_SLEEP_TIME, NULL,.dstStore = &stores[1] },
		{.sleepTime = C_SLEEP_TIME, NULL,.dstStore = &stores[2] },
		{.sleepTime = 0, &moduleSrcStoreList,.dstStore = &stores[3] },
		{.sleepTime = 0, &widgetSrcStoreList,.dstStore = &stores[4] }
	};

	//run producers
	pthread_t producerThreads[PRODUSER_COUNT]; 
	for (int i = 0; i < PRODUSER_COUNT; i++)
		pthread_create(&producerThreads[i], NULL, producerThreadFunc, &produserArgs[i]);

	//wait for 3 widgets to be done
	for(int i = 0; i < 3; i++){
		sem_wait(&stores[4]);
		printf("Got widget\n");
	}
	//destroy store lists
	destroyList(&moduleSrcStoreList);
	destroyList(&widgetSrcStoreList);

	//cancel producers
	for (int i = 0; i < PRODUSER_COUNT; i++)
		pthread_cancel(producerThreads[i]);

	//join producers
	for (int i = 0; i < PRODUSER_COUNT; i++)
		pthread_join(producerThreads[i], NULL);

	//destroy stores
	for (int i = 0; i < PRODUSER_COUNT; i++)
		sem_destroy(&stores[i]);

	return 0;
}
