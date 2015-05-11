/*
 * pool.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */
#include "commons.h"
#include "pool.h"
#include "Chunk.h"

Pool* createPool(int threads)
{
	Pool* pool = (Pool*)malloc(sizeof(Pool));
	pool->threads = (Thread*)malloc(sizeof(Thread) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		Thread* ptr = getThread(pool, i);
		ptr->stack = (Stack*)malloc(sizeof(Stack));
		printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		stackCreate(ptr->stack, sizeof(Chunk));
		//printf("after pool create the sizze of element is %u\n", ptr->stack->elementSize);
	}
	return pool;
}

void deletePool(Pool* pool)
{
	stackFree(pool->threads->stack);
	free(pool->threads);
	pool->numberOfThreads = 0;
}

Thread* getThread(Pool* pool, int index)
{
	//printf("address of stacks %u\n", pool->threads);
	//printf("size of thread*index %u\n", sizeof(Thread)*index);
	//printf("address of index stack %u\n", pool->threads + sizeof(Thread)*index);
	//printf("**************\n");
	return (pool->threads + sizeof(Thread)*index);
}
