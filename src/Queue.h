#ifndef WAITFREEMEMALLOC_SRC_QUEUE_H_
#define WAITFREEMEMALLOC_SRC_QUEUE_H_

#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>

typedef struct _QueueElement {
	void* value;
	struct _QueueElement* next;
} QueueElement;

typedef struct {
	 QueueElement *head;
	 QueueElement *tail;
	// StackElement* elements;

	int elementSize;
//	int numberOfElements;
} Queue;


void queueCreate(Queue *queue, int elementSize);

bool queueEnq(Queue *queue, const void* element);

void* queueDeq(Queue *queue);

void queueFree(Queue *queue);

#endif /* WAITFREEMEMALLOC_SRC_QUEUE_H_ */
