#include "SharedPools.h"
#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

SharedQueuePools* createSharedQueuePools(int threads)
{
	log_msg_prolog("createSharedQueuePools");
	SharedQueuePools* pool = (SharedQueuePools*)my_malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)my_malloc(sizeof(SharedQueuePool) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		SharedQueuePool* ptr = getSharedQueuePool(pool, i);
		ptr->sharedQueuePool = createQueuePool(threads, sizeof(Block));
		//printf("createSQP: SQPPtr = %u, queuePtr = %u, HeadPtr = %u, TailPtr = %u\n", ptr->sharedQueuePool, getQueueThread(ptr->sharedQueuePool,i)->queue, getQueueThread(ptr->sharedQueuePool,i)->queue->head, getQueueThread(ptr->sharedQueuePool,i)->queue->tail);
	}
	log_msg_epilog("createSharedQueuePools");
	return pool;
}

void deleteSharedQueuePools(SharedQueuePools* pool)
{
	/*	stackFree(pool->threads->stack);
	my_free(pool->threads);
	pool->numberOfThreads = 0;*/
}

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadId, int primThreadIndex, int secThreadIndex) {
	log_msg_prolog("getFromSharedQueuePools");
	SharedQueuePool* queuePool = getSharedQueuePool(pool, primThreadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	//printf("getFromSQP: threadId = %d, queuePtr = %u\n", threadIndex, queue);
	//printf("getFRomSQP: setting HP of thread %d for secondary queue head %u\n", threadId, queue->head);
	QueueElement *oldHead = setHazardPointer(globalHPStructure, threadId, queue->head);
	void *ptr = queueDeq(queue, oldHead, threadId);
	log_msg_epilog("getFromSharedQueuePools");
	return ptr;
}

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, Block* block) {
	log_msg_prolog("putInSharedQueuePools");
	SharedQueuePool* queuePool = getSharedQueuePool(pool, threadIndex);
	Queue *queue = getQueueThread(queuePool->sharedQueuePool, secThreadIndex)->queue;
	bool flag = queueEnq(queue, block, secThreadIndex);
	log_msg_epilog("putInSharedQueuePools");
	return flag;
}

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index)
{
	log_msg_prolog("getSharedQueuePool");
	SharedQueuePool *ptr = (pool->sharedQueuePools + index);
	log_msg_epilog("getSharedQueuePool");
	return ptr;
}

