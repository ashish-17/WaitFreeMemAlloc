/*
 * pool.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */
#include "QueuePool.h"

#include "commons.h"
#include "Chunk.h"

QueuePool* createQueuePool(int threads, int elementSize)
{
	QueuePool* pool = (QueuePool*)malloc(sizeof(QueuePool));
	pool->threads = (QueueThread*)malloc(sizeof(QueueThread) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		QueueThread* ptr = getQueueThread(pool, i);
		ptr->queue = (Queue*)malloc(sizeof(Queue));
		//printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		queueCreate(ptr->queue, elementSize);
		printf("createQueuePool: queuePtr: %u, headPtr = %u, TailPtr = %u\n", ptr->queue, ptr->queue->head, ptr->queue->tail);
	}
	//printf("after pool create the address of thread ptr is %u\n", pool->threads);
	return pool;
}

void deleteQueuePool(QueuePool* pool)
{
	queueFree(pool->threads->queue);
	free(pool->threads);
	pool->numberOfThreads = 0;
}

QueueThread* getQueueThread(QueuePool* pool, int index)
{
	/*printf("Pool address %u\n", pool);
	printf("address of stacks %u\n", pool->threads);*/
	//printf("size of thread*index %u\n", sizeof(Thread)*index);
	//printf("address of index stack %u\n", pool->threads + sizeof(Thread)*index);
	//printf("**************\n");
	return (pool->threads + index);
}
