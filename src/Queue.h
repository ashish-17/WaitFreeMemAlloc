#ifndef WAITFREEMEMALLOC_SRC_QUEUE_H_
#define WAITFREEMEMALLOC_SRC_QUEUE_H_

#include <stdatomic.h>
#include "commons.h"

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

//bool queueEnq1(Queue *queue, const void* element);

void queueCreate(Queue *queue, int elementSize);

bool isQueueEmpty(Queue *queue);

void queueEnqUC(Queue *queue, void* element);

bool queueEnqC(Queue *queue, void* element, int threadId);

void* queueDeqUC(Queue *queue);

void* queueDeqC(Queue *queue, QueueElement *oldQueueHead, int threadId);

void queueFree(Queue *queue);

#endif /* WAITFREEMEMALLOC_SRC_QUEUE_H_ */
