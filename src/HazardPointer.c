#include "HazardPointer.h"

FreeQueue* getFreeQueue(FreeQueue *queue, int index) {
	return (queue + index);
}

/*int* getHP(int *hazardPointers, int numberOfHP, int priIndex, int secIndex) {
	return (hazardPointers + (priIndex*numberOfHP) + secIndex);
}*/

uintptr_t combine(void *ptr, bool mark) {
	if (mark) {
		return (((uintptr_t) ptr) | ((uintptr_t) 0x1));
	}
	else {
		return (uintptr_t) ptr;
	}
}

bool getMark(uintptr_t x) {
	return x & (uintptr_t)0x01;
}

void* setMark(void *node, bool mark) {
	return combine(node, mark);
}

bool isMarked(void *value) {
	uintptr_t current = value;
	return getMark(current);
}

void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP) {
	hpStructure->freeQueues = (FreeQueue*) malloc(sizeof(FreeQueue) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		CircularQueue *queue = getFreeQueue(hpStructure->freeQueues, i)->queue;
		queue = (CircularQueue*) malloc(sizeof(CircularQueue));
		circularQueueCreate(queue, sizeof(int *), noOfThreads * noOfHP);
	}
	// pushing sentinel nodes in all the circular queues
	for (int i = 0; i < noOfThreads; i++) {
		CircularQueue *queue = getFreeQueue(hpStructure->freeQueues, i)->queue;
		for (int j = 1; j < noOfHP * noOfThreads; j++) {
			circularQueueEnq(queue, NULL);
		}
	}

	hpStructure->hazardPointers = (int**) malloc(sizeof(int *) * noOfThreads * noOfHP);
	for (int i = 0; i < noOfThreads * noOfHP; i++) {
		hpStructure->hazardPointers[i] = NULL;
	}

	hpStructure->roundCounters = (int*) malloc(sizeof(int) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		hpStructure->roundCounters[i] = 0;
	}

	hpStructure->numberOfHP = noOfHP;
	hpStructure->numberOfThreads = noOfThreads;
}

void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr) {
	circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId), ptr);
	void* node = NULL;
	while (node == NULL) {
		void *inspect = hpStructure->hazardPointers[hpStructure->roundCounters[threadId]];
		hpStructure->roundCounters[threadId]++;
		if (inspect != NULL) {
			inspect = setMark(inspect, 1);
		}
		node = circularQueueDeq(getFreeQueue(hpStructure->freeQueues, threadId));
		if (node == NULL) {
			return;
		}
		else if (getMark(node) == 0) {
			free(node);
			return;
		}
		else {
			setMark(node, 0);
			circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId), node);
		}
	}
}
