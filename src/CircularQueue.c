#include "CircularQueue.h"
#include <pthread.h>


CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	LOG_PROLOG();
	//LOG_INFO("getCircularQueueElement: Trying to access %d th element of Circular Queue from address %u (%d)\n", index, queue->baseAddress, pthread_self());
	CircularQueueElement *ptr = (queue->baseAddress + index);
	//LOG_INFO("getCircularQueueElement: Success(%u) to access %d th element of Circular Queue from address %u (%d)\n", ptr, index, queue->baseAddress, pthread_self());
	LOG_EPILOG();
	return ptr;
}

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements) {
	LOG_PROLOG();
	queue->baseAddress = (CircularQueueElement*) my_malloc(sizeof(CircularQueueElement) * noOfElements);
	queue->head = queue->tail = -1;
	queue->elementSize = elementSize;
	queue->maxNumberOfElements = noOfElements;

	for (int i = 0; i < noOfElements; i++) {
		getCircularQueueElement(queue, i)->value = NULL;
	}
	LOG_EPILOG();
}

bool circularQueueEnq(CircularQueue *queue, const void* element) {
	LOG_PROLOG();
	bool flag;
	if ((queue->tail + 1) % queue->maxNumberOfElements == queue->head) {
		LOG_INFO("circularQueue full \n");
		flag = false;
	}
	else {
		queue->tail = (queue->tail + 1) % queue->maxNumberOfElements;
		//LOG_INFO("maxnoOfElements = %d\n", queue->maxNumberOfElements);
		//LOG_INFO("tailValue = %d\n", queue->tail);
		//LOG_INFO("address of QueueElement %d is %u", queue->tail, getCircularQueueElement(queue, queue->tail));
		getCircularQueueElement(queue, queue->tail)->value = element;
		if(queue->head == -1) {
			queue->head = queue->tail;
		}
		flag = true;
	}
	LOG_EPILOG();
	return flag;
}

void* circularQueueDeq(CircularQueue *queue) {
	//LOG_INFO("CircularQueueDEque: thread (%d)\n", pthread_self());
	LOG_PROLOG();
	void *ptr;
	if (queue->head == -1) {
		ptr = NULL;
	}
	else {
		void *element = getCircularQueueElement(queue, queue->head)->value;
		//LOG_INFO("circularQueueDeq: returned the element");
		getCircularQueueElement(queue, queue->head)->value = NULL;
		//LOG_INFO("circularQueueDeq: setting the value to null thread");
		if (queue->head == queue->tail) {
			queue->head = queue->tail = -1;
		}
		else {
			queue->head = (queue->head + 1) % queue->maxNumberOfElements;
		}
		ptr = element;
	}
	LOG_EPILOG();
	return ptr;
}
