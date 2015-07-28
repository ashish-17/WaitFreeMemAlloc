/*
 * pool.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */
#include "QueuePool.h"

#include "commons.h"
#include "Chunk.h"
#include <pthread.h>

QueuePool* createQueuePool(int threads, int elementSize)
{
	LOG_PROLOG();
	QueuePool* pool = (QueuePool*)my_malloc(sizeof(QueuePool));
	pool->threads = (QueueThread*)my_malloc(sizeof(QueueThread) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		QueueThread* ptr = GET_QUEUE_THREAD(pool, i);
		ptr->queue = (Queue*)my_malloc(sizeof(Queue));
		//printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		queueCreate(ptr->queue, elementSize);
		//printf("createQueuePool: queuePtr: %u, headPtr = %u, TailPtr = %u\n", ptr->queue, ptr->queue->head, ptr->queue->tail);
	}
	//printf("after pool create the address of thread ptr is %u\n", pool->threads);
	LOG_EPILOG();
	return pool;
}

void deleteQueuePool(QueuePool* pool)
{
	LOG_PROLOG();
	if (pool != NULL) {
		if (pool->threads != NULL) {
			my_free(pool->threads);
			pool->threads = NULL;
		}
		else {
			LOG_ERROR("Trying to free thread pointer which was NULL");
		}
		pool->numberOfThreads = 0;
		my_free(pool);
		pool = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}
