#include "HazardPointer.h"
#include <assert.h>
#include<pthread.h>

//#define log_debug(str, threadId) printf("%s%d", str, threadId);


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


void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP) {
	hpStructure->freeQueues = (FreeQueue*) my_malloc(sizeof(FreeQueue) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		getFreeQueue(hpStructure->freeQueues, i)->queue = (CircularQueue*) my_malloc(sizeof(CircularQueue));
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
	hpStructure->hazardPointers = (int**) my_malloc(sizeof(int *) * noOfThreads * noOfHP);
	for (int i = 0; i < noOfThreads * noOfHP; i++) {
		hpStructure->hazardPointers[i] = NULL;
	}
	//printf("created hazardPointersArray\n");
	hpStructure->roundCounters = (int*) my_malloc(sizeof(int) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		hpStructure->roundCounters[i] = 0;
	}

	hpStructure->topPointers = (int*) my_malloc(sizeof(int) * noOfThreads);
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
	printf("freeMemHP: threadId = %d enqueue was successful = %u \n", threadId, flag);
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
			setDirty(inspect, 1);
		}
		node = circularQueueDeq(getFreeQueue(hpStructure->freeQueues, threadId)->queue);
		printf("freeMemHp: threadId = %d nodePtr = %u dequeued\n", threadId, node);
		if (node == NULL) {
			//printf("threadId = %d node dequeued was null\n", threadId);
			return;
		}
		else if (isDirty(node) == 0) {
			printf("&&&&&&&threadId = %d freeing nodeptr = %u\n", threadId, node);
			my_free(node);
			return;
		}
		else {
			printf("***** threadID = %d marking the non null node\n", threadId);
			setDirty(node, 0);
			circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId)->queue, node);
		}
	}
}

void* setHazardPointer(HPStructure *hpStructure, int threadId, void *element) {
	//printf("in setHP\n");
	//printf("hpStructure = %u\n", hpStructure);
	printf("setHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = element;
	//printf("setHP\n");
	hpStructure->topPointers[threadId]++;
	return element;
}

void* getHazardPointer(HPStructure *hpStructure, int threadId) {
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]];
}

void clearHazardPointer(HPStructure *hpStructure, int threadId) {
	printf("clearHP\n");
	printf("clearHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	assert(hpStructure->topPointers[threadId] > 0);
	hpStructure->topPointers[threadId]--;
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = NULL;
}
