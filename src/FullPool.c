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
	LOG_PROLOG();
	StackPool* fullPool = createStackPool(numThreads);
	LOG_EPILOG();
	return fullPool;
}

void destroyFullPool(StackPool *pool) {
	LOG_PROLOG();
	deleteStackPool(pool);
	pool = NULL;
	LOG_EPILOG();
}

bool isFullPoolEmpty(StackPool* pool, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = getStackThread(pool, threadIndex);
	bool flag = (stackIsEmpty(thread->stack));
	LOG_EPILOG();
	return flag;
}

Chunk* getFromOtherFullPool(StackPool* pool, int otherThreadId, int threadIndex) {
	LOG_PROLOG();
	//printf("inside getFromOtherFullPool\n");
	//printf("getFromOtheFullPool: is victim %d stack Empty %d\n", threadIndex, stackIsEmpty(getThread(pool,threadIndex)->stack));
	StackThread* thread = getStackThread(pool, otherThreadId);
	//printf("getFromOtheFullPool: thread ptr = %u\n", thread);
	Chunk* chunk = stackPopOther(thread->stack, threadIndex);
	//printf("getFromOtheFullPool: chunk ptr = %u\n", chunk);
	LOG_EPILOG();
	return chunk;
}

Chunk* getFromOwnFullPool(StackPool* pool, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = getStackThread(pool, threadIndex);
	Chunk * chunk = (Chunk*)stackPopOwner(thread->stack, threadIndex);
	LOG_EPILOG();
	return chunk;
}

bool putInOtherFullPool(StackPool* pool, int otherThreadId, Chunk* chunk, ReferenceIntegerPair* oldTop, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = getStackThread(pool, otherThreadId);
	bool flag = stackPushOther(thread->stack, chunk, oldTop, threadIndex);
	LOG_EPILOG();
	return flag;
}

bool putInOwnFullPool(StackPool* pool, int threadIndex, Chunk* chunk) {
	LOG_PROLOG();
	StackThread* thread = getStackThread(pool, threadIndex);
	bool flag = stackPushOwner(thread->stack, chunk, threadIndex);
	LOG_EPILOG();
	return flag;
}


