/*
 * pool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_POOL_H_
#define WAITFREEMEMALLOC_SRC_POOL_H_

#include "Stack.h"

typedef struct {
	Stack* stack;
} Thread;

typedef struct {
	Thread* threads;
	int numberOfThreads;
} Pool;

Pool* createPool(int threads)
{
	Pool* pool = (Pool*)malloc(sizeof(Pool));
	pool->threads = (Thread*)malloc(sizeof(Thread) * threads);
	pool->numberOfThreads = threads;

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
	return (pool->threads + sizeof(Thread)*index);
}


#endif /* WAITFREEMEMALLOC_SRC_POOL_H_ */
