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
	log_msg_prolog("createFreePoolUC");
	StackPool* freePool = createStackPool(numThreads);
	log_msg_epilog("createFreePoolUC");
	return freePool;
}

Chunk* getFromFreePoolUC(StackPool* pool, int threadIndex) {
	log_msg_prolog("getFromFreePoolUC");
	StackThread* thread = getStackThread(pool, threadIndex);
	Chunk* chunk = stackPop(thread->stack);
	log_msg_epilog("getFromFreePoolUC");
	return chunk;
}

bool putInFreePoolUC(StackPool* pool, int threadIndex, Chunk* chunk) {
	log_msg_prolog("putInFreePoolUC");
	/*printf("inside putInFreePool with threadIndex %d\n", threadIndex);
	printf("Inside putInfreePool the valeue of pool ptr is %u\n", pool);
	printf("Inside putInfreePool the valeue of StackThread ptr is %u\n", pool->threads);*/
	StackThread* thread = getStackThread(pool, threadIndex);
	/*printf("inside putinFreePool the value of thread ptr is %u\n", thread);
	printf("Stack ptr in putInFreePool %u\n", thread->stack);
	printf("Stack element size = %d\n", thread->stack->elementSize);*/
	bool flag = stackPush(thread->stack, chunk);
	log_msg_epilog("putInFreePoolUC");
	return flag;
}

QueuePool* createFreePoolC(int numThreads) {
	log_msg_prolog("createFreePoolC");
	QueuePool* freePool = createQueuePool(numThreads, sizeof(Chunk));
	log_msg_epilog("createFreePoolC");
	return freePool;
}

Chunk* getFromFreePoolC(QueuePool* pool, int threadIndex, int primThread, QueueElement *oldQueueHead) {
	log_msg_prolog("getFromFreePoolC");
	QueueThread* thread = getQueueThread(pool, primThread);
	Chunk* chunk = queueDeq(thread->queue, oldQueueHead, threadIndex);
	log_msg_epilog("getFromFreePoolC");
	return chunk;
}

bool putInFreePoolC(QueuePool* pool, int threadIndex, Chunk* chunk) {
	log_msg_prolog("putInFreePoolC");
	//printf("putInFreePoolC: threadId : %d\n", threadIndex);
	QueueThread* thread = getQueueThread(pool, threadIndex);
	bool flag = queueEnq(thread->queue, chunk, threadIndex);
	log_msg_epilog("putInFreePoolC");
	return flag;
}


