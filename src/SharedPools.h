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

void destroySharedQueuePools(SharedQueuePools* pool);

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index);

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadId, int primThreadIndex, int secThreadIndex);

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, BLOCK_MEM block);

#endif /* WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_ */
