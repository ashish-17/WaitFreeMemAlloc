#ifndef WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_
#define WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_

#include "commons.h"

typedef struct _CircularQueueElement {
	void* value;
} CircularQueueElement;

typedef struct {
	int head;
	int tail;

	CircularQueueElement *baseAddress;
	int elementSize;
	int maxNumberOfElements;
} CircularQueue;

CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index);

CircularQueue* circularQueueCreate(int elementSize, int noOfElements);

void circularQueueFree(CircularQueue *queue);

bool circularQueueEnq(CircularQueue *queue, const void* element);

void* circularQueueDeq(CircularQueue *queue);

#endif /* WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_ */
