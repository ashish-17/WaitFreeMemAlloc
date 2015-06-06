#ifndef WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_
#define WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_

#include "commons.h"

CircularQueueElement * getCircularQueueElement(CircularQueue *queue, int index);

void circularQueueCreate(CircularQueue *queue, int elementSize, int noOfElements);

bool circularQueueEnq(CircularQueue *queue, const void* element);

void* circularQueueDeq(CircularQueue *queue);

#endif /* WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_ */
