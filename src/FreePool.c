/*
 * FreePool.c
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#include "FreePool.h"
#include "Chunk.h"

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
	Thread* thread = getThread(pool, threadIndex);
	return stackPush(thread->stack, chunk);
}

