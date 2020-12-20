#include "list.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_STR_SIZE 80
#define SORTERS_COUNT 1

typedef struct sorterArg {
	list *listToSort;
	int sleepTime;
}sorterArg;

char* pointerToStr(void* pointer) {
	return (char*)pointer;
}

int srtCompare(const void *str1, const void *str2) {
	return strcmp((char*)str1, (char*)str2);
}

void *sortThread(void *arg) {
	sorterArg *mySorterArg = (sorterArg*)arg;
	while (1) {
		sleep(mySorterArg->sleepTime);
		sort(mySorterArg->listToSort, srtCompare);
		//pthread_testcancel();
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
		else if (strcmp(str, ".\n") == 0) {
			print(&strList, pointerToStr);
		}
		else {
			push(&strList, (void*)str);
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