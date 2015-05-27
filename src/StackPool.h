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

StackPool* createStackPool(int threads);

void deleteStackPool(StackPool* pool);

StackThread* getStackThread(StackPool* pool, int index);


#endif /* WAITFREEMEMALLOC_SRC_STACKPOOL_H_ */
