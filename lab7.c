#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define N (1 << 20)

typedef struct calcArg {
	int start;
	int step;
	double out;
}calcArg;

void *calc(void *arg) {
	calcArg *Arg = (calcArg*)arg;
	Arg->out = 0.0;
	for (int i = Arg->start; i < N; i+= Arg->step) {
		Arg->out += 1.0 / (i*4.0 + 1.0);
		Arg->out -= 1.0 / (i*4.0 + 3.0);
	}
	return NULL;
}

int main(int argc, char**argv) {
	int threadN = atoi(argv[1]);

	pthread_t *threads = (pthread_t*)malloc(threadN * sizeof(pthread_t));
	calcArg *args = (calcArg*)malloc(threadN * sizeof(calcArg));

	//create threads
	for (int i = 0; i < threadN; i++) {
		args[i].start = i;
		args[i].step = threadN;
		args[i].out = 0;
		pthread_create(&threads[i], NULL, calc, &args[i]);
	}

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

	//print, return
	printf("%lf\n", pi);
	return 0;
}