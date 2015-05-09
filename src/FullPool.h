/*
 * LocalPool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_FULLPOOL_H_
#define WAITFREEMEMALLOC_SRC_FULLPOOL_H_

#include "commons.h"

Pool* createFullPool(int numThreads);

bool isFullPoolEmpty(Pool* pool, int threadIndex);

Chunk* getFromOtherFullPool(Pool* pool, int threadIndex);

Chunk* getFromOwnFullPool(Pool* pool, int threadIndex);

bool putInOtherFullPool(Pool* pool, int threadIndex, Chunk* chunk, AtomicStampedReference* oldTop);

bool putInOwnFullPool(Pool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_FULLPOOL_H_ */
