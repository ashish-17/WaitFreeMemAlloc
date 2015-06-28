/*
 * pool.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */
#include "StackPool.h"

#include "commons.h"
#include "Chunk.h"

StackPool* createStackPool(int threads)
{
	LOG_PROLOG();
	StackPool* pool = (StackPool*)my_malloc(sizeof(StackPool));
	pool->threads = (StackThread*)my_malloc(sizeof(StackThread) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		StackThread* ptr = getStackThread(pool, i);
		ptr->stack = (Stack*)my_malloc(sizeof(Stack));
		//printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		stackCreate(ptr->stack, sizeof(Chunk));
	}
	//printf("after pool create the address of thread ptr is %u\n", pool->threads);
	LOG_EPILOG();
	return pool;
}

void deleteStackPool(StackPool* pool)
{
	LOG_PROLOG();
	stackFree(pool->threads->stack);
	my_free(pool->threads);
	pool->numberOfThreads = 0;
	LOG_EPILOG();
}

StackThread* getStackThread(StackPool* pool, int index)
{
	LOG_PROLOG();
	StackThread* ptr = (pool->threads + index);
	LOG_EPILOG();
	return ptr;
}
