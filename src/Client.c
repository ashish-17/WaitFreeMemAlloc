#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"
#include "HazardPointer.h"


#define NUM_THREADS 3   // 3
#define NUM_BLOCKS  24    // 24
#define CHUNK_SIZE 2
#define NUM_DONATION_STEPS 2

//HPStructure *globalHPStructure = NULL;

// The actual tester
void* clienttester(void *threadId) {
	/*if ((int)threadId == 1) {
		LOG_INFO("thread %d is FINSISHED\n",(int)threadId);
		pthread_exit(NULL);
	}*/
	LOG_INFO("In thread %d\n", (int)threadId);
	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(50);
	LOG_INFO("In thread %d, the totalNumOfOps %d\n", (int)threadId, totalNumOfOps);
	Stack* stack = (Stack*) my_malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(11);

		//LOG_INFO("In thread %d, the flag %d\n", (int)threadId, flag);
		if (flag <= 7) {
			numOfAllocBlocks++;
			Block* block = allocate((int)threadId, 0);
			LOG_INFO("thread %d allocated the block %d with block number %d\n",(int)threadId, block->memBlock, block->threadId);
			stackPush(stack,block);
		}
		else {
			if (numOfAllocBlocks == 0) {
				//LOG_INFO("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				continue;
			}
			else {
				LOG_INFO("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				numOfAllocBlocks--;
				Block *block = stackPop(stack);
				//LOG_INFO("thread %d trying to free the block %d\n",(int)threadId, block->memBlock);
				freeMem((int)threadId, block);
				LOG_INFO("thread %d freed the block %d\n",(int)threadId, block->memBlock);
			}
		}
		totalNumOfOps--;
		//LOG_INFO("thread %d totalNumOfOps remaining %d\n",(int)threadId, totalNumOfOps);
	}
	LOG_INFO("thread %d is FINSISHED\n",(int)threadId);
	pthread_exit(NULL);
}

int tymain() {

	//Wrapper wrapper = (Wrapper*) my_malloc(sizeof(Wrapper));
	globalHPStructure = (HPStructure*)my_malloc(sizeof(HPStructure));
	hpStructureCreate(globalHPStructure, NUM_THREADS, 10);
	createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);


	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		LOG_INFO("In main: creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, clienttester, (void *)t);
		if (rc){
			LOG_INFO("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	// waiting for threads to terminate
	void *status;
	for (int t = 0; t < NUM_THREADS; t++) {
		rc = pthread_join(threads[t], &status);
	}
	LOG_INFO("Test Client\n");
	pthread_exit(NULL);
}
