#ifndef WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_
#define WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_

#include "commons.h"
#include "QueuePool.h"
#include "Block.h"

typedef struct {
	QueuePool* sharedQueuePool;
} SharedQueuePool;

typedef struct {
	SharedQueuePool* sharedQueuePools;
	int numberOfThreads;
} SharedQueuePools;

SharedQueuePools* createSharedQueuePools(int threads);

void deleteSharedQueuePools(SharedQueuePools* pool);

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index);

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex);

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, Block *block);

#endif /* WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_ */
