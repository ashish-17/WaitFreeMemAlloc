#ifndef WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_
#define WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_

#include "commons.h"

typedef struct CircularQueue CircularQueue;

CircularQueue* circularQueueCreate(int elementSize, int noOfElements);

void circularQueueFree(CircularQueue *queue);

int circularQueueGetElementSize(CircularQueue *queue);

int circularQueueGetMaxNumberOfElements(CircularQueue *queue);

bool circularQueueEnq(CircularQueue *queue, void* element);

void* circularQueueDeq(CircularQueue *queue);

#endif /* WAITFREEMEMALLOC_SRC_CIRCULARQUEUE_H_ */
