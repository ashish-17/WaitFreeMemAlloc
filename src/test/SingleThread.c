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
#include <stdio.h>
#include <time.h>

typedef struct _ThreadData {
	int allocatorNo;
	int nThreads;
	int objSize;
	int iterations;
	int repetitions;
	int threadId;
	void *obj;
} ThreadData;


void* workerWaitFreePool(void *data) {
	LOG_PROLOG();
	ThreadData* threadData = (ThreadData*) data;
	printf("threadid %d \n", threadData->threadId);
	char **ptr = (char**) malloc(sizeof(char*) * threadData->iterations);
	for (int i = 0; i < threadData->iterations; i++) {
		ptr[i] = allocate(threadData->threadId, 0);
		LOG_INFO("thread %d ptr got is %u\n", threadData->threadId, ptr);
	}
	for (int i = 0; i < threadData->iterations; i++) {
		freeMem(threadData->threadId, ptr[i]);
		LOG_INFO("thread %d ptr got is %u\n", threadData->threadId, ptr);
	}
	free(ptr);
	LOG_EPILOG();
	return NULL;
}



int csdcsdcmain() {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();
	LOG_PROLOG();

	int allocatorNo, nThreads, objSize, iterations, repetitions;
	clock_t start, diff;

	allocatorNo = 1;
	nThreads = 1;
	objSize = 100;
	iterations = 10000;
	repetitions = 1000;


	ThreadData *threadData = (ThreadData*)malloc(nThreads * sizeof(ThreadData));
	pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * nThreads);
	int rc;

	if (allocatorNo == 1) {
		int nBlocks = nThreads * iterations;
		createWaitFreePool(nBlocks, nThreads, iterations, iterations, objSize); // nBlocks, nThreads, chunkSize, donationsSteps

	}
	LOG_INFO("hereeee\n");
	start = clock();
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
	diff = clock() - start;
	if (allocatorNo == 1) {
		destroyWaitFreePool();
	}
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("%d", msec);
	free(threadData);

	LOG_EPILOG();
	//printf("Test Client");
	LOG_CLOSE();
	return 0;
}



