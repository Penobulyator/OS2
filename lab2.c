#include <stdio.h>
#include <pthread.h>

int ret = 123;

void *thread_func(void *arg) {
	for (int i = 0; i < 10; i++)
		printf("Child prints %d\n", i);
	return &ret;
}

int main() {
	pthread_t thread;

	int pthread_create_r = pthread_create(&thread, NULL, thread_func, NULL);
	if (pthread_create_r != 0) {
		printf("pthread_create failed with code %d\n", pthread_create_r);
		return 1;
	}
	void* status;
	int pthread_join_r = pthread_join(thread, &status);
	if (pthread_join_r != 0) {
		printf("pthread_join failed with code %d\n", pthread_join_r);
		return 1;
	}

	int ret = *(int*)status;
	printf("Child returned pointer on %d\n", ret);
	for (int i = 0; i < 10; i++)
		printf("Parrent prints %d\n", i);
	return 0;
}