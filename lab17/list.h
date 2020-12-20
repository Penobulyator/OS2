#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct node {
	void *data;
	struct node *next;
	pthread_mutex_t mutex;
}node;

typedef struct list {
	node* head;
}list;

void initNode(node *newNode, void* data) {
	newNode->data = data;

	pthread_mutexattr_t arrt;
	pthread_mutexattr_init(&arrt);
	pthread_mutexattr_settype(&arrt, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&newNode->mutex, &arrt);
}

void destroyNode(node *node) {
	pthread_mutex_destroy(&node->mutex);
	free(node);
}

void initList(list *list) {
	list->head = NULL;
}

int isEmpty(list *list) {
	return list->head == NULL;
}

void push(list *list, void *data) {
	node *newNode = (node*)malloc(sizeof(node));;
	initNode(newNode, data);
	newNode->next = list->head;
	
	pthread_mutex_lock(&newNode->mutex);

	if (isEmpty(list)) {
		list->head = newNode;
	}
	else {
		pthread_mutex_lock(&list->head->mutex);

		list->head = newNode;

		pthread_mutex_unlock(&list->head->next->mutex);
	}

	pthread_mutex_unlock(&newNode->mutex);
}

void* pop(list* list) {
	if (isEmpty(list)) {
		return NULL;
	}

	pthread_mutex_lock(&list->head->mutex);

	node* oldHead = list->head;
	void *data = oldHead->data;
	list->head = list->head->next;

	pthread_mutex_unlock(&oldHead->mutex);

	destroyNode(oldHead);

	return data;
}

void swap(node *a, node *b) {
	void *t = a->data;
	a->data = b->data;
	b->data = t;
}

//
// int comparator(const void* p1, const void* p2); 
// Returnns:
// < 0 The element pointed by p1 goes before the element pointed by p2
// 0  The element pointed by p1 is equivalent to the element pointed by p2
// >0 The element pointed by p1 goes after the element pointed by p2
//
void sort(list* list, int(*comparator)(const void*, const void*)) {
	//do not sort empty list
	if (isEmpty(list)) {
		return;
	}

	pthread_mutex_lock(&list->head->mutex);
	node* start = list->head;

	//do not sort list of size 1
	if (start->next == NULL) {
		pthread_mutex_unlock(&start->mutex);
		return;
	}

	for (; start->next != NULL; start = start->next) {

		for (node* next = start->next; next != NULL; next = next->next) {
			pthread_mutex_lock(&next->mutex);
			if (comparator(start->data, next->data) > 0) {
				swap(start, next);
			}
			pthread_mutex_unlock(&next->mutex);
		}

		pthread_mutex_lock(&start->next->mutex);
		pthread_mutex_unlock(&start->mutex);
	}

	pthread_mutex_unlock(&start->mutex);
}	