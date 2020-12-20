#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define LINES_COUNT 10
#define MUTEX_COUNT 3
#define PARENT_INDEX 0
#define CHILD_INDEX 1

pthread_mutex_t mutex[MUTEX_COUNT];

char* stringForParentThread = "Parent string\n";
char* stringForChildThread = "Child string\n";

int init_mutexes() {
	for (int i = 0; i < MUTEX_COUNT; i++) {
		int result = pthread_mutex_init(&mutex[i], NULL);
		if (result != 0) 
			return result;
	}
	return 0;
}

int destroy_mutexes() {
	for (int i = 0; i < MUTEX_COUNT; i++) {
		int result = pthread_mutex_destroy(&mutex[i]);
		if (result != 0) 
			return result;
	}
	return 0;
}

int lock(int index) {
	return pthread_mutex_lock(&mutex[index]);
}

int unlock(int index) {
	return pthread_mutex_unlock(&mutex[index]);
}

/*
---------------------------—
	parent		child
----------------------------—
	lock(1)		lock(2)
	start child
	sleep
----------------------------—
	lock(0)		lock(1)		
	print		print
	unlock(1)	unlock(2)

	lock(2)		lock(0)
	print		print
	unlock(0)	unlock(1)

	lock(1)		lock(2)
	print		print
	unlock(2)	unlock(0)
----------------------------—

*/

/* with 2 mutexes
	
---------------------------—
	parent		child
----------------------------—
	lock(0)		lock(1)
----------------------------—
	lock(1)		lock(0)
	print		print	
	unlock(0)	unlock(1)
-----------------------------
*/

int print_lines(char* line, int count, int thread_index) {
	int result;
	for (int i = 0; i < count; i++) {
		result = lock((2 * i + thread_index) % MUTEX_COUNT); //PARENT_INDEX = 0, CHILD_INDEX = 1
		if (result != 0) 
			return result;
		printf("%s", line);
		result = unlock((2 * i + 1 + thread_index) % 3);
		if (result != 0) 
			return result;
	}
	return 0;
}

void* thread_control(void* arg) {
	lock(2);
	int result = print_lines(stringForChildThread, LINES_COUNT, CHILD_INDEX);
	if (result != 0) {
		errno = result;
		perror("Error in child thread");
		exit(EXIT_FAILURE);
	}
	return NULL;
}
int main() {
	int result;
	pthread_t child;
	init_mutexes();
	lock(1);
	result = pthread_create(&child, NULL, thread_control, NULL);
	if (result != 0) {
		perror("Error while creating a thread");
		return EXIT_FAILURE;
	}
	sleep(1);
	result = print_lines(stringForParentThread, LINES_COUNT, PARENT_INDEX);
	if (result != 0) {
		perror("Error in parent thread");
		return EXIT_FAILURE;
	}
	destroy_mutexes();
	pthread_exit(NULL);
}
