#include <pthread.h>
typedef struct sem_t {
	int counter;
	pthread_mutex_t mutex;
	pthread_cond_t cv;
}sem_t;

void sem_init(sem_t *sem, int pshared, int maxCount){
	sem->counter = maxCount;
	pthread_mutex_init(&sem->mutex, NULL);
	pthread_cond_init(&sem->cv, NULL);
}

void sem_wait(sem_t *sem) {
	pthread_mutex_lock(&sem->mutex);

	while (sem->counter == 0)
		pthread_cond_wait(&sem->cv, &sem->mutex);

	sem->counter--;

	pthread_mutex_unlock(&sem->mutex);
}


void sem_post(sem_t *sem) {
	pthread_mutex_lock(&sem->mutex);

	sem->counter++;

	pthread_cond_signal(&sem->cv);

	pthread_mutex_unlock(&sem->mutex);
}

void sem_destroy(sem_t *sem) {
	pthread_mutex_destroy(&sem->mutex);
	pthread_cond_destroy(&sem->cv);
}