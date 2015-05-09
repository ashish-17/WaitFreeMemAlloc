/*
 * LocalPool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_LOCALPOOL_H_
#define WAITFREEMEMALLOC_SRC_LOCALPOOL_H_

#include "commons.h"

Pool* createLocalPool(int numThreads);

Chunk* getFromLocalPool(Pool* pool, int threadIndex);

bool putInLocalPool(Pool* pool, int threadIndex, Chunk* chunk);

#endif /* WAITFREEMEMALLOC_SRC_LOCALPOOL_H_ */
