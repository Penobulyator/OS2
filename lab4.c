#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_func(void *arg) {
	while (1)
	{
		printf("HELLO ");
	}
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