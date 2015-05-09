/*
 * FreePool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_FREEPOOL_H_
#define WAITFREEMEMALLOC_SRC_FREEPOOL_H_

#include "commons.h"
#include "pool.h"
#include "Chunk.h"

Pool* createFreePool(int numThreads);

Chunk* getFromFreePool(Pool* pool, int threadIndex);

bool putInFreePool(Pool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_FREEPOOL_H_ */
