#include "ProducerConsumer.h"

SharedQueuePools* createSharedQueuePools(int threads)
{
	SharedQueuePools* pool = (SharedQueuePools*)malloc(sizeof(SharedQueuePools));
	pool->sharedQueuePools = (SharedQueuePool*)malloc(sizeof(SharedQueuePool) * threads);
	pool->numberOfThreads = threads;

	for (int i = 0; i < threads; i++) {
		//printf("Thread ptr = %u \n", getThread(pool, i));
		SharedQueuePool* ptr = getSharedQueuePool(pool, i);
		ptr->sharedQueuePool = (QueuePool*)malloc(sizeof(QueuePool));
		//printf("Stack ptr = %u\n", ptr->stack);
		//printf("Inside create pool and size of chunk is %u\n", sizeof(Chunk));
		createQueuePool(ptr->sharedQueuePool);
	}
	//printf("after pool create the address of thread ptr is %u\n", pool->threads);
	return pool;
}

void deleteSharedQueuePools(SharedQueuePools* pool)
{
/*	stackFree(pool->threads->stack);
	free(pool->threads);
	pool->numberOfThreads = 0;*/
}

SharedQueuePool* getSharedQueuePool(SharedQueuePools* pool, int index)
{
	/*printf("Pool address %u\n", pool);
	printf("address of stacks %u\n", pool->threads);*/
	//printf("size of thread*index %u\n", sizeof(Thread)*index);
	//printf("address of index stack %u\n", pool->threads + sizeof(Thread)*index);
	//printf("**************\n");
	return (pool->sharedQueuePools + index);
}
