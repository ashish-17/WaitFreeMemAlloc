/*
 * test.c
 *
 *  Created on: May 9, 2015
 *      Author: Archita
 */

#include "stdio.h"
#include "Stack.h"
#include "Chunk.h"
#include "localPool.h"

void testStack() {
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
	int numOfThreads = 5;

	// Testing createLocalPool()
	Pool *localPool = createLocalPool(numOfThreads);
	for(int j = 0; j < numOfThreads ; j++) {
		for (int i = 1; i <= 3; i++) {
			Chunk *chunk;
			createChunk(chunk, (j * numOfThreads + i));
			putInLocalPool(localPool, j, chunk);
		}
	}
}

int main() {

	//	testStack();
	testLocalPool();

	printf("Test client");
	return 0;
}

