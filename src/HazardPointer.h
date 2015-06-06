#ifndef WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_
#define WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_

#include "commons.h"

typedef struct {
	CircularQueue* queue;
} FreeQueue;

typedef struct {
	FreeQueue* freeQueues;
	int *hazardPointers;
	int numberOfThreads;
	int numberOfHP;
}HPStructure;

FreeQueue* getFreeQueue(FreeQueue *queue, int index) {
	return (queue + index);
}

int* getHP(int *hazardPointers, int priIndex, int secIndex) {
	return ()
}

void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP) {
	hpStructure->freeQueues = (FreeQueue*) malloc(sizeof(FreeQueue) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		CircularQueue *queue = getFreeQueue(hpStructure->freeQueues, i)->queue;
		queue = (CircularQueue*) malloc(sizeof(CircularQueue));
		circularQueueCreate(queue, sizeof(int *), noOfThreads * noOfHP);
	}
	hpStructure->hazardPointers = (int*) malloc(sizeof(int) * noOfThreads * noOfHP);
	for (int i = 0; i < noOfThreads; i++) {
		for (int j = 0; j < noOfHP; j++) {
			getHP(hpStructure->hazardPointers, i, j) = NULL;
		}
	}
}

#endif /* WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_ */
