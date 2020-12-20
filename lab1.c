#include <stdio.h>
#include <pthread.h>

void *thread_func(void *arg) {
	for (int i = 0; i < 10; i++)
		printf("Child prints %d\n", i);
}

int main() {
	pthread_t thread;

	int pthread_create_r = pthread_create(&thread, NULL, thread_func, NULL);
	if (pthread_create_r != 0) {
		printf("pthread_create failed with code %d\n", pthread_create_r);
		return 1;
	}
	for (int i = 0; i < 10; i++)
		printf("Parent prints %d\n", i);
	return 0;
}