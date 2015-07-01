#include "SharedPools.h"
#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

SharedQueuePools* createSharedQueuePools(int threads)
{
	LOG_PROLOG();
	SharedQueuePools* pool = (SharedQueuePools*)my_malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)my_malloc(sizeof(SharedQueuePool) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		SharedQueuePool* ptr = getSharedQueuePool(pool, i);
		ptr->sharedQueuePool = createQueuePool(threads, sizeof(Block));
		//printf("createSQP: SQPPtr = %u, queuePtr = %u, HeadPtr = %u, TailPtr = %u\n", ptr->sharedQueuePool, getQueueThread(ptr->sharedQueuePool,i)->queue, getQueueThread(ptr->sharedQueuePool,i)->queue->head, getQueueThread(ptr->sharedQueuePool,i)->queue->tail);
	}
	LOG_EPILOG();
	return pool;
}

void destroySharedQueuePools(SharedQueuePools* pool)
{
	for (int i = 0; i < pool->numberOfThreads; i++) {
		deleteQueuePool(getSharedQueuePool(pool, i));
	}
	my_free(pool->sharedQueuePools);
	pool->numberOfThreads = 0;
	my_free(pool);
}

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadId, int primThreadIndex, int secThreadIndex) {
	LOG_PROLOG();
	SharedQueuePool* queuePool = getSharedQueuePool(pool, primThreadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	//printf("getFromSQP: threadId = %d, queuePtr = %u\n", threadIndex, queue);
	//printf("getFRomSQP: setting HP of thread %d for secondary queue head %u\n", threadId, queue->head);
	QueueElement *oldHead = setHazardPointer(globalHPStructure, threadId, queue->head);
	void *ptr = queueDeqC(queue, oldHead, threadId);
	LOG_EPILOG();
	return ptr;
}

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, Block* block) {
	LOG_PROLOG();
	SharedQueuePool* queuePool = getSharedQueuePool(pool, threadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	bool flag = queueEnqC(queue, block, secThreadIndex);
	LOG_EPILOG();
	return flag;
}

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index)
{
	LOG_PROLOG();
	SharedQueuePool *ptr = (pool->sharedQueuePools + index);
	LOG_EPILOG();
	return ptr;
}

