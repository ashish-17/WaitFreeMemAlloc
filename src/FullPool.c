/*
 * FullPool.c
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#include "FullPool.h"
#include "Stack.h"

StackPool* createFullPool(int numThreads)
{
	StackPool* fullPool = createStackPool(numThreads);

	return fullPool;
}

bool isFullPoolEmpty(StackPool* pool, int threadIndex) {
	StackThread* thread = getStackThread(pool, threadIndex);
	return (stackIsEmpty(thread->stack));
}

Chunk* getFromOtherFullPool(StackPool* pool, int threadIndex) {

	//printf("inside getFromOtherFullPool\n");
	//printf("getFromOtheFullPool: is victim %d stack Empty %d\n", threadIndex, stackIsEmpty(getThread(pool,threadIndex)->stack));
	StackThread* thread = getStackThread(pool, threadIndex);
	//printf("getFromOtheFullPool: thread ptr = %u\n", thread);
	Chunk* chunk = stackPopOther(thread->stack, threadIndex);
	//printf("getFromOtheFullPool: chunk ptr = %u\n", chunk);
	return chunk;
}

Chunk* getFromOwnFullPool(StackPool* pool, int threadIndex) {
	StackThread* thread = getStackThread(pool, threadIndex);
	return (Chunk*)stackPopOwner(thread->stack, threadIndex);
}

bool putInOtherFullPool(StackPool* pool, int threadIndex, Chunk* chunk, AtomicStampedReference* oldTop) {
	StackThread* thread = getStackThread(pool, threadIndex);
	return stackPushOther(thread->stack, chunk, oldTop, threadIndex);
}

bool putInOwnFullPool(StackPool* pool, int threadIndex, Chunk* chunk) {
	StackThread* thread = getStackThread(pool, threadIndex);
	return stackPushOwner(thread->stack, chunk, threadIndex);
}


