#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define LINES_COUNT 10

pthread_cond_t condVar;
pthread_mutex_t mutex;

int parentCanPrint = 1;

void print_lines_parrent() {
	for (int i = 0; i < LINES_COUNT; i++) {
		pthread_mutex_lock(&mutex);

		//wait until we can print
		while (!parentCanPrint) {
			pthread_cond_wait(&condVar, &mutex);
		}

		//print
		printf("Parent string\n");

		//let child print
		parentCanPrint = 0;
		pthread_cond_signal(&condVar);

		pthread_mutex_unlock(&mutex);
	}
}

void* print_lines_child(void* arg) {
	for (int i = 0; i < LINES_COUNT; i++) {
		pthread_mutex_lock(&mutex);

		//wait until we can print
		while (parentCanPrint) {
			pthread_cond_wait(&condVar, &mutex);
		}

		//print
		printf("Child string\n");

		//let parent print
		parentCanPrint = 1;
		pthread_cond_signal(&condVar);

		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

int main() {
	pthread_t child;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&condVar, NULL);

	int result = pthread_create(&child, NULL, print_lines_child, NULL);
	if (result != 0) {
		perror("Error while creating a thread");
		return EXIT_FAILURE;
	}

	print_lines_parrent();

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&condVar);

	pthread_exit(NULL);
}
