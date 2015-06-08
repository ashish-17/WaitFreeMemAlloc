#ifndef WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_
#define WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_

#include "commons.h"
#include "CircularQueue.h"

typedef struct {
	CircularQueue* queue;
} FreeQueue;

typedef struct {
	FreeQueue* freeQueues;
	int **hazardPointers;
	int *roundCounters;
	int numberOfThreads;
	int numberOfHP;
}HPStructure;

HPStructure *globalHPStructure = NULL;


void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP);

void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr);

#endif /* WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_ */
