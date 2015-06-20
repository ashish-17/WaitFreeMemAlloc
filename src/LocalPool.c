/*
 * LocalPool.c
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#include "LocalPool.h"
#include "Stack.h"

StackPool* createLocalPool(int numThreads)
{
	log_msg_prolog("createLocalPool");
	StackPool* localPool = createStackPool(numThreads);
	log_msg_epilog("createLocalPool");
	return localPool;
}

Chunk* getFromLocalPool(StackPool* pool, int threadIndex) {
	log_msg_prolog("getFromLocalPool");
	StackThread* thread = getStackThread(pool, threadIndex);
	//printf("getFromLocalPool: threadId %d, stack ptr = %u\n",threadIndex, thread->stack);
	Chunk* chunk =  stackPop(thread->stack);
	//printf("getFromLocalPool: threadId %d, chunk ptr = %u\n", threadIndex, chunk);
	log_msg_epilog("getFromLocalPool");
	return chunk;
}

bool putInLocalPool(StackPool* pool, int threadIndex, Chunk* chunk) {
	log_msg_prolog("putInLocalPool");
	//printf("putInLocalPool: Chunk's stack ptr: %u\n", chunk->stack);
	StackThread* thread = getStackThread(pool, threadIndex);
	//printf("Stack ptr in putInLocakPool %u\n", thread->stack);
	//printf("Stack element size = %d\n", thread->stack->elementSize);
	bool res = stackPush(thread->stack, chunk);
	//printf("putInLocalPool: after pushing Chunk ptr: %u\n", thread->stack->top->atomicRef->reference);
	//printf("putInLocalPool: threadId = %d, returned from stackPush\n", threadIndex);
	log_msg_epilog("putInLocalPool");
	return res;
}
