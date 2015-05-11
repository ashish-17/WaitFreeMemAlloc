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
	return stackPop(thread->stack);
}

bool putInLocalPool(Pool* pool, int threadIndex, Chunk* chunk) {
	printf("inside putInLocalPool with threadIndex %d\n", threadIndex);
	Thread* thread = getThread(pool, threadIndex);
	printf("Stack ptr in putInLocakPool %u\n", thread->stack);
	printf("Stack element size = %u\n", thread->stack->elementSize);
	bool res = stackPush(thread->stack, chunk);
	printf("returned from stackPush\n");
	return res;
}
