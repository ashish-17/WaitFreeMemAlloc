#include "SharedPools.h"
#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

SharedQueuePools* createSharedQueuePools(int threads)
{
	SharedQueuePools* pool = (SharedQueuePools*)my_malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)my_malloc(sizeof(SharedQueuePool) * threads);
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
	my_free(pool->threads);
	pool->numberOfThreads = 0;*/
}

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadId, int primThreadIndex, int secThreadIndex) {
	SharedQueuePool* queuePool = getSharedQueuePool(pool, primThreadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	//printf("getFromSQP: threadId = %d, queuePtr = %u\n", threadIndex, queue);
	printf("getFRomSQP: setting HP of thread %d for secondary queue head %u\n", threadId, queue->head);
	QueueElement *oldHead = setHazardPointer(globalHPStructure, threadId, queue->head);
	return queueDeq(queue, oldHead, threadId);
}

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, Block* block) {
	SharedQueuePool* queuePool = getSharedQueuePool(pool, threadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	return queueEnq(queue, block, secThreadIndex);
}

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index)
{
	return (pool->sharedQueuePools + index);
}

