/*
 * SingleThread.c
 *
 *  Created on: 14-Jul-2015
 *      Author: architaagarwal
 */

#include "../WaitFreePool.h"
#include "RandomGenerator.h"
#include "../commons.h"
#include <pthread.h>
# include "../Block.h"
#include "../Stack.h"
//#include "HazardPointer.h"
#include "CodeCorrectness.h"

typedef struct _ThreadData {
	int allocatorNo;
	int nThreads;
	int objSize;
	int iterations;
	int repetitions;
	int threadId;
	void *obj;
} ThreadData;


void workerWaitFreePool(void *data) {
	LOG_PROLOG();
	ThreadData* threadData = (ThreadData*) data;
	freeMem(threadData->threadId, threadData->obj);
	for (int i = 0; i < threadData->iterations; i++) {
		char* ptr = allocate(threadData->threadId, 0);
		LOG_INFO("thread %d ptr got is %u\n", threadData->threadId, ptr);
		// Write into ptr a bunch of times
		for (int j = 0; j < threadData->repetitions; j++) {
			for  (int k = 0; k < threadData->objSize; k++) {
				*(ptr + k) = (char)k;
			}
		}
		freeMem(threadData->threadId, ptr);
	}
	LOG_EPILOG();
}



int main(int argc, char* argv[]) {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();
	LOG_PROLOG();

	int allocatorNo, nThreads, objSize, iterations, repetitions;

		allocatorNo = 1;
		nThreads = 1;
		objSize = 8;
		iterations = 10000;
		repetitions = 1000;


	ThreadData *threadData = (ThreadData*)malloc(nThreads * sizeof(ThreadData));
	pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * nThreads);
	int rc;

	if (allocatorNo == 1) {
		int nBlocks = nThreads * 1 + 3 * nThreads;
		createWaitFreePool(nBlocks, nThreads, 1, iterations); // nBlocks, nThreads, chunkSize, donationsSteps
		//hashTableCreate(nBlocks);
		for (int t = 0; t < nThreads; t++) {
			threadData[t].obj = allocate(0,1);
		}
	}

	for (int t = 0; t < nThreads; t++) {
		threadData[t].allocatorNo = allocatorNo;
		threadData[t].nThreads = nThreads;
		threadData[t].objSize = objSize;
		threadData[t].iterations = iterations;
		threadData[t].repetitions = repetitions;
		threadData[t].threadId = t;

		if (allocatorNo == 1) {
			rc = pthread_create((threads + t), NULL, workerWaitFreePool, (threadData + t));
		}

		if (rc) {
			printf("ERROR; return code from pthread_create() is %d", rc);
			exit(-1);
		}
	}

	void *status;
	for (int t = 0; t < nThreads; t++) {
		rc = pthread_join(threads[t], &status);
	}

	if (allocatorNo == 1) {
		destroyWaitFreePool();
	}

	free(threadData);

	LOG_EPILOG();
	LOG_INFO("Test Client");
	LOG_CLOSE();
}



