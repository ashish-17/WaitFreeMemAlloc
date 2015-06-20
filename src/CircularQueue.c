#include "CircularQueue.h"
#include <pthread.h>


CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	log_msg_prolog("getCircularQueueElement");
	//printf("getCircularQueueElement: Trying to access %d th element of Circular Queue from address %u (%d)\n", index, queue->baseAddress, pthread_self());
	CircularQueueElement *ptr = (queue->baseAddress + index);
	//printf("getCircularQueueElement: Success(%u) to access %d th element of Circular Queue from address %u (%d)\n", ptr, index, queue->baseAddress, pthread_self());
	log_msg_epilog("getCircularQueueElement");
	return ptr;
}

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements) {
	log_msg_prolog("circularQueueCreate");
	queue->baseAddress = (CircularQueueElement*) my_malloc(sizeof(CircularQueueElement) * noOfElements);
	queue->head = queue->tail = -1;
	queue->elementSize = elementSize;
	queue->maxNumberOfElements = noOfElements;

	for (int i = 0; i < noOfElements; i++) {
		getCircularQueueElement(queue, i)->value = NULL;
	}
	log_msg_epilog("circularQueueCreate");
}

bool circularQueueEnq(CircularQueue *queue, const void* element) {
	log_msg_prolog("circularQueueEnq");
	bool flag;
	if ((queue->tail + 1) % queue->maxNumberOfElements == queue->head) {
		printf("circularQueue full \n");
		flag = false;
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
		flag = true;
	}
	log_msg_epilog("circularQueueEnq");
	return flag;
}

void* circularQueueDeq(CircularQueue *queue) {
	//printf("CircularQueueDEque: thread (%d)\n", pthread_self());
	log_msg_prolog("circularQueueDeq");
	void *ptr;
	if (queue->head == -1) {
		ptr = NULL;
	}
	else {
		void *element = getCircularQueueElement(queue, queue->head)->value;
		log_msg("circularQueueDeq: returned the element");
		getCircularQueueElement(queue, queue->head)->value = NULL;
		log_msg("circularQueueDeq: setting the value to null thread");
		if (queue->head == queue->tail) {
			queue->head = queue->tail = -1;
		}
		else {
			queue->head = (queue->head + 1) % queue->maxNumberOfElements;
		}
		ptr = element;
	}
	log_msg_epilog("circularQueueDeq");
	return ptr;
}
