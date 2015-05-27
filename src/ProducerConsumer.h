#ifndef WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_
#define WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_

#include "commons.h"
#include "QueuePool.h"

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

#endif /* WAITFREEMEMALLOC_SRC_PRODUCERCONSUMER_H_ */
