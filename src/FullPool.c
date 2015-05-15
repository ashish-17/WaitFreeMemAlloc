/*
 * FullPool.c
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#include "FullPool.h"
#include "Stack.h"

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

	//printf("inside getFromOtherFullPool\n");
	//printf("getFromOtheFullPool: is victim %d stack Empty %d\n", threadIndex, stackIsEmpty(getThread(pool,threadIndex)->stack));
	Thread* thread = getThread(pool, threadIndex);
	//printf("getFromOtheFullPool: thread ptr = %u\n", thread);
	Chunk* chunk = stackPopOther(thread->stack);
	//printf("getFromOtheFullPool: chunk ptr = %u\n", chunk);
	return chunk;
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


