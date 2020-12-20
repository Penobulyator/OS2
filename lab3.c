#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_func(void *arg) {
	char **str = (char**)arg;
	for (int i = 0; i < pthread_self() - 1; i++)
		printf("Thread number %d prints \"%s\" \n", pthread_self(), str[i]);
	printf("\n");
	return NULL;
}

int main() {
	pthread_t thread[4];
	char **args;
	args[0] = "String 1";
	args[1] = "String 2";
	args[2] = "String 3";
	args[3] = "String 4";
	//create threads
	for (int i = 0; i < 4; i++) {
		int pthread_create_r = pthread_create(&thread[i], NULL, thread_func, (void*)args);
		if (pthread_create_r != 0) {
			printf("pthread_create failed with code %d\n", pthread_create_r);
			return 1;
		}

		int pthread_join_r = pthread_join(thread[i], NULL);
		if (pthread_join_r != 0) {
			printf("pthread_join failed with code %d\n", pthread_join_r);
			return 1;
		}
	}
}