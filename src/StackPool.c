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
	log_msg_prolog("createStackPool");
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
	log_msg_epilog("createStackPool");
	return pool;
}

void deleteStackPool(StackPool* pool)
{
	log_msg_prolog("deleteStackPool");
	stackFree(pool->threads->stack);
	my_free(pool->threads);
	pool->numberOfThreads = 0;
	log_msg_epilog("deleteStackPool");
}

StackThread* getStackThread(StackPool* pool, int index)
{
	log_msg_prolog("getStackThread");
	StackThread* ptr = (pool->threads + index);
	log_msg_epilog("getStackThread");
	return ptr;
}
