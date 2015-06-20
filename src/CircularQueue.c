#include "CircularQueue.h"
#include <pthread.h>


CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	printf("getCircularQueueElement: Trying to access %d th element of Circular Queue from address %u (%d)\n", index, queue->baseAddress, pthread_self());
	CircularQueueElement *ptr = (queue->baseAddress + index);
	printf("getCircularQueueElement: Success(%u) to access %d th element of Circular Queue from address %u (%d)\n", ptr, index, queue->baseAddress, pthread_self());
	//ptr->value = NULL;
	return ptr;
}

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements) {
	queue->baseAddress = (CircularQueueElement*) my_malloc(sizeof(CircularQueueElement) * noOfElements);
	queue->head = queue->tail = -1;
	queue->elementSize = elementSize;
	queue->maxNumberOfElements = noOfElements;

	for (int i = 0; i < noOfElements; i++) {
		getCircularQueueElement(queue, i)->value = NULL;
	}
}

bool circularQueueEnq(CircularQueue *queue, const void* element) {
	if ((queue->tail + 1) % queue->maxNumberOfElements == queue->head) {
		printf("circularQueue full \n");
		return false;
	}
	else {
		queue->tail = (queue->tail + 1) % queue->maxNumberOfElements;
		//printf("maxnoOfElements = %d\n", queue->maxNumberOfElements);
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
	printf("CircularQueueDEque: thread (%d)\n", pthread_self());
	if (queue->head == -1) {
		return NULL;
	}
	else {
		void *element = getCircularQueueElement(queue, queue->head)->value;
		printf("CircularQueueDEque: returned the element thread (%d)\n", pthread_self());
		getCircularQueueElement(queue, queue->head)->value = NULL;
		printf("CircularQueueDEque: settinf the value to null thread (%d)\n", pthread_self());
		if (queue->head == queue->tail) {
			queue->head = queue->tail = -1;
		}
		else {
			queue->head = (queue->head + 1) % queue->maxNumberOfElements;
		}
		printf("CircularQueueDEque: successfully returning the element (%d)\n", pthread_self());
		return element;
	}
}
