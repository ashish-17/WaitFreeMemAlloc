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
		getFreeQueue(hpStructure->freeQueues, i)->queue = (CircularQueue*) malloc(sizeof(CircularQueue));
		circularQueueCreate(getFreeQueue(hpStructure->freeQueues, i)->queue, sizeof(int *), noOfThreads * noOfHP);
	}
	//printf("created Circular queues\n");
	// pushing sentinel nodes in all the circular queues
	for (int i = 0; i < noOfThreads; i++) {
		CircularQueue *queue = getFreeQueue(hpStructure->freeQueues, i)->queue;
		//printf("CQueuePtr = %u\n", queue);
		for (int j = 1; j < noOfHP * noOfThreads; j++) {
			circularQueueEnq(queue, NULL);
		}
	}
	//printf("initialized Circular queues with NULL values\n");
	hpStructure->hazardPointers = (int**) malloc(sizeof(int *) * noOfThreads * noOfHP);
	for (int i = 0; i < noOfThreads * noOfHP; i++) {
		hpStructure->hazardPointers[i] = NULL;
	}
	//printf("created hazardPointersArray\n");
	hpStructure->roundCounters = (int*) malloc(sizeof(int) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		hpStructure->roundCounters[i] = 0;
	}

	hpStructure->topPointers = (int*) malloc(sizeof(int) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		hpStructure->topPointers[i] = 0;
	}
	//printf("created roundCounters\n");
	hpStructure->numberOfHP = noOfHP;
	hpStructure->numberOfThreads = noOfThreads;
	//printf("hpCreate: hpStructure = %u\n", hpStructure);
}

void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr) {
	bool flag = circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId)->queue, ptr);
	//printf("threadId = %d enqueue was successful = %u \n", threadId, flag);
	void* node = NULL;
	while (node == NULL) {
		//printf("came here\n");
		//printf("threadId = %d roundCounter = %u\n", threadId, hpStructure->roundCounters[threadId]);
		//printf("threadId = %d, size of queue = %d\n", threadId,hpStructure->numberOfHP * hpStructure->numberOfThreads);
		void *inspect = hpStructure->hazardPointers[hpStructure->roundCounters[threadId]];
		hpStructure->roundCounters[threadId] = (hpStructure->roundCounters[threadId] + 1) % (hpStructure->numberOfHP * hpStructure->numberOfThreads);
		//printf("threadId = %d roundCounter = %u\n", threadId, hpStructure->roundCounters[threadId]);
		//printf("threadId = %d inspectPtr = %u\n", threadId, inspect);
		if (inspect != NULL) {
			inspect = setMark(inspect, 1);
		}
		node = circularQueueDeq(getFreeQueue(hpStructure->freeQueues, threadId)->queue);
		printf("threadId = %d nodePtr = %u dequeued\n", threadId, node);
		if (node == NULL) {
			//printf("threadId = %d node dequeued was null\n", threadId);
			return;
		}
		else if (getMark(node) == 0) {
			printf("&&&&&&&threadId = %d freeing nodeptr = %u\n", threadId, node);
			free(node);
			return;
		}
		else {
			printf("***** threadID = %d marking the non null node\n", threadId);
			setMark(node, 0);
			circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId)->queue, node);
		}
	}
}

void* setHazardPointer(HPStructure *hpStructure, int threadId, void *element) {
	//printf("in setHP\n");
	//printf("hpStructure = %u\n", hpStructure);
	//printf("thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = element;
	//printf("setHP\n");
	hpStructure->topPointers[threadId]++;
	return element;
}

void* getHazardPointer(HPStructure *hpStructure, int threadId) {
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]];
}

void clearHazardPointer(HPStructure *hpStructure, int threadId) {
	hpStructure->topPointers[threadId]--;
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = NULL;
}
