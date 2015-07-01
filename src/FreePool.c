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

void destroyFreePoolUC(StackPool *pool) {
	LOG_PROLOG();
	deleteStackPool(pool); // deleteStackPool will empty the StackPool
	pool = NULL;
	LOG_EPILOG();
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

void destroyFreePoolC(QueuePool *pool) {
	LOG_PROLOG();
	// have to empty the pool here.
	// Can't push the work to deleteQueuePool
	// as we are pushing Chunks in this QueuePool and Blocks in the QueuePool of SharedPools.
	// So deleteQueuePool can't differentiate and memory leak might happen.
	if (pool != NULL) {
		for (int i = 0; i < pool->numberOfThreads; i++) {
			QueueThread* thread = getQueueThread(pool, i);
			if (thread != NULL) {
				while (!isQueueEmpty(thread->queue)) {
					Chunk *chunk = queueDeqUC(thread->queue);
					destroyChunk(chunk);
				}
				queueFree(thread->queue);
				my_free(thread);
				thread = NULL;
			}
			else {
				LOG_ERROR("Trying to free NULL pointer");
			}
		}
		deleteQueuePool(pool);
		pool = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

Chunk* getFromFreePoolC(QueuePool* pool, int threadIndex, int primThread, QueueElement *oldQueueHead) {
	LOG_PROLOG();
	QueueThread* thread = getQueueThread(pool, primThread);
	Chunk* chunk = queueDeqC(thread->queue, oldQueueHead, threadIndex);
	LOG_EPILOG();
	return chunk;
}

bool putInFreePoolC(QueuePool* pool, int threadIndex, Chunk* chunk) {
	LOG_PROLOG();
	QueueThread* thread = getQueueThread(pool, threadIndex);
	bool flag = queueEnqC(thread->queue, chunk, threadIndex);
	LOG_EPILOG();
	return flag;
}


