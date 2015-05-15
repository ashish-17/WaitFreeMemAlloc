/*
 * LocalPool.c
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#include "LocalPool.h"
#include "Stack.h"

Pool* createLocalPool(int numThreads)
{
	Pool* localPool = createPool(numThreads);

	return localPool;
}

Chunk* getFromLocalPool(Pool* pool, int threadIndex) {
	Thread* thread = getThread(pool, threadIndex);
	//printf("getFromLocalPool: threadId %d, stack ptr = %u\n",threadIndex, thread->stack);
	Chunk* chunk =  stackPop(thread->stack);
	//printf("getFromLocalPool: threadId %d, chunk ptr = %u\n", threadIndex, chunk);
	return chunk;
}

bool putInLocalPool(Pool* pool, int threadIndex, Chunk* chunk) {
	//printf("putInLocalPool: Chunk's stack ptr: %u\n", chunk->stack);
	Thread* thread = getThread(pool, threadIndex);
	//printf("Stack ptr in putInLocakPool %u\n", thread->stack);
	//printf("Stack element size = %d\n", thread->stack->elementSize);
	bool res = stackPush(thread->stack, chunk);
	//printf("putInLocalPool: after pushing Chunk ptr: %u\n", thread->stack->top->atomicRef->reference);
	//printf("returned from stackPush\n");
	return res;
}
