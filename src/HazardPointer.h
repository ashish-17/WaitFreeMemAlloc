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
	int *topPointers;
	int numberOfThreads;
	int numberOfHP;
}HPStructure;

extern HPStructure *globalHPStructure;

void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP);

void hpStructureDestroy(HPStructure *hpStructure);

void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr);

void* setHazardPointer(HPStructure *hpStructure, int threadId, void *element);

void clearHazardPointer(HPStructure *hpStructure, int threadId);

#endif /* WAITFREEMEMALLOC_SRC_HAZARDPOINTER_H_ */
