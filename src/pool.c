/*
 * pool.c
 *
 *  Created on: May 9, 2015
 *      Author: nteg
 */

#include "pool.h"

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
