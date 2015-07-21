//Single enqueue single dequeue

#include "CircularQueue.h"
#include <pthread.h>

typedef struct CircularQueueElement {
	void* value;
} CircularQueueElement;

struct CircularQueue {
	int head;
	int tail;

	CircularQueueElement *baseAddress;
	int elementSize;
	int maxNumberOfElements;
};

bool isCircularQueueFull(CircularQueue *queue);
bool isCircularQueueEmpty(CircularQueue *queue);
CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index);

CircularQueue* circularQueueCreate(int elementSize, int noOfElements) {
	LOG_PROLOG();

	CircularQueue *queue = (CircularQueue*) my_malloc(sizeof(CircularQueue));
	if (queue != NULL) {
        queue->head = queue->tail = -1;
        queue->elementSize = elementSize;
        queue->maxNumberOfElements = noOfElements;

        queue->baseAddress = (CircularQueueElement*) my_malloc(sizeof(CircularQueueElement) * noOfElements);
        if (queue->baseAddress == NULL) {
            LOG_ERROR("Unable to allocate memory for circular queue elements");
        } else {
            for (int i = 0; i < noOfElements; i++) {
                getCircularQueueElement(queue, i)->value = NULL;
            }
        }

	} else {
        LOG_ERROR("Unable to allocate memory for circular queue");
	}

	LOG_EPILOG();
	return queue;
}

void circularQueueFree(CircularQueue *queue) {
	LOG_PROLOG();

	if (queue != NULL) {
		CircularQueueElement *ptr = NULL;
		while(!isCircularQueueEmpty(queue)) {
			ptr = circularQueueDeq(queue);
			if (ptr != NULL) {
				if (ptr->value != NULL) {
					my_free(ptr->value);
					ptr->value = NULL;
				}

				my_free(ptr);
				ptr = NULL;
			} else {
				LOG_ERROR("Trying to free NULL pointer popped from circular queue");
			}
		}

		my_free(queue);
		queue = NULL;
	} else {
		LOG_ERROR("Trying to free NULL pointer");
	}

	LOG_EPILOG();
}

int circularQueueGetElementSize(CircularQueue *queue){
    LOG_PROLOG();

	int size = 0;
	if (queue != NULL) {
        size = queue->elementSize;
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return size;
}

int circularQueueGetMaxNumberOfElements(CircularQueue *queue){
    LOG_PROLOG();

	int numElementes = 0;
	if (queue != NULL) {
        numElementes = queue->maxNumberOfElements;
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return numElementes;
}

bool circularQueueEnq(CircularQueue *queue, void* element) {
	LOG_PROLOG();

	bool flag = false;
	if (queue != NULL) {
        if (isCircularQueueFull(queue)) {
            LOG_INFO("circularQueue full \n");
        } else {
            queue->tail = (queue->tail + 1) % queue->maxNumberOfElements;

            getCircularQueueElement(queue, queue->tail)->value = element;
            if(queue->head == -1) {
                queue->head = queue->tail;
            }

            flag = true;
        }
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return flag;
}

void* circularQueueDeq(CircularQueue *queue) {
	LOG_PROLOG();

	void *ptr = NULL;
	if (queue != NULL) {
        if (isCircularQueueEmpty(queue)) {
            LOG_INFO("Empty Circular queue, nothing to dequeue");
        } else {
            ptr = getCircularQueueElement(queue, queue->head)->value;

            getCircularQueueElement(queue, queue->head)->value = NULL;

            if (queue->head == queue->tail) {
                queue->head = queue->tail = -1;
            } else {
                queue->head = (queue->head + 1) % queue->maxNumberOfElements;
            }
        }
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return ptr;
}
bool isCircularQueueFull(CircularQueue *queue) {
	LOG_PROLOG();
	bool flag = false;
	if (queue != NULL) {
        if (((queue->tail + 1) % queue->maxNumberOfElements) == queue->head) {
            flag = true;
        }
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return flag;
}

bool isCircularQueueEmpty(CircularQueue* queue) {
	LOG_PROLOG();
	bool flag = false;

	if (queue != NULL) {
        if (queue->head == -1) {
            flag = true;
        }
	} else {
        LOG_ERROR("Invalid queue!");
	}

	LOG_EPILOG();
	return flag;
}

CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index) {
	LOG_PROLOG();

    CircularQueueElement *ptr = NULL;
    if (queue != NULL) {
        if ((index >= 0) && (index < queue->maxNumberOfElements)) {
            ptr = (queue->baseAddress + index);
        } else {
            LOG_ERROR("Index out of bound for circular queue");
        }
    } else {
        LOG_ERROR("Invalid queue!");
    }

	LOG_EPILOG();
	return ptr;
}

