#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *cancel_func(void *arg) {
	printf("I'm leaving\n");
	return NULL;
}

void *thread_func(void *arg) {
	pthread_cleanup_push(cancel_func, NULL);
	while (1)
	{
		pthread_testcancel();
		printf("HELLO ");
	}
	pthread_cleanup_pop(1);
}

int main() {
	pthread_t thread;
	int pthread_create_r = pthread_create(&thread, NULL, thread_func, NULL);
	sleep(2);
	pthread_cancel(thread);
	printf("Thread canceled\n");
	pthread_join(thread, NULL);
	printf("Thread joined\n");
	return 0;
}

