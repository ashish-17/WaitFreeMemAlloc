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

QueuePool* createQueuePool(int threads);

void deleteQueuePool(QueuePool* pool);

QueueThread* getQueueThread(QueuePool* pool, int index);


#endif /* WAITFREEMEMALLOC_SRC_QUEUEPOOL_H_ */
