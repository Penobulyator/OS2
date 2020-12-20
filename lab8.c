#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define iterCount (1 << 10)

pthread_barrier_t barrier;
pthread_mutex_t mutex;

int wasSig = 0;

int quit = 0;
int threadWasHereCount = 0;

int threadN;

typedef struct calcArg {
	int start;
	int step;
	double out;
}calcArg;

void checkQuit() {
	if (quit)
		pthread_exit(NULL);

	pthread_mutex_lock(&mutex);
	threadWasHereCount++;
	if (threadWasHereCount == threadN) {
		if (wasSig)
			quit = 1;
		threadWasHereCount = 0;
	}
	pthread_mutex_unlock(&mutex);
}

void *calc(void *arg) {
	calcArg *Arg = (calcArg*)arg;
	Arg->out = 0.0;
	int curPos = Arg->start;

	while (1) {
		for (int i = 0; i < iterCount; i++) {
			Arg->out += 1.0 / (curPos*4.0 + 1.0);
			Arg->out -= 1.0 / (curPos*4.0 + 3.0);
			curPos += Arg->step;
		}

		pthread_barrier_wait(&barrier);

		checkQuit();	
	}
}

void interruptHandler() {
	wasSig = 1;
}

int main(int argc, char**argv) {
	threadN = atoi(argv[1]);

	pthread_t *threads = (pthread_t*)malloc(threadN * sizeof(pthread_t));
	calcArg *args = (calcArg*)malloc(threadN * sizeof(calcArg));
	pthread_barrier_init(&barrier, NULL, threadN);
	pthread_mutex_init(&mutex, NULL);

	//create threads
	for (int i = 0; i < threadN; i++) {
		args[i].start = i;
		args[i].step = threadN;
		args[i].out = 0;
		pthread_create(&threads[i], NULL, calc, &args[i]);
	}

	//set handler for SIGINT
	signal(SIGINT, interruptHandler);

	//count pi
	double pi = 0;
	for (int i = 0; i < threadN; i++) {
		pthread_join(threads[i], NULL);
		pi += args[i].out;
	}
	pi *= 4;

	//free resources
	free(args);
	free(threads);
	pthread_barrier_destroy(&barrier);
	pthread_mutex_destroy(&mutex);

	//print pi, return
	printf("\npi = %.10f\n", pi);
	return 0;
}