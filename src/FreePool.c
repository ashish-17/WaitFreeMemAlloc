/*
 * FreePool.c
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#include "FreePool.h"
#include "Stack.h"

Pool* createFreePool(int numThreads)
{
	Pool* freePool = createPool(numThreads);

	return freePool;
}

Chunk* getFromFreePool(Pool* pool, int threadIndex) {
	Thread* thread = getThread(pool, threadIndex);
	return stackPop(thread->stack);
}

bool putInFreePool(Pool* pool, int threadIndex, Chunk* chunk) {
	/*printf("inside putInFreePool with threadIndex %d\n", threadIndex);
	printf("Inside putInfreePool the valeue of pool ptr is %u\n", pool);
	printf("Inside putInfreePool the valeue of Thread ptr is %u\n", pool->threads);*/
	Thread* thread = getThread(pool, threadIndex);
	/*printf("inside putinFreePool the value of thread ptr is %u\n", thread);
	printf("Stack ptr in putInFreePool %u\n", thread->stack);
	printf("Stack element size = %d\n", thread->stack->elementSize);*/
	return stackPush(thread->stack, chunk);
}
