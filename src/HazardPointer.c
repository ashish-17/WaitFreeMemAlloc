#include "HazardPointer.h"
#include <assert.h>
#include<pthread.h>

//#define log_debug(str, threadId) printf("%s%d", str, threadId);
#define GET_FREE_QUEUE(queue, index) ((queue + index))

void hpStructureCreate(HPStructure *hpStructure, int noOfThreads, int noOfHP) {
	LOG_PROLOG();
	hpStructure->freeQueues = (FreeQueue*) my_malloc(sizeof(FreeQueue) * noOfThreads);
	for (int i = 0; i < noOfThreads; i++) {
		//getFreeQueue(hpStructure->freeQueues, i)->queue = (CircularQueue*) my_malloc(sizeof(CircularQueue));
		//circularQueueCreate(getFreeQueue(hpStructure->freeQueues, i)->queue, sizeof(int *), noOfThreads * noOfHP);
		GET_FREE_QUEUE(hpStructure->freeQueues, i)->queue = circularQueueCreate(sizeof(int *), noOfThreads * noOfHP);
	}
	//printf("created Circular queues\n");
	// pushing sentinel nodes in all the circular queues
	for (int i = 0; i < noOfThreads; i++) {
		CircularQueue *queue = GET_FREE_QUEUE(hpStructure->freeQueues, i)->queue;
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
	LOG_EPILOG();
}

void hpStructureDestroy(HPStructure *hpStructure) {
	LOG_PROLOG();

	for(int i = 0; i < hpStructure->numberOfThreads; i++) {
		circularQueueFree((GET_FREE_QUEUE(hpStructure->freeQueues, i))->queue);
	}
	my_free(hpStructure->freeQueues);
	hpStructure->freeQueues = NULL;

	for(int i = 0; i < hpStructure->numberOfThreads * hpStructure->numberOfHP; i++) {
		my_free(hpStructure->hazardPointers[i]);
	}
	my_free(hpStructure->hazardPointers);
	hpStructure->hazardPointers = NULL;

	my_free(hpStructure->roundCounters);
	hpStructure->roundCounters = NULL;

	my_free(hpStructure->topPointers);
	hpStructure->topPointers = NULL;

	hpStructure->numberOfHP = 0;
	hpStructure->numberOfThreads = 0;

	LOG_EPILOG();
}


void freeMemHP(HPStructure *hpStructure, int threadId, void *ptr) {
	LOG_PROLOG();
	circularQueueEnq(GET_FREE_QUEUE(hpStructure->freeQueues, threadId)->queue, ptr);

	//LOG_INFO("freeMemHP:  enqueue of %u was successful = %u", ptr, flag);
	void* node = NULL;
	while (node == NULL) {
		void *inspect = hpStructure->hazardPointers[hpStructure->roundCounters[threadId]];
		hpStructure->roundCounters[threadId] = (hpStructure->roundCounters[threadId] + 1) % (hpStructure->numberOfHP * hpStructure->numberOfThreads);

		if (inspect != NULL) {
			setDirty(inspect, 1);
		}
		node = circularQueueDeq(GET_FREE_QUEUE(hpStructure->freeQueues, threadId)->queue);
		if (node == NULL) {
			break;
		}
		else if (isDirty(node) == 0) {
			LOG_INFO("freeing nodeptr = %u", node);
			my_free(node);
			break;
		}
		else {
			setDirty(node, 0);
			circularQueueEnq(GET_FREE_QUEUE(hpStructure->freeQueues, threadId)->queue, node);
		}
	}
	LOG_EPILOG();
	return;
}

void* setHazardPointer(HPStructure *hpStructure, int threadId, void *element) {
	LOG_PROLOG();
	//printf("in setHP\n");
	//printf("hpStructure = %u\n", hpStructure);
	//printf("setHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = element;
	//printf("setHP\n");
	hpStructure->topPointers[threadId]++;
	LOG_EPILOG();
	return element;
}

void clearHazardPointer(HPStructure *hpStructure, int threadId) {
	LOG_PROLOG();
	//printf("clearHP\n");
	//printf("clearHP: thread = %d, topPointer = %d\n", threadId, hpStructure->topPointers[threadId]);
	assert(hpStructure->topPointers[threadId] > 0);
	hpStructure->topPointers[threadId]--;
	hpStructure->hazardPointers[threadId * hpStructure->numberOfHP + hpStructure->topPointers[threadId]] = NULL;
	LOG_EPILOG();
}
