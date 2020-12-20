#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#define PARENT_SEM_NAME "/parentSem"
#define CHILD_SEM_NAME  "/childSem"

#define SEM_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP

#define LINES_COUNT 10

sem_t *parentSem;
sem_t *childSem;

void print_lines_parent() {
	for (int i = 0; i < LINES_COUNT; i++) {
		sem_wait(parentSem);

		printf("Parent string\n");

		sem_post(childSem);

	}	
}

void print_lines_child() {
	for (int i = 0; i < LINES_COUNT; i++) {
		sem_wait(childSem);

		printf("Child string\n");

		sem_post(parentSem);
	}

	return;
}

int main() {
	//open parent semaphore
	parentSem = sem_open(PARENT_SEM_NAME, O_CREAT , SEM_MODE, 1);
	if (parentSem == SEM_FAILED)
	{
		perror("parentSem open");
		exit(1);
	}

	//open child semaphore
	childSem = sem_open(CHILD_SEM_NAME, O_CREAT, SEM_MODE, 0);
	if (childSem == SEM_FAILED)
	{
		perror("childSem open");
		exit(1);
	}
	
	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork"); 
		exit(1);
	case 0:

		//we are child
		print_lines_child();
		break;
	default:
		break;
	}

	//we are parent
	print_lines_parent();
	//unlink semaprores if they are allready existing
	int ret = sem_unlink(PARENT_SEM_NAME);
	if (ret != 0)
	{
		perror("parentSem unlink");
		exit(1);
	}


	ret = sem_unlink(CHILD_SEM_NAME);
	if (ret != 0)
	{
		perror("parentSem unlink");
		exit(1);
	}

	sem_close(parentSem);
	sem_close(childSem);
}