#include "SharedPools.h"
#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

HPStructure *globalHPStructure;

SharedQueuePools* createSharedQueuePools(int threads)
{
	LOG_PROLOG();
	SharedQueuePools* pool = (SharedQueuePools*)my_malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)my_malloc(sizeof(SharedQueuePool) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		SharedQueuePool* ptr = GET_SHARED_QUEUE_POOL(pool, i);
		ptr->sharedQueuePool = createQueuePool(threads, sizeof(BLOCK_MEM));
		//printf("createSQP: SQPPtr = %u, queuePtr = %u, HeadPtr = %u, TailPtr = %u\n", ptr->sharedQueuePool, getQueueThread(ptr->sharedQueuePool,i)->queue, getQueueThread(ptr->sharedQueuePool,i)->queue->head, getQueueThread(ptr->sharedQueuePool,i)->queue->tail);
	}
	LOG_EPILOG();
	return pool;
}

void destroySharedQueuePools(SharedQueuePools* pool)
{
	// have to empty the pool herre itself.
	// Since SharedQueuePool can't differentiate whether it contains blocks or chunks
	LOG_PROLOG();
	if (pool != NULL) {
		if (pool->sharedQueuePools != NULL) {
			for (int i = 0; i < pool->numberOfThreads; i++) {
				SharedQueuePool *ptr = GET_SHARED_QUEUE_POOL(pool, i);
				for (int j = 0; j < ptr->sharedQueuePool->numberOfThreads; j++) {
					while (!IS_QUEUE_EMPTY(GET_QUEUE_THREAD(ptr->sharedQueuePool, j)->queue)) {
						BLOCK_MEM block = queueDeqUC(GET_QUEUE_THREAD(ptr->sharedQueuePool,j)->queue);
						destroyBlock(block);
						block = NULL;
					}
					queueFree(GET_QUEUE_THREAD(ptr->sharedQueuePool, j)->queue);
					GET_QUEUE_THREAD(ptr->sharedQueuePool, j)->queue = NULL;
				}
				deleteQueuePool(ptr->sharedQueuePool);
			}
			my_free(pool->sharedQueuePools);
			pool->sharedQueuePools = NULL;
		} else {
			LOG_ERROR("Trying to free sharedQueuePools which is a NULL pointer");
		}
		pool->numberOfThreads = 0;
		my_free(pool);
		pool = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

void* getFromSharedQueuePools(SharedQueuePools* pool, int threadId, int primThreadIndex, int secThreadIndex) {
	LOG_PROLOG();
	SharedQueuePool* queuePool = GET_SHARED_QUEUE_POOL(pool, primThreadIndex);
	Queue *queue = GET_QUEUE_THREAD(queuePool->sharedQueuePool, secThreadIndex)->queue;
	//printf("getFromSQP: threadId = %d, queuePtr = %u\n", threadIndex, queue);
	//printf("getFRomSQP: setting HP of thread %d for secondary queue head %u\n", threadId, queue->head);
	QueueElement *oldHead = setHazardPointer(globalHPStructure, threadId, queue->head);
	void *ptr = queueDeqC(queue, oldHead, threadId);
	LOG_EPILOG();
	return ptr;
}

bool putInSharedQueuePools(SharedQueuePools* pool, int threadIndex, int secThreadIndex, BLOCK_MEM block) {
	LOG_PROLOG();
	SharedQueuePool* queuePool = GET_SHARED_QUEUE_POOL(pool, threadIndex);
	Queue *queue = GET_QUEUE_THREAD(queuePool->sharedQueuePool, secThreadIndex)->queue;
	bool flag = queueEnqC(queue, block, secThreadIndex);
	LOG_EPILOG();
	return flag;
}


