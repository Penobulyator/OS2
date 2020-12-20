 #include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#define LINES_COUNT 10

sem_t parentSem;
sem_t childSem;

void print_lines_parrent() {
	for (int i = 0; i < LINES_COUNT; i++) {
		sem_wait(&parentSem);

		printf("Parent string\n");

		sem_post(&childSem);

	}
}

void* print_lines_child(void* arg) {
	for (int i = 0; i < LINES_COUNT; i++) {
		sem_wait(&childSem);

		printf("Child string\n");

		sem_post(&parentSem);
	}

	return NULL;
}

int main() {
	pthread_t child;

	sem_init(&parentSem, 0, 1);
	sem_init(&childSem, 0, 0);

	int result = pthread_create(&child, NULL, print_lines_child, NULL);
	if (result != 0) {
		perror("Error while creating a thread");
		return EXIT_FAILURE;
	}

	print_lines_parrent();

	sem_destroy(&parentSem);
	sem_destroy(&childSem);

	pthread_exit(NULL);
}
