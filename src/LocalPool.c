/*
 * LocalPool.c
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#include "LocalPool.h"
#include "Stack.h"

StackPool* createLocalPool(int numThreads)
{
	LOG_PROLOG();
	StackPool* localPool = createStackPool(numThreads);
	LOG_EPILOG();
	return localPool;
}

void destroyLocalPool(StackPool *pool) {
	LOG_PROLOG();
	deleteStackPool(pool);
	pool = NULL;
	LOG_EPILOG();
}

Chunk* getFromLocalPool(StackPool* pool, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = GET_STACK_THREAD(pool, threadIndex);
	Chunk *chunk = ((StackElement*)(thread->stack->top->atomicRef->reference))->value;
	LOG_EPILOG();
	return chunk;
}

Chunk* removeFromLocalPool(StackPool* pool, int threadIndex) {
	LOG_PROLOG();
	StackThread* thread = GET_STACK_THREAD(pool, threadIndex);
	Chunk* chunk =  stackPop(thread->stack);
	//printf("getFromLocalPool: threadId %d, chunk ptr = %u\n", threadIndex, chunk);
	LOG_EPILOG();
	return chunk;
}

bool putInLocalPool(StackPool* pool, int threadIndex, Chunk* chunk) {
	LOG_PROLOG();
	//printf("putInLocalPool: Chunk's stack ptr: %u\n", chunk->stack);
	StackThread* thread = GET_STACK_THREAD(pool, threadIndex);
	//LOG_INFO("thread ptr in putInLocakPool %u\n", thread);
	//LOG_INFO("Stack ptr in putInLocakPool %u\n", thread->stack);
	//printf("Stack element size = %d\n", thread->stack->elementSize);
	bool res = stackPush(thread->stack, chunk);
	//printf("putInLocalPool: after pushing Chunk ptr: %u\n", thread->stack->top->atomicRef->reference);
	//printf("putInLocalPool: threadId = %d, returned from stackPush\n", threadIndex);
	LOG_EPILOG();
	return res;
}
