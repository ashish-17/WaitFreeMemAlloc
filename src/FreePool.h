/*
 * FreePool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_FREEPOOL_H_
#define WAITFREEMEMALLOC_SRC_FREEPOOL_H_

#include "commons.h"
#include "Chunk.h"
#include "StackPool.h"
#include "QueuePool.h"

StackPool* createFreePoolUC(int numThreads);

Chunk* getFromFreePoolUC(StackPool* pool, int threadIndex);

bool putInFreePoolUC(StackPool* pool, int threadIndex, Chunk* chunk);

QueuePool* createFreePoolC(int numThreads);

Chunk* getFromFreePoolC(QueuePool* pool, int threadIndex, int primThread, QueueElement *oldQueueHead);

bool putInFreePoolC(QueuePool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_FREEPOOL_H_ */
