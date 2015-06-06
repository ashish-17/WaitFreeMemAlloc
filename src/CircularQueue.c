#include "CircularQueue.h"

typedef struct _CircularQueueElement {
	void* value;
} CircularQueueElement;

typedef struct {
	CircularQueueElement *head;
	CircularQueueElement *tail;

	CircularQueueElement *baseAddress;
	int elementSize;
	int maxNumberOfElements;
} CircularQueue;

CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	return (queue->baseAddress + index);
}

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements) {
	queue->head = malloc(sizeof(CircularQueueElement) * noOfElements);
	queue->tail = queue->head;
	queue->elementSize = elementSize;
	queue->maxNumberOfElements = noOfElements;

	for (int i = 0; i < noOfElements; i++) {
		getCircularQueueElement(queue, i)->value = NULL;
	}
}

bool circularQueueEnq(CircularQueue *queue, const void* element) {
	if ((queue->tail + 1) % queue->maxNumberOfElements == queue->head) {
		return false;
	}
	else {
		queue->tail->value = element;
		queue->tail = (queue->tail + 1) % queue->maxNumberOfElements;
		return true;
	}
}

void* circularQueueDeq(CircularQueue *queue) {
	if (queue->head == queue->tail) {
		return NULL;
	}
	else {
		void *element = queue->head->value;
		queue->head->value = NULL;
		queue->head = (queue->head + 1) % queue->maxNumberOfElements;
		return element;
	}
}
