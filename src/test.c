/*
 * test.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */

#include "stdio.h"
#include "Stack.h"
#include "Chunk.h"
#include "LocalPool.h"
#include "FreePool.h"
#include "FullPool.h"
#include "pool.h"
#include "Block.h"

/*void testStack() {
	Stack * stack = (Stack*)malloc(sizeof(Stack));
	int noOfOps = 0;

	// Testing stackCreate()
	printf("Before stackCreate\n");
	stackCreate(stack, sizeof(int*));
	printf("After stackCreate\n");

	// Testing stackPush()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
		*ptr = i;
		printf("Pushing the value using stackPush i=%d\n",i);
		stackPush(stack, ptr);
	}

	// Testing stackPop()
	for (int i = 1; i <= 9; i++) {
		int* ptr = (int*)stackPop(stack);
		printf("value popped using stackPop is %d\n", *ptr);
	}

	// Testing stackIsEmpty()
	printf("is stack empty %d\n", stackIsEmpty(stack));

	// Checking the value of stamp
	printf("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);

	// Testing stackPushOwner()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
		*ptr = i;
		printf("Pushing the value using pushOwner i=%d\n",i);
		stackPushOwner(stack, ptr);
		noOfOps++;
	}

	// Checking the value of stamp
	printf("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	printf("Current value of the noOfOps is %d\n", noOfOps);

	// Testing stackPopOwner()
	while(!stackIsEmpty(stack)) {
		int* ptr = (int*)stackPopOwner(stack);
		noOfOps++;
		printf("value popped using popOwner is %d\n", *ptr);
	}

	// Checking the value of stamp
	printf("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	printf("Current value of the noOfOps is %d\n", noOfOps);

	// Testing stackPushOther()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
		*ptr = i;
		bool out = stackPushOther(stack, ptr,stack->top);
		printf("Pushing the value using pushOther i=%d succeeded with %d\n", i, out);
		if (out)
			noOfOps++;
	}

	//Testing stackPopOther()
	// Adding one extra node to test stackPopOther
	int *ptr1 = (int*) malloc(sizeof(int));
	*ptr1 = 2;
	stackPush(stack,ptr1);
	while(!stackIsEmpty(stack)) {
		int* ptr = (int*)stackPopOther(stack);
		if	(ptr != NULL){
			noOfOps++;
			printf("value popped using popOwner is %d\n", *ptr);
		}
		else {
			printf("Can't pop more from other's stack but is stack empty %d\n",stackIsEmpty(stack));
			break;
		}
	}

	// Checking the value of stamp
	printf("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	printf("Current value of the noOfOps is %d\n", noOfOps);
}

void testLocalPool() {
	int numOfThreads = 4;
	int numOfChunks = 3;

	// Testing createLocalPool()
	Pool *localPool = createLocalPool(numOfThreads);

	// Testing putInLocalPool
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk,(j * (numOfThreads-1) + i));
			printf("the value of chunk ptr is%u\n",chunk);
			//	printf("in test the chunk value is %d\n", chunk->value);
			putInLocalPool(localPool, j, chunk);
		}
	}

	// Testing getFromLocalPool
	for(int j = 0; j < numOfThreads; j++) {
		for (int i = 1; i <= numOfChunks; i++) {
			Chunk *chunk = getFromLocalPool(localPool, j);
			//		printf("Chunk popped from localPool of thread %d is %d\n", j, chunk->value);
		}
	}
}

void testFreePool() {
	int numOfThreads = 5;
	int numOfChunks = 3;
	int numOfBlocks = 2;

	// Testing createFreePool()
	Pool *freePool = createFreePool(numOfThreads);
	//printf("Inside test pool ptr is %u\n", freePool);
	//printf("Inside test Thread ptr is %u\n", freePool->threads);

	// Testing putInFreePool
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk, numOfBlocks);
			for (int i = 0; i < numOfBlocks; i++) {
				Block *block = createBlock(i);
				//printf("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			//printf("the value of chunk ptr is%u\n",chunk);
			//printf("in test the chunk value is %d\n", chunk->value);
			putInFreePool(freePool, j, chunk);

		}
	}

	// Testing getFromFreePool
	for(int j = 0; j < numOfThreads; j++) {
		for (int i = 1; i <= numOfChunks; i++) {
			Chunk *chunk = getFromFreePool(freePool, j);
			while(!isChunkEmpty(chunk)) {
				printf("thread id %d, chunk id %d, Block value %d\n", j, i, getFromChunk(chunk)->memBlock);
			}
			//			printf("Chunk popped from freePool of thread %d is %d\n", j, chunk->value);
		}
	}
}

void testFullPool() {
	printf("************Testing Full Pool\n");
	int numOfThreads = 3;
	int numOfChunks = 2;
	int numOfBlocks = 3;
	// Testing createFullPool()
	Pool *fullPool = createFullPool(numOfThreads);

	// Testing putInOwnFullPool
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk, numOfBlocks);
			for (int i = 0; i < numOfBlocks; i++) {
				Block *block = createBlock(i);
				//printf("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			//printf("the value of chunk ptr is%u\n",chunk);
			//printf("in test the chunk value is %d\n", chunk->value);
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	// Testing getFromOwnFullPool
	for(int j = 0; j < numOfThreads; j++) {
		while(!isFullPoolEmpty(fullPool, j)) {
			Chunk *chunk = getFromOwnFullPool(fullPool, j);
			while(!isChunkEmpty(chunk)) {
				printf("thread id %d, Block value %d\n", j, getFromChunk(chunk)->memBlock);
			}
			//	printf("Chunk popped from fullPool of thread %d is %d\n", j, chunk->value);
		}
	}

	// Populating fullPools again
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk,numOfBlocks);
			//printf("the value of chunk ptr is%u\n",chunk);
			//printf("in test the chunk value is %d\n", chunk->value);
			for (int i = 0; i < numOfBlocks; i++) {
				Block *block = createBlock(i);
				//printf("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	// Testing getFromOtherFullPool- making thread 0 pop from stack 1. Should be able to pop only 3 chunks
	printf("testing getFromOtherFullPool\n");
	while(!isFullPoolEmpty(fullPool, 1)) {
		Chunk *chunk = getFromOtherFullPool(fullPool, 1);
		if (chunk != NULL) {
			while(!isChunkEmpty(chunk)) {
				printf("thread id %d, Block value %d\n", 1, getFromChunk(chunk)->memBlock);
			}
			//	printf("Chunk popped from otherfullPool of thread %d is %d\n", 1, chunk->value);
		}
		else {
			printf("Can't pop more from other's stack 1 but is stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
			break;
		}
	}
	printf("popping the last chunk from stack 1 using popOwner\n");
	Chunk *chunk = getFromOwnFullPool(fullPool, 1);
	while(!isChunkEmpty(chunk)) {
		printf("thread id %d, Block value %d\n", 1, getFromChunk(chunk)->memBlock);
	}
	//printf("Chunk popped from fullPool of thread 1 is %d\n", chunk->value);
	printf("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));

	// Testing pushOther
	printf("Testing pushOther\n");
	chunk = createChunk(chunk, numOfBlocks);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//printf("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}
	printf("was Chunk push successful %d \n", putInOtherFullPool(fullPool, 1, chunk, getThread(fullPool,1)->stack->top));
	printf("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
	printf("trying to push again using pushOther\n");
	chunk = createChunk(chunk, numOfBlocks);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//printf("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}
	printf("was Chunk push successful %d \n", putInOtherFullPool(fullPool, 1, chunk, getThread(fullPool,1)->stack->top));
	printf("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
	printf("now trying to push using pushOwner\n");
	printf("was Chunk push successful %d \n", putInOwnFullPool(fullPool, 1, chunk));
	printf("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));

}

void testPoolsTogether() {

	int numOfThreads = 5;
	int numOfChunks = 4;
	Chunk *chunk;

	Pool *fullPool = createFullPool(numOfThreads);
	Pool *localPool = createLocalPool(numOfThreads);
	Pool *freePool = createFreePool(numOfThreads);

	printf("Populating fullPool\n");
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			chunk = createChunk(chunk,(j * (numOfThreads-1) + i));
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	printf("Thread 1 moves all the chunks from stack 1 to local pool\n");
	while(!isFullPoolEmpty(fullPool, 1)) {
		chunk = getFromOwnFullPool(fullPool, 1);
		//	printf("removed chunk with value %d from stack 1\n", chunk->value);
		putInLocalPool(localPool, 1, chunk);
	}
	printf("popping the last chunk from local pool of stack 1\n");
	//printf("Chunk popped from local pool of stack 1 is %d\n", getFromLocalPool(localPool,1)->value);
}

void testChunk() {
	int numOfBlocks = 5;
	Chunk *chunk = createChunk(chunk, numOfBlocks);
	printf("chunk ptr= %u\n", chunk);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//printf("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}

	while(!isChunkEmpty(chunk)) {
		printf("Block value %d\n", getFromChunk(chunk)->memBlock);
	}
}
*/
int somemain() {

	//testStack();
	//testLocalPool(); // have to update as chunk definition has been changed
	//testFreePool();
	//testFullPool();

	//testPoolsTogether(); // have to update as chunk definition has been changed

	//testChunk();


	printf("Test client");
	return 0;
}

