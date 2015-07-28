/*
 * pool.h
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#ifndef WAITFREEMEMALLOC_SRC_STACKPOOL_H_
#define WAITFREEMEMALLOC_SRC_STACKPOOL_H_

#include "Stack.h"

typedef struct {
	Stack* stack;
} StackThread;

typedef struct {
	StackThread* threads;
	int numberOfThreads;
} StackPool;

#define GET_STACK_THREAD(pool, index) ((pool->threads + index))

StackPool* createStackPool(int threads);

void deleteStackPool(StackPool* pool);



#endif /* WAITFREEMEMALLOC_SRC_STACKPOOL_H_ */
