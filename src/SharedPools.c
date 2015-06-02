#include "SharedPools.h"

#include "Queue.h"
#include "Block.h"

SharedQueuePools* createSharedQueuePools(int threads)
{
	SharedQueuePools* pool = (SharedQueuePools*)malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)malloc(sizeof(SharedQueuePool) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		SharedQueuePool* ptr = getSharedQueuePool(pool, i);
		ptr->sharedQueuePool = createQueuePool(threads, sizeof(Block));
		//printf("createSQP: SQPPtr = %u, queuePtr = %u, HeadPtr = %u, TailPtr = %u\n", ptr->sharedQueuePool, getQueueThread(ptr->sharedQueuePool,i)->queue, getQueueThread(ptr->sharedQueuePool,i)->queue->head, getQueueThread(ptr->sharedQueuePool,i)->queue->tail);
	}
	return pool;
}

void deleteSharedQueuePools(SharedQueuePools* pool)
{
	/*	stackFree(pool->threads->stack);
	free(pool->threads);
	pool->numberOfThreads = 0;*/
}

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex) {
	SharedQueuePool* queuePool = getSharedQueuePool(pool, threadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	//printf("getFromSQP: threadId = %d, queuePtr = %u\n", threadIndex, queue);
	return queueDeq(queue, queue->head);
}

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, Block* block) {
	SharedQueuePool* queuePool = getSharedQueuePool(pool, threadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	return queueEnq(queue, block);
}

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index)
{
	return (pool->sharedQueuePools + index);
}

