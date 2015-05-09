/*
 * pool.h
 *
 *  Created on: May 8, 2015
 *      Author: nteg
 */

#ifndef WAITFREEMEMALLOC_SRC_POOL_H_
#define WAITFREEMEMALLOC_SRC_POOL_H_

#include "Stack.h"

typedef struct {
	Stack* stack;
} Thread;

typedef struct {
	Thread* threads;
	int numberOfThreads;
} Pool;

Pool* createPool(int threads);

void deletePool(Pool* pool);

Thread* getThread(Pool* pool, int index);


#endif /* WAITFREEMEMALLOC_SRC_POOL_H_ */
