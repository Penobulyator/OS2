#include "list.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_STR_SIZE 80
#define SORTERS_COUNT 10

typedef struct sorterArg {
	list *listToSort;
	int sleepTime;
}sorterArg;

int srtCompare(const void *str1, const void *str2) {
	return strcmp((char*)str1, (char*)str2);
}

void *sortThread(void *arg) {
	sorterArg *mySorterArg = (sorterArg*)arg;
	while (1) {
		sleep(mySorterArg->sleepTime);
		sort(mySorterArg->listToSort, srtCompare);
	}
}

int main() {
	list strList;
	initList(&strList);

	pthread_t sorterThread[SORTERS_COUNT];
	sorterArg args[SORTERS_COUNT];
	for (int i = 0; i < SORTERS_COUNT; i++) {
		args[i].listToSort = &strList;
		args[i].sleepTime = 1 + i % 3;
	}

	for (int i = 0; i < SORTERS_COUNT; i++)
		pthread_create(&sorterThread[i], NULL, sortThread, &args[i]);

	while (1) {
		char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
		fgets(str, MAX_STR_SIZE, stdin);
		if (str[0] == '\n') {
			break;
		}
		else {
			push(&strList, (void*)str);
			//printf("%s", (char*)pop(&strList));
		}
	}

	for (int i = 0; i < SORTERS_COUNT; i++)
		pthread_cancel(sorterThread[i]);

	for (int i = 0; i < SORTERS_COUNT; i++)
		pthread_join(sorterThread[i], NULL);

	printf("\nSorted:\n");
	while (!isEmpty(&strList)) {
		char* str = (char*)pop(&strList);
		printf("%s", str);
		free(str);
	}

	return 0;
}