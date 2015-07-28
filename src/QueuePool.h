/*
 * pool.h
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#ifndef WAITFREEMEMALLOC_SRC_QUEUEPOOL_H_
#define WAITFREEMEMALLOC_SRC_QUEUEPOOL_H_

#include "Queue.h"

typedef struct {
	Queue* queue;
} QueueThread;

typedef struct {
	QueueThread* threads;
	int numberOfThreads;
} QueuePool;

#define GET_QUEUE_THREAD(pool, index) ((pool->threads + index))

QueuePool* createQueuePool(int threads, int elementSize);

void deleteQueuePool(QueuePool* pool);


#endif /* WAITFREEMEMALLOC_SRC_QUEUEPOOL_H_ */
