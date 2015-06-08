#include "CircularQueue.h"


CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	return (queue->baseAddress + index);
}

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements) {
	queue->baseAddress = (CircularQueueElement*) malloc(sizeof(CircularQueueElement) * noOfElements);
	queue->head = queue->tail = -1;
	queue->elementSize = elementSize;
	queue->maxNumberOfElements = noOfElements;

	for (int i = 0; i < noOfElements; i++) {
		getCircularQueueElement(queue, i)->value = NULL;
	}
}

bool circularQueueEnq(CircularQueue *queue, const void* element) {
	if ((queue->tail + 1) % queue->maxNumberOfElements == queue->head) {
		//printf("circularQueue full \n");
		return false;
	}
	else {
		queue->tail = (queue->tail + 1) % queue->maxNumberOfElements;
		//printf("tailValue = %d\n", queue->tail);
		//printf("address of QueueElement %d is %u", queue->tail, getCircularQueueElement(queue, queue->tail));
		getCircularQueueElement(queue, queue->tail)->value = element;
		if(queue->head == -1) {
			queue->head = queue->tail;
		}
		return true;
	}
}

void* circularQueueDeq(CircularQueue *queue) {
	if (queue->head == -1) {
		return NULL;
	}
	else {
		void *element = getCircularQueueElement(queue, queue->head)->value;
		getCircularQueueElement(queue, queue->head)->value = NULL;
		if (queue->head == queue->tail) {
			queue->head = queue->tail = -1;
		}
		else {
			queue->head = (queue->head + 1) % queue->maxNumberOfElements;
		}
		return element;
	}
}
