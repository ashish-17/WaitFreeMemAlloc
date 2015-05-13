#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"


#define NUM_THREADS 5
#define NUM_BLOCKS 100
#define CHUNK_SIZE 5
#define NUM_DONATION_STEPS 10

/*typedef struct {
	Memory *memory;
	int threadId;
} Wrapper;
 */

// The actual tester
void* tester(void *threadId) {

	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(81);

	Stack* stack = (Stack*) malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(2);
		if (flag == 0) {
			numOfAllocBlocks++;
			Block* block = allocate((int)threadId);
			printf("thread %d allocated the block %d\n",(int)threadId, block->memBlock);
			stackPush(stack,block);
		}
		else {
			if (numOfAllocBlocks == 0) {
				continue;
			}
			else {
				numOfAllocBlocks--;
				freeMem((int)threadId,(Block*)stackPop(stack));
			}
		}
		totalNumOfOps--;
	}
	pthread_exit(NULL);
}

int somemain1() {

	//Wrapper wrapper = (Wrapper*) malloc(sizeof(Wrapper));
	createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);

	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, tester, (void *)t);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	// waiting for threads to terminate
	void *status;
	for (int t = 0; t < NUM_THREADS; t++) {
		rc = pthread_join(threads[t], &status);
	}
	printf("Test Client\n");
	pthread_exit(NULL);
}
