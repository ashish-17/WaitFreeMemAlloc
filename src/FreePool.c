/*
 * FreePool.c
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#include "FreePool.h"
#include "Stack.h"
#include "Queue.h"
#include "Chunk.h"


StackPool* createFreePoolUC(int numThreads) {
	LOG_PROLOG();
	StackPool* freePool = createStackPool(numThreads);
	LOG_EPILOG();
	return freePool;
}

Chunk* getFromFreePoolUC(StackPool* pool, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = getStackThread(pool, threadIndex);
	Chunk* chunk = stackPop(thread->stack);
	LOG_EPILOG();
	return chunk;
}

bool putInFreePoolUC(StackPool* pool, int threadIndex, Chunk* chunk) {
	LOG_PROLOG();
	/*printf("inside putInFreePool with threadIndex %d\n", threadIndex);
	printf("Inside putInfreePool the valeue of pool ptr is %u\n", pool);
	printf("Inside putInfreePool the valeue of StackThread ptr is %u\n", pool->threads);*/
	StackThread* thread = getStackThread(pool, threadIndex);
	/*printf("inside putinFreePool the value of thread ptr is %u\n", thread);
	printf("Stack ptr in putInFreePool %u\n", thread->stack);
	printf("Stack element size = %d\n", thread->stack->elementSize);*/
	bool flag = stackPush(thread->stack, chunk);
	LOG_EPILOG();
	return flag;
}

QueuePool* createFreePoolC(int numThreads) {
	LOG_PROLOG();
	QueuePool* freePool = createQueuePool(numThreads, sizeof(Chunk));
	LOG_EPILOG();
	return freePool;
}

Chunk* getFromFreePoolC(QueuePool* pool, int threadIndex, int primThread, QueueElement *oldQueueHead) {
	LOG_PROLOG();
	QueueThread* thread = getQueueThread(pool, primThread);
	Chunk* chunk = queueDeq(thread->queue, oldQueueHead, threadIndex);
	LOG_EPILOG();
	return chunk;
}

bool putInFreePoolC(QueuePool* pool, int threadIndex, Chunk* chunk) {
	LOG_PROLOG();
	//printf("putInFreePoolC: threadId : %d\n", threadIndex);
	QueueThread* thread = getQueueThread(pool, threadIndex);
	bool flag = queueEnq(thread->queue, chunk, threadIndex);
	LOG_EPILOG();
	return flag;
}


