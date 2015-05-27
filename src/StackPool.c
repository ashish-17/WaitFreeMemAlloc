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
	StackPool* pool = (StackPool*)malloc(sizeof(StackPool));
	pool->threads = (StackThread*)malloc(sizeof(StackThread) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		StackThread* ptr = getStackThread(pool, i);
		ptr->stack = (Stack*)malloc(sizeof(Stack));
		//printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		stackCreate(ptr->stack, sizeof(Chunk));
	}
	//printf("after pool create the address of thread ptr is %u\n", pool->threads);
	return pool;
}

void deleteStackPool(StackPool* pool)
{
	stackFree(pool->threads->stack);
	free(pool->threads);
	pool->numberOfThreads = 0;
}

StackThread* getStackThread(StackPool* pool, int index)
{
	/*printf("Pool address %u\n", pool);
	printf("address of stacks %u\n", pool->threads);*/
	//printf("size of thread*index %u\n", sizeof(Thread)*index);
	//printf("address of index stack %u\n", pool->threads + sizeof(Thread)*index);
	//printf("**************\n");
	return (pool->threads + index);
}
