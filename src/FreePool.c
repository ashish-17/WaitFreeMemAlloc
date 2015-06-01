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
	StackPool* freePool = createStackPool(numThreads);
	return freePool;
}

Chunk* getFromFreePoolUC(StackPool* pool, int threadIndex) {
	StackThread* thread = getStackThread(pool, threadIndex);
	return stackPop(thread->stack);
}

bool putInFreePoolUC(StackPool* pool, int threadIndex, Chunk* chunk) {
	/*printf("inside putInFreePool with threadIndex %d\n", threadIndex);
	printf("Inside putInfreePool the valeue of pool ptr is %u\n", pool);
	printf("Inside putInfreePool the valeue of StackThread ptr is %u\n", pool->threads);*/
	StackThread* thread = getStackThread(pool, threadIndex);
	/*printf("inside putinFreePool the value of thread ptr is %u\n", thread);
	printf("Stack ptr in putInFreePool %u\n", thread->stack);
	printf("Stack element size = %d\n", thread->stack->elementSize);*/
	return stackPush(thread->stack, chunk);
}

QueuePool* createFreePoolC(int numThreads) {
	QueuePool* freePool = createQueuePool(numThreads, sizeof(Chunk));
	return freePool;
}

Chunk* getFromFreePoolC(QueuePool* pool, int threadIndex) {
	QueueThread* thread = getQueueThread(pool, threadIndex);
		return queueDeq(thread->queue);
}

bool putInFreePoolC(QueuePool* pool, int threadIndex, Chunk* chunk) {
		QueueThread* thread = getQueueThread(pool, threadIndex);
		return queueEnq(thread->queue, chunk);
}
