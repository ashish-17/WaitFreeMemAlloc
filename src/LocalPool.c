/*
 * LocalPool.c
 *
 *  Created on: May 8, 2015
 *      Author: nteg
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
	return stackPop(thread->stack);
}

bool putInLocalPool(Pool* pool, int threadIndex, Chunk* chunk) {
	Thread* thread = getThread(pool, threadIndex);
	return stackPush(thread->stack, chunk);
}
