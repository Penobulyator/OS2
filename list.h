#include <stdlib.h>

typedef struct node {
	void *data;
	struct node *next;
}node;

typedef struct list {
	node* head;
}list;

void initList(list *list) {
	list->head = NULL;
}

void destroyList(list* list) {
	node* curNode = list->head;
	while (curNode != NULL) {
		node* tmpNext = curNode->next;
		free(curNode);
		curNode = tmpNext;
	}
}

size_t size(list* list) {
	size_t size = 0;

	for (node* head = list->head; head = head->next; head->next != NULL)
		size++;

	return size;
}

int isEmpty(list *list) {
	return list->head == NULL;
}

void push(list *list, void *data) {
	node *newNode = (node*)malloc(sizeof(node));
	newNode->data = data;
	newNode->next = list->head;

	list->head = newNode;
}

void* pop(list* list) {
	if (isEmpty(list))
		return NULL;

	void *data = list->head->data;
	node* oldHead = list->head;
	list->head = list->head->next;
	free(oldHead);
	return data;
}