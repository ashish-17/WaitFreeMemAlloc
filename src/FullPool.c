/*
 * FullPool.c
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#include "FullPool.h"

Pool* createFullPool(int numThreads)
{
	Pool* fullPool = createPool(numThreads);

	return fullPool;
}

bool isFullPoolEmpty(Pool* pool, int threadIndex) {
	Thread* thread = getThread(pool, threadIndex);
	return (stackIsEmpty(thread->stack));
}

Chunk* getFromOtherFullPool(Pool* pool, int threadIndex) {
	Thread* thread = getThread(pool, threadIndex);
	return (Chunk*)stackPopOther(thread->stack);
}

Chunk* getFromOwnFullPool(Pool* pool, int threadIndex) {
	Thread* thread = getThread(pool, threadIndex);
	return (Chunk*)stackPopOwner(thread->stack);
}

bool putInOtherFullPool(Pool* pool, int threadIndex, Chunk* chunk, AtomicStampedReference* oldTop) {
	Thread* thread = getThread(pool, threadIndex);
	return stackPushOther(thread->stack, chunk, oldTop);
}

bool putInOwnFullPool(Pool* pool, int threadIndex, Chunk* chunk) {
	Thread* thread = getThread(pool, threadIndex);
	return stackPushOwner(thread->stack, chunk);
}

