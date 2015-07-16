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
		//LOG_INFO("thread ptr at index %i is %u\n", i, ptr);
		//LOG_INFO("Stack ptr = %u", ptr->stack);
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
	if (pool != NULL) {
		for (int i = 0; i < pool->numberOfThreads; i++) {
			StackThread *thread = getStackThread(pool, i);
			if (thread != NULL) {
				while (!stackIsEmpty(thread->stack)) {
					Chunk* chunk = stackPop(thread->stack);
					destroyChunk(chunk);
					chunk = NULL;
				}
				stackFree(thread->stack);
			}
			else {
				LOG_ERROR("Trying to free thread NULL pointer");
			}
		}
		my_free(pool->threads);
		pool->threads = NULL;
		pool->numberOfThreads = 0;
		my_free(pool);
		pool = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

StackThread* getStackThread(StackPool* pool, int index)
{
	LOG_PROLOG();
	StackThread* ptr = (pool->threads + index);
	LOG_EPILOG();
	return ptr;
}
