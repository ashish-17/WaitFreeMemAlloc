#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"
#include "HazardPointer.h"


#define NUM_THREADS 3   //3   // 3    //6
#define NUM_BLOCKS  18    //18   // 24   //36
#define CHUNK_SIZE 2
#define NUM_DONATION_STEPS 2
#define NUM_PRODUCERS 2
#define NUM_NORMAL_THREADS 0


#define MAX 10000000000			/* Numbers to produce */
pthread_mutex_t the_mutex[NUM_THREADS];
pthread_cond_t condc[NUM_THREADS], condp[NUM_THREADS];
Block* buffer[NUM_THREADS];

HPStructure *globalHPStructure = NULL;

void* producer(void *threadID) {
	int threadId = (int*) threadID;

	srand(time(NULL));

	for (int i = 1; i <= NUM_BLOCKS + 50; i++) {
		Block* block = allocate(threadID, 1);
		int con;

		//while ((con = randint(NUM_THREADS)) == 0);
		//int con = 1 + randint(NUM_THREADS - 1);
		while (true) {
			con = randint(NUM_THREADS);
			if (con >= NUM_PRODUCERS + NUM_NORMAL_THREADS){
				break;
			}
		}
		//printf("Producer: consumer chosen = %d\n", con);
		pthread_mutex_lock(&the_mutex[con]);	/* protect buffer */
		while (buffer[con] != NULL)	{	       /* If there is something in the buffer then wait */
			//printf("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&condp[con], &the_mutex[con]);
		}
		printf("Producer %d passing block %d to thread %d\n", threadId, block->memBlock, con);
		buffer[con] = block;
		pthread_cond_signal(&condc[con]);	/* wake up consumer */
		pthread_mutex_unlock(&the_mutex[con]);	/* release the buffer */
	}
	printf("thread %d is FINSISHED\n",(int)threadId);
	pthread_exit(0);
}

void* consumer(void *threadID) {
	int threadId = (int*) threadID;
	Block *block;

	while(true) {
		pthread_mutex_lock(&the_mutex[threadId]);	/* protect buffer */
		while (buffer[threadId] == NULL) {	/* If there is nothing in the buffer then wait */
			//printf("Consumer: %d waiting for producer \n",threadId);
			pthread_cond_wait(&condc[threadId], &the_mutex[threadId]);
		}
		//printf("Consumer: %d done waiting \n",threadId);
		block = buffer[threadId];
		//printf("Consumer: %d read the block \n",threadId);
		buffer[threadId] = NULL;
		pthread_cond_signal(&condp[threadId]);	/* wake up consumer */
		pthread_mutex_unlock(&the_mutex[threadId]);	/* release the buffer */
		//printf("Consumer %d consumed the block %d\n", threadId, block->memBlock);
		freeMem(threadId, block);
	}
	pthread_exit(0);
}

void* normalExec(void *threadID) {
	int threadId = (int*) threadID;
	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(50);
	printf("In thread %d, the totalNumOfOps %d\n", (int)threadId, totalNumOfOps);
	Stack* stack = (Stack*) my_malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(11);

		//printf("In thread %d, the flag %d\n", (int)threadId, flag);
		if (flag <= 7) {
			numOfAllocBlocks++;
			Block* block = allocate((int)threadId, 0);
			printf("thread %d allocated the block %d with block number %d\n",(int)threadId, block->memBlock, block->threadId);
			stackPush(stack,block);
		}
		else {
			if (numOfAllocBlocks == 0) {
				//printf("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				continue;
			}
			else {
				printf("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				numOfAllocBlocks--;
				Block *block = stackPop(stack);
				//printf("thread %d trying to free the block %d\n",(int)threadId, block->memBlock);
				freeMem((int)threadId, block);
				printf("thread %d freed the block %d\n",(int)threadId, block->memBlock);
			}
		}
		totalNumOfOps--;
		//printf("thread %d totalNumOfOps remaining %d\n",(int)threadId, totalNumOfOps);
	}
	printf("thread %d is FINSISHED\n",(int)threadId);
	pthread_exit(NULL);
}

int dsmain() {
	printf("startung\n");
	pthread_mutex_init(&the_mutex, NULL);
	pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
	pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */
	for (int i = 0; i < NUM_THREADS; i++) {
		buffer[i] = NULL;
	}

	//Wrapper wrapper = (Wrapper*) malloc(sizeof(Wrapper));
	//createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);
	printf("here...\n");
	globalHPStructure = (HPStructure*)my_malloc(sizeof(HPStructure));
	printf("initialised globalSruct\n");
	hpStructureCreate(globalHPStructure, NUM_THREADS, 5);
	printf("created globalSruct\n");
	createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);
	printf("created wait free pools\n");

	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		if (t < NUM_PRODUCERS)
			rc = pthread_create(&threads[t], NULL, producer, (void *)t);
		else if ((t >= NUM_PRODUCERS) && (t < NUM_PRODUCERS + NUM_NORMAL_THREADS))
			rc = pthread_create(&threads[t], NULL, normalExec, (void *)t);
		else
			rc = pthread_create(&threads[t], NULL, consumer, (void *)t);
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
