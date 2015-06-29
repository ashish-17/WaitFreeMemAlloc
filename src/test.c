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
#include "Block.h"
#include "StackPool.h"
#include "CircularQueue.h"
#include "commons.h"
#include <time.h>
//#include "logging.h"

/*
typedef struct {
    int year;
    int mon;
    int day;
    int hour;
    int min;
    int sec;
} SystemDate;

void get_system_date(SystemDate *mdate)
{
    time_t rawtime;
    struct tm *timeinfo;
    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);

    mdate->year = timeinfo->tm_year+1900;
    mdate->mon = timeinfo->tm_mon+1;
    mdate->day = timeinfo->tm_mday;
    mdate->hour = timeinfo->tm_hour;
    mdate->min = timeinfo->tm_min;
    mdate->sec = timeinfo->tm_sec;
}

void testStack() {
	Stack * stack = (Stack*)malloc(sizeof(Stack));
	int noOfOps = 0;

	// Testing stackCreate()
	LOG_INFO("Before stackCreate\n");
	stackCreate(stack, sizeof(int*));
	LOG_INFO("After stackCreate\n");

	// Testing stackPush()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
 *ptr = i;
		LOG_INFO("Pushing the value using stackPush i=%d\n",i);
		stackPush(stack, ptr);
	}

	// Testing stackPop()
	for (int i = 1; i <= 9; i++) {
		int* ptr = (int*)stackPop(stack);
		LOG_INFO("value popped using stackPop is %d\n", *ptr);
	}

	// Testing stackIsEmpty()
	LOG_INFO("is stack empty %d\n", stackIsEmpty(stack));

	// Checking the value of stamp
	LOG_INFO("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);

	// Testing stackPushOwner()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
 *ptr = i;
		LOG_INFO("Pushing the value using pushOwner i=%d\n",i);
		stackPushOwner(stack, ptr);
		noOfOps++;
	}

	// Checking the value of stamp
	LOG_INFO("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	LOG_INFO("Current value of the noOfOps is %d\n", noOfOps);

	// Testing stackPopOwner()
	while(!stackIsEmpty(stack)) {
		int* ptr = (int*)stackPopOwner(stack);
		noOfOps++;
		LOG_INFO("value popped using popOwner is %d\n", *ptr);
	}

	// Checking the value of stamp
	LOG_INFO("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	LOG_INFO("Current value of the noOfOps is %d\n", noOfOps);

	// Testing stackPushOther()
	for(int i = 1; i <= 10; i++) {
		int *ptr = (int*) malloc(sizeof(int));
 *ptr = i;
		bool out = stackPushOther(stack, ptr,stack->top);
		LOG_INFO("Pushing the value using pushOther i=%d succeeded with %d\n", i, out);
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
			LOG_INFO("value popped using popOwner is %d\n", *ptr);
		}
		else {
			LOG_INFO("Can't pop more from other's stack but is stack empty %d\n",stackIsEmpty(stack));
			break;
		}
	}

	// Checking the value of stamp
	LOG_INFO("Current stamp value of the top pointer is %d\n", stack->top->atomicRef->integer);
	LOG_INFO("Current value of the noOfOps is %d\n", noOfOps);
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
			LOG_INFO("the value of chunk ptr is%u\n",chunk);
			//	LOG_INFO("in test the chunk value is %d\n", chunk->value);
			putInLocalPool(localPool, j, chunk);
		}
	}

	// Testing getFromLocalPool
	for(int j = 0; j < numOfThreads; j++) {
		for (int i = 1; i <= numOfChunks; i++) {
			Chunk *chunk = getFromLocalPool(localPool, j);
			//		LOG_INFO("Chunk popped from localPool of thread %d is %d\n", j, chunk->value);
		}
	}
}

void testFreePool() {
	int numOfThreads = 5;
	int numOfChunks = 3;
	int numOfBlocks = 2;

	// Testing createFreePool()
	Pool *freePool = createFreePool(numOfThreads);
	//LOG_INFO("Inside test pool ptr is %u\n", freePool);
	//LOG_INFO("Inside test Thread ptr is %u\n", freePool->threads);

	// Testing putInFreePool
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk, numOfBlocks);
			for (int i = 0; i < numOfBlocks; i++) {
				Block *block = createBlock(i);
				//LOG_INFO("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			//LOG_INFO("the value of chunk ptr is%u\n",chunk);
			//LOG_INFO("in test the chunk value is %d\n", chunk->value);
			putInFreePool(freePool, j, chunk);

		}
	}

	// Testing getFromFreePool
	for(int j = 0; j < numOfThreads; j++) {
		for (int i = 1; i <= numOfChunks; i++) {
			Chunk *chunk = getFromFreePool(freePool, j);
			while(!isChunkEmpty(chunk)) {
				LOG_INFO("thread id %d, chunk id %d, Block value %d\n", j, i, getFromChunk(chunk)->memBlock);
			}
			//			LOG_INFO("Chunk popped from freePool of thread %d is %d\n", j, chunk->value);
		}
	}
}

void testFullPool() {
	LOG_INFO("************Testing Full Pool\n");
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
				//LOG_INFO("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			//LOG_INFO("the value of chunk ptr is%u\n",chunk);
			//LOG_INFO("in test the chunk value is %d\n", chunk->value);
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	// Testing getFromOwnFullPool
	for(int j = 0; j < numOfThreads; j++) {
		while(!isFullPoolEmpty(fullPool, j)) {
			Chunk *chunk = getFromOwnFullPool(fullPool, j);
			while(!isChunkEmpty(chunk)) {
				LOG_INFO("thread id %d, Block value %d\n", j, getFromChunk(chunk)->memBlock);
			}
			//	LOG_INFO("Chunk popped from fullPool of thread %d is %d\n", j, chunk->value);
		}
	}

	// Populating fullPools again
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			Chunk *chunk;
			chunk = createChunk(chunk,numOfBlocks);
			//LOG_INFO("the value of chunk ptr is%u\n",chunk);
			//LOG_INFO("in test the chunk value is %d\n", chunk->value);
			for (int i = 0; i < numOfBlocks; i++) {
				Block *block = createBlock(i);
				//LOG_INFO("block ptr= %u\n", block);
				putInChunk(chunk, block);
			}
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	// Testing getFromOtherFullPool- making thread 0 pop from stack 1. Should be able to pop only 3 chunks
	LOG_INFO("testing getFromOtherFullPool\n");
	while(!isFullPoolEmpty(fullPool, 1)) {
		Chunk *chunk = getFromOtherFullPool(fullPool, 1);
		if (chunk != NULL) {
			while(!isChunkEmpty(chunk)) {
				LOG_INFO("thread id %d, Block value %d\n", 1, getFromChunk(chunk)->memBlock);
			}
			//	LOG_INFO("Chunk popped from otherfullPool of thread %d is %d\n", 1, chunk->value);
		}
		else {
			LOG_INFO("Can't pop more from other's stack 1 but is stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
			break;
		}
	}
	LOG_INFO("popping the last chunk from stack 1 using popOwner\n");
	Chunk *chunk = getFromOwnFullPool(fullPool, 1);
	while(!isChunkEmpty(chunk)) {
		LOG_INFO("thread id %d, Block value %d\n", 1, getFromChunk(chunk)->memBlock);
	}
	//LOG_INFO("Chunk popped from fullPool of thread 1 is %d\n", chunk->value);
	LOG_INFO("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));

	// Testing pushOther
	LOG_INFO("Testing pushOther\n");
	chunk = createChunk(chunk, numOfBlocks);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//LOG_INFO("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}
	LOG_INFO("was Chunk push successful %d \n", putInOtherFullPool(fullPool, 1, chunk, getThread(fullPool,1)->stack->top));
	LOG_INFO("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
	LOG_INFO("trying to push again using pushOther\n");
	chunk = createChunk(chunk, numOfBlocks);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//LOG_INFO("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}
	LOG_INFO("was Chunk push successful %d \n", putInOtherFullPool(fullPool, 1, chunk, getThread(fullPool,1)->stack->top));
	LOG_INFO("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));
	LOG_INFO("now trying to push using pushOwner\n");
	LOG_INFO("was Chunk push successful %d \n", putInOwnFullPool(fullPool, 1, chunk));
	LOG_INFO("Now is the stack 1 empty %d\n",isFullPoolEmpty(fullPool,1));

}

void testPoolsTogether() {

	int numOfThreads = 5;
	int numOfChunks = 4;
	Chunk *chunk;

	Pool *fullPool = createFullPool(numOfThreads);
	Pool *localPool = createLocalPool(numOfThreads);
	Pool *freePool = createFreePool(numOfThreads);

	LOG_INFO("Populating fullPool\n");
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 0; i < numOfChunks; i++) {
			chunk = createChunk(chunk,(j * (numOfThreads-1) + i));
			putInOwnFullPool(fullPool, j, chunk);
		}
	}

	LOG_INFO("Thread 1 moves all the chunks from stack 1 to local pool\n");
	while(!isFullPoolEmpty(fullPool, 1)) {
		chunk = getFromOwnFullPool(fullPool, 1);
		//	LOG_INFO("removed chunk with value %d from stack 1\n", chunk->value);
		putInLocalPool(localPool, 1, chunk);
	}
	LOG_INFO("popping the last chunk from local pool of stack 1\n");
	//LOG_INFO("Chunk popped from local pool of stack 1 is %d\n", getFromLocalPool(localPool,1)->value);
}

void testChunk() {
	int numOfBlocks = 5;
	Chunk *chunk = createChunk(chunk, numOfBlocks);
	LOG_INFO("chunk ptr= %u\n", chunk);
	for (int i = 0; i < numOfBlocks; i++) {
		Block *block = createBlock(i);
		//LOG_INFO("block ptr= %u\n", block);
		putInChunk(chunk, block);
	}

	while(!isChunkEmpty(chunk)) {
		LOG_INFO("Block value %d\n", getFromChunk(chunk)->memBlock);
	}
}


void testCircularQueue() {
	int numOfElements = 5;
	CircularQueue *queue = (CircularQueue*) malloc(sizeof(CircularQueue));
	//LOG_INFO("queuePtr = %u\n", queue);
	circularQueueCreate(queue, sizeof(int*), numOfElements);
	//LOG_INFO("baseAddressPtr = %u\n", queue->baseAddress);

	for(int i = 0; i < numOfElements + 1; i++) {
		int* element = (int*) malloc(sizeof(int));
 *element = i;
		//LOG_INFO("elementPtr = %u\n", element);
		LOG_INFO("element %d successfully enqueued %u \n", i, circularQueueEnq(queue, element));
	}

	for (int i = 0; i < numOfElements + 1; i++) {
		int *element = circularQueueDeq(queue);
		if (element)
			LOG_INFO("element %d dequeued \n", *element);
		else
			LOG_INFO("queueIsEmpty\n");
	}
	for(int i = 0; i < numOfElements - 1; i++) {
		int* element = (int*) malloc(sizeof(int));
 *element = i;
		//LOG_INFO("elementPtr = %u\n", element);
		LOG_INFO("element %d successfully enqueued %u \n", i, circularQueueEnq(queue, element));
	}
	for (int i = 0; i < 1; i++) {
		int *element = circularQueueDeq(queue);
		if (element)
			LOG_INFO("element %d dequeued \n", *element);
		else
			LOG_INFO("queueIsEmpty\n");
	}
	for(int i = 0; i < numOfElements - 1; i++) {
		int* element = (int*) malloc(sizeof(int));
 *element = i;
		//LOG_INFO("elementPtr = %u\n", element);
		LOG_INFO("element %d successfully enqueued %u \n", i, circularQueueEnq(queue, element));
	}
	for (int i = 0; i < numOfElements ; i++) {
		int *element = circularQueueDeq(queue);
		if (element)
			LOG_INFO("element %d dequeued \n", *element);
		else
			LOG_INFO("queueIsEmpty\n");
	}
}



int main() {
	log_msg("hello world");
	log_msg("integer %d", 2);
	log_msg("int - %d, string %s", 1, "lol");

	log_msg_prolog("");
	log_msg_prolog("hello world");
	log_msg_prolog("integer %d", 2);
	log_msg_prolog("int - %d, string %s", 1, "lol");

	log_msg_epilog("");
	log_msg_epilog("hello world");
	log_msg_epilog("integer %d", 2);
	log_msg_epilog("int - %d, string %s", 1, "lol");
}
*/
int stmain() {

	LOG_INFO("hello world\n");
	//testStack();
	//testLocalPool(); // have to update as chunk definition has been changed
	//testFreePool();
	//testFullPool();

	//testPoolsTogether(); // have to update as chunk definition has been changed

	//testChunk();
	//testCircularQueue();

	LOG_INFO("Test client");
	return 0;
}

