#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>	
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include "../list.h"
#include <errno.h>
#define MAX_FILENAME_SIZE 256
#define READ_LENGTH 100

#define RET_CHECK(func) do { int ret = func; if (ret) perror(#func);  }while (0)

sem_t fdSem;

typedef struct cpArg{
	char *sourceDir;
	char *destDir;
}cpArg;


void makePath(char *dirName, char* fileName, char *buf) {
	sprintf(buf, "%s/%s", dirName, fileName);
}

int isDir(struct stat file) {
	return (file.st_mode & S_IFMT) == S_IFDIR;
}

int isReg(struct stat file) {
	return (file.st_mode & S_IFMT) == S_IFREG;
}

void post() {
	sem_post(&fdSem);
	//printf("Posting\n");
}

void wait() {
	sem_wait(&fdSem);
	//printf("Waiting\n");
}


void* copyFile(void* input) {
	cpArg arg = *(cpArg*)input;
	char *sourceDir = arg.sourceDir;
	char *destDir = arg.destDir;


	//open file from source directory
	wait();
	int srcFd = open(sourceDir, O_RDONLY);
	if (srcFd == -1) {
		printf("Failed to open %s\n", sourceDir);
		return NULL;
	}

	//create file in dest directory
    wait();
	int destFd = open(destDir, O_WRONLY | O_CREAT, 0644);
	if (destFd == -1) {
		printf("Failed to open %s\n", destDir);
		return NULL;
	}

	char buf[READ_LENGTH];
	while (1) {

		int len = read(srcFd, buf, READ_LENGTH);
		if (len == 0)
			break;
		else
			write(destFd, buf, len);
	}


	//close files
	close(srcFd);
	post();

	close(destFd);
	post();

	return NULL;
}

void *cp(void *input) {
	cpArg arg = *(cpArg*)input;

	list threadList;
	initList(&threadList);

	list argsList;
	initList(&argsList);

	DIR *dir = opendir(arg.sourceDir);
	if (dir == NULL) {
		perror("opendir");
		return NULL;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		//create full path to entry
		char *fullEntryPath = (char*)malloc(MAX_FILENAME_SIZE * sizeof(char));
		makePath(arg.sourceDir, entry->d_name, fullEntryPath);

		//open entry stat to check if it's a dirrectory or not
		struct stat entryStat;
		stat(fullEntryPath, &entryStat);

		//create full path to copy of entry in dest directory
		char *newDestPath = (char*)malloc(MAX_FILENAME_SIZE * sizeof(char));
		makePath(arg.destDir, entry->d_name, newDestPath);


		//create argument for new thread to copy entry
		cpArg *newArg = (cpArg*)malloc(sizeof(cpArg));
		newArg->sourceDir = fullEntryPath;
		newArg->destDir = newDestPath;

		//create thread
		pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));

		//push thread to list to join later
		push(&threadList, (void*)thread);

		//push newArg to l
		push(&argsList, newArg);
		if (isDir(entryStat)) { //entry is directory

			//create new subdirrectory in dest directory
			mkdir(newDestPath, 0777);

			RET_CHECK(pthread_create(thread, NULL, cp, newArg));

		}
		else if (isReg(entryStat)){ //enty is a regular file
			RET_CHECK(pthread_create(thread, NULL, copyFile, newArg));
		}
	}
	closedir(dir);

	//join all subthreads
	while (!isEmpty(&threadList)) {
		pthread_t *thread = (pthread_t*)pop(&threadList);
		RET_CHECK(pthread_join(*thread, NULL));
		free(thread);
	}

	//free all args for subthread
	while (!isEmpty(&argsList)) {
		cpArg *arg = (cpArg*)pop(&argsList);
		free(arg->sourceDir);
		free(arg->destDir);
		free(arg);
	}

}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: lab6.out <sourceDir> <destDir>");
		return 1;
	}
	
	sem_init(&fdSem, 0, 4864);

	cpArg arg;
	arg.sourceDir = argv[1];
	arg.destDir = argv[2];

	pthread_t thread;
	RET_CHECK(pthread_create(&thread, NULL, cp, &arg));
	RET_CHECK(pthread_join(thread, NULL));
	return 0;
}