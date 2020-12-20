#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
typedef struct node {
	void *data;
	struct node *next;
}node;

typedef struct list {
	node* head;
	pthread_mutex_t mutex;
}list;

void initList(list *list) {
	list->head = NULL; 
	pthread_mutexattr_t arrt;
	pthread_mutexattr_init(&arrt);
	pthread_mutexattr_settype(&arrt, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&list->mutex, &arrt);
}

int isEmpty(list *list) {
	pthread_mutex_lock(&list->mutex);
	int ret = list->head == NULL;
	pthread_mutex_unlock(&list->mutex);
	return ret;
}

void push(list *list, void *data) {
	pthread_mutex_lock(&list->mutex);
	node *newNode = (node*)malloc(sizeof(node));
	newNode->data = data;
	newNode->next = list->head;

	list->head = newNode;
	pthread_mutex_unlock(&list->mutex);
}

void* pop(list* list) {
	pthread_mutex_lock(&list->mutex);
	if (isEmpty(list))
		return NULL;

	void *data = list->head->data;
	node* oldHead = list->head;
	list->head = list->head->next;
	free(oldHead);
	pthread_mutex_unlock(&list->mutex);
	return data;
}

void swap(node *a, node *b) {
	void *t = a->data;
	a->data = b->data;
	b->data = t;
}

//
//int comparator(const void* p1, const void* p2); 
//Returnns:
//< 0 The element pointed by p1 goes before the element pointed by p2
//0  The element pointed by p1 is equivalent to the element pointed by p2
//>0 The element pointed by p1 goes after the element pointed by p2
//
void sort(list* list, int(*comparator)(const void*, const void*)) {
	pthread_mutex_lock(&list->mutex);

	for (node *start = list->head; start != NULL; start = start->next) {

		for (node* next = start->next; next != NULL; next = next->next) {
			if (comparator(start->data, next->data) > 0) {
				swap(start, next);
			}
		}
	}

	pthread_mutex_unlock(&list->mutex);
}