#include "list.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_STR_SIZE 80

int srtCompare(const void *str1, const void *str2) {
	return strcmp((char*)str1, (char*)str2);
}

void *sortThread(void *arg) {
	list* sortingList = (list*)arg;
	while (1) {
		sleep(1);
		sort(sortingList, srtCompare);
		pthread_testcancel();
	}
}

int main() {
	list strList;
	initList(&strList);

	pthread_t sorterThread;
	pthread_create(&sorterThread, NULL, sortThread, &strList);

	while (1) {
		char *str = (char*)malloc(MAX_STR_SIZE * sizeof(char));
		fgets(str, MAX_STR_SIZE, stdin);
		if (str[0] == '\n') {
			break;
		}
		else
			push(&strList, (void*)str);
	}

	pthread_cancel(sorterThread);
	pthread_join(sorterThread, NULL);

	printf("\nSorted:\n");
	while (!isEmpty(&strList)) {
		char* str = (char*)pop(&strList);
		printf("%s", str);
		free(str);
	}

	return 0;
}