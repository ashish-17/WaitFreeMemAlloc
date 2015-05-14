#include "stdio.h"
#include "Stack.h"
#include "Chunk.h"
//#include "LocalPool.h"
//#include "FreePool.h"
#include "FullPool.h"
#include "pool.h"
#include "Block.h"
#include <pthread.h>
#include "RandomGenerator.h"

#define NUM_THREADS 5
#define NUM_CHUNKS_PER_THREAD 15
#define NUM_BLOCKS_PER_CHUNK 3

Pool *fullPool;

void* testMultiFullPool1(void *threadId) {
	printf("Hre... in thread %d\n", (int)threadId);
	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(50);
	printf("In thread %d, the totalNumOfOps %d\n", (int)threadId, totalNumOfOps);
	Stack* stack = (Stack*) malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(2);
		printf("In thread %d, the flag %d\n", (int)threadId, flag);
		if (flag == 0) {
			Chunk* chunk = getFromOwnFullPool(fullPool,(int)threadId);
			//printf("In thread %d, chunk ptr %u\n", (int)threadId, chunk);
			while (!isChunkEmpty(chunk)) {
				numOfAllocBlocks++;
				Block *block = getFromChunk(chunk);
				//printf("In thread %d, chunk ptr %u\n", (int)threadId, chunk);
				printf("thread %d allocated the block %d\n",(int)threadId, block->memBlock);
				stackPush(stack,block);
			}
		}
		else {
			if (numOfAllocBlocks == 0) {
				totalNumOfOps--;
				//printf("thread %d decreased totalnumofOps %d\n",(int)threadId, totalNumOfOps);
				continue;
			}
			else {
				Block* block;
				Chunk *chunk = createChunk(chunk,NUM_BLOCKS_PER_CHUNK);
				for (int i = 0; i < NUM_BLOCKS_PER_CHUNK; i++) {
					block = stackPop(stack);
					putInChunk(chunk,block);
					numOfAllocBlocks--;
					printf("thread %d freed the block %d\n",(int)threadId, block->memBlock);
				}
				putInOwnFullPool(fullPool,(int)threadId,chunk);

			}
		}
		totalNumOfOps--;
		//printf("thread %d decreased totalnumofOps %d\n",(int)threadId, totalNumOfOps);
	}
	printf("thread %d is FINSISHED\n",(int)threadId);
	pthread_exit(NULL);
}

void testMultiFullPool() {
	int numOfThreads = NUM_THREADS;
	int numOfChunks = NUM_CHUNKS_PER_THREAD;
	int numOfBlocks = NUM_BLOCKS_PER_CHUNK;
	int blockNumer = 0;

	// Testing createFullPool()
	fullPool = createFullPool(numOfThreads);

	// Set-up of initial full pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < numOfChunks; i++) {
			Chunk *chunk = createChunk(chunk,numOfBlocks);
			for(int k = 0; k < numOfBlocks; k++) {
				Block* block = createBlock(blockNumer);
				blockNumer++;
				putInChunk(chunk, block);
			}
			putInOwnFullPool(fullPool,j,chunk);
		}
	}

	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, testMultiFullPool1, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	void *status;
	for (int t = 0; t < NUM_THREADS; t++) {
		rc = pthread_join(threads[t], &status);
	}
	printf("Test Client\n");
	pthread_exit(NULL);
}


int summain() {
	testMultiFullPool();
}
