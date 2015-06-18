/*
 * LocalPool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_FULLPOOL_H_
#define WAITFREEMEMALLOC_SRC_FULLPOOL_H_

#include "commons.h"
#include "Chunk.h"
#include "StackPool.h"

StackPool* createFullPool(int numThreads);

bool isFullPoolEmpty(StackPool* pool, int threadIndex);

Chunk* getFromOtherFullPool(StackPool* pool, int otherThreadId, int threadIndex);

Chunk* getFromOwnFullPool(StackPool* pool, int threadIndex);

bool putInOtherFullPool(StackPool* pool, int otherThreadId, Chunk* chunk, ReferenceIntegerPair* oldTop, int threadIndex);

bool putInOwnFullPool(StackPool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_FULLPOOL_H_ */
