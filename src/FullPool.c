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
	log_msg_prolog("createFullPool");
	StackPool* fullPool = createStackPool(numThreads);
	log_msg_epilog("createFullPool");
	return fullPool;
}

bool isFullPoolEmpty(StackPool* pool, int threadIndex) {
	log_msg_prolog("isFullPoolEmpty");
	StackThread* thread = getStackThread(pool, threadIndex);
	bool flag = (stackIsEmpty(thread->stack));
	log_msg_epilog("isFullPoolEmpty");
	return flag;
}

Chunk* getFromOtherFullPool(StackPool* pool, int otherThreadId, int threadIndex) {
	log_msg_prolog("getFromOtherFullPool");
	//printf("inside getFromOtherFullPool\n");
	//printf("getFromOtheFullPool: is victim %d stack Empty %d\n", threadIndex, stackIsEmpty(getThread(pool,threadIndex)->stack));
	StackThread* thread = getStackThread(pool, otherThreadId);
	//printf("getFromOtheFullPool: thread ptr = %u\n", thread);
	Chunk* chunk = stackPopOther(thread->stack, otherThreadId, threadIndex);
	//printf("getFromOtheFullPool: chunk ptr = %u\n", chunk);
	log_msg_epilog("getFromOtherFullPool");
	return chunk;
}

Chunk* getFromOwnFullPool(StackPool* pool, int threadIndex) {
	log_msg_prolog("getFromOwnFullPool");
	StackThread* thread = getStackThread(pool, threadIndex);
	Chunk * chunk = (Chunk*)stackPopOwner(thread->stack, threadIndex);
	log_msg_epilog("getFromOwnFullPool");
	return chunk;
}

bool putInOtherFullPool(StackPool* pool, int otherThreadId, Chunk* chunk, ReferenceIntegerPair* oldTop, int threadIndex) {
	log_msg_prolog("putInOtherFullPool");
	StackThread* thread = getStackThread(pool, otherThreadId);
	bool flag = stackPushOther(thread->stack, chunk, oldTop, otherThreadId, threadIndex);
	log_msg_epilog("putInOtherFullPool");
	return flag;
}

bool putInOwnFullPool(StackPool* pool, int threadIndex, Chunk* chunk) {
	log_msg_prolog("putInOwnFullPool");
	StackThread* thread = getStackThread(pool, threadIndex);
	bool flag = stackPushOwner(thread->stack, chunk, threadIndex);
	log_msg_epilog("putInOwnFullPool");
	return flag;
}


