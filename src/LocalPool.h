/*
 * LocalPool.h
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#ifndef WAITFREEMEMALLOC_SRC_LOCALPOOL_H_
#define WAITFREEMEMALLOC_SRC_LOCALPOOL_H_

#include "commons.h"
#include "Chunk.h"
#include "StackPool.h"

StackPool* createLocalPool(int numThreads);

Chunk* getFromLocalPool(StackPool* pool, int threadIndex);

bool putInLocalPool(StackPool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_LOCALPOOL_H_ */
