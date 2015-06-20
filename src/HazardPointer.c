#include "HazardPointer.h"
#include <assert.h>
#include<pthread.h>

//#define log_debug(str, threadId) printf("%s%d", str, threadId);


FreeQueue* getFreeQueue(FreeQueue *queue, int index) {
	log_msg_prolog("getFreeQueue");
	FreeQueue* ptr = (queue + index);
	log_msg_epilog("getFreeQueue");
	return ptr;
}

/*int* getHP(int *hazardPointers, int numberOfHP, int priIndex, int secIndex) {
	return (hazardPointers + (priIndex*numberOfHP) + secIndex);
}

uintptr_t combine(void *ptr, bool mark) {
	if (mark) {
		return (((uintptr_t) ptr) | ((uintptr_t) 0x1));
	}
	else {
		return (uintptr_t) ptr;
	}
}*/


void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP) {
	log_msg_prolog("hpStructureCreate");
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
	log_msg_epilog("hpStructureCreate");
}

void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr) {
	log_msg_prolog("freeMemHP");
	bool flag = circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId)->queue, ptr);
	//log_msg("freeMemHP:  enqueue of %u was successful = %u", ptr, flag);
	void* node = NULL;
	while (node == NULL) {
		//printf("came here\n");
		//printf("threadId = %d roundCounter = %u\n", threadId, hpStructure->roundCounters[threadId]);
		//printf("threadId = %d, size of queue = %d\n", threadId,hpStructure->numberOfHP * hpStructure->numberOfThreads);
		void *inspect = hpStructure->hazardPointers[hpStructure->roundCounters[threadId]];
		hpStructure->roundCounters[threadId] = (hpStructure->roundCounters[threadId] + 1) % (hpStructure->numberOfHP * hpStructure->numberOfThreads);
		//printf("freeMemHP: threadId = %d roundCounter = %u\n", threadId, hpStructure->roundCounters[threadId]);
		//printf("threadId = %d inspectPtr = %u\n", threadId, inspect);
		if (inspect != NULL) {
			setDirty(inspect, 1);
		}
		node = circularQueueDeq(getFreeQueue(hpStructure->freeQueues, threadId)->queue);
		//printf("freeMemHp: threadId = %d nodePtr = %u dequeued\n", threadId, node);
		if (node == NULL) {
			//printf("threadId = %d node dequeued was null\n", threadId);
			break;
		}
		else if (isDirty(node) == 0) {
			log_msg("freeing nodeptr = %u", node);
			my_free(node);
			break;
		}
		else {
			//log_msg("***** threadID = %d marking the non null node\n", threadId);
			setDirty(node, 0);
			circularQueueEnq(getFreeQueue(hpStructure->freeQueues, threadId)->queue, node);
		}
	}
	log_msg_epilog("freeMemHP");
	return;
}

void* setHazardPointer(HPStructure *hpStructure, int threadId, void *element) {
	log_msg_prolog("setHazardPointer");
	//printf("in setHP\n");
	//printf("hpStructure = %u\n", hpStructure);
	//printf("setHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = element;
	//printf("setHP\n");
	hpStructure->topPointers[threadId]++;
	log_msg_epilog("setHazardPointer");
	return element;
}

void clearHazardPointer(HPStructure *hpStructure, int threadId) {
	log_msg_prolog("clearHazardPointer");
	//printf("clearHP\n");
	//printf("clearHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	assert(hpStructure->topPointers[threadId] > 0);
	hpStructure->topPointers[threadId]--;
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = NULL;
	log_msg_epilog("clearHazardPointer");
}
