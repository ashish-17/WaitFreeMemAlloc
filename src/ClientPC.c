#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"
#include "HazardPointer.h"


//#define NUM_THREADS 3   //3   // 3    //6
#define NUM_BLOCKS  54    //18   // 24   //36
#define CHUNK_SIZE 3
#define NUM_DONATION_STEPS 2
#define NUM_PRODUCERS 3
#define NUM_NORMAL_THREADS 0

#define NUM_THREADS (3 * NUM_PRODUCERS + NUM_NORMAL_THREADS)
#define NUM_BLOCKS_TO_BE_PASSED (100)
#define MAX 10000000000			/* Numbers to produce */

pthread_mutex_t the_mutex[NUM_THREADS];
pthread_cond_t condc[NUM_THREADS], condp[NUM_THREADS];
Block* buffer[NUM_THREADS];

HPStructure *globalHPStructure = NULL;

/*void* producer(void *threadID) {
	LOG_PROLOG();
	int threadId = (int*) threadID;
	LOG_INFO("producer: thread has id %d", threadId);
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
		//LOG_INFO("Producer: consumer chosen = %d\n", con);
		pthread_mutex_lock(&the_mutex[con]);	// protect buffer
		while (buffer[con] != NULL)	{	       // If there is something in the buffer then wait
			//LOG_INFO("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&condp[con], &the_mutex[con]);
		}
		LOG_INFO("\nProducer %d passing block %d to thread %d\n", threadId, block->memBlock, con);
		buffer[con] = block;
		pthread_cond_signal(&condc[con]);	// wake up consumer
		pthread_mutex_unlock(&the_mutex[con]);	// release the buffer
	}
	LOG_INFO("FINISHED");
	LOG_EPILOG();
	pthread_exit(0);
}

void* consumer(void *threadID) {
	LOG_PROLOG();
	int threadId = (int*) threadID;
	LOG_INFO("consumer: thread has id %d", threadId);
	Block *block;

	while(true) {
		pthread_mutex_lock(&the_mutex[threadId]);	// protect buffer
		while (buffer[threadId] == NULL) {	// If there is nothing in the buffer then wait
			//LOG_INFO("Consumer: %d waiting for producer \n",threadId);
			pthread_cond_wait(&condc[threadId], &the_mutex[threadId]);
		}
		//LOG_INFO("Consumer: %d done waiting \n",threadId);
		block = buffer[threadId];
		//LOG_INFO("Consumer: %d read the block \n",threadId);
		buffer[threadId] = NULL;
		pthread_cond_signal(&condp[threadId]);	// wake up consumer
		pthread_mutex_unlock(&the_mutex[threadId]);	// release the buffer
		//LOG_INFO("Consumer %d consumed the block %d\n", threadId, block->memBlock);
		freeMem(threadId, block);
	}
	pthread_exit(0);
	LOG_EPILOG();
}*/


void* producer(void *threadID) {
	LOG_PROLOG();
	int threadId = (int*) threadID;
	LOG_INFO("producer: thread has id %d", threadId);
	//srand(time(NULL));
	int con;
	for (int i = 1; i <= NUM_BLOCKS_TO_BE_PASSED; i++) {
		Block* block = allocate(threadID, 1);
		int con1 = 2*threadId + NUM_PRODUCERS + NUM_NORMAL_THREADS;
		int con2 = con1 + 1;

		if (i % 2 == 0) {
			con = con1;
		}
		else {
			con = con2;
		}
		//LOG_INFO("Producer: consumer chosen = %d\n", con);
		pthread_mutex_lock(&the_mutex[con]);	// protect buffer
		while (buffer[con] != NULL)	{	       // If there is something in the buffer then wait
			//LOG_INFO("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&condp[con], &the_mutex[con]);
		}
		LOG_INFO("Producer %d passing block %d to thread %d", threadId, block->memBlock, con);
		buffer[con] = block;
		pthread_cond_signal(&condc[con]);	// wake up consumer
		pthread_mutex_unlock(&the_mutex[con]);	// release the buffer
	}
	LOG_INFO("PRODUCER FINISHED");
	LOG_EPILOG();
	pthread_exit(0);
}

void* consumer(void *threadID) {
	LOG_PROLOG();
	int threadId = (int*) threadID;
	LOG_INFO("consumer: thread has id %d", threadId);
	Block *block;

	for(int i = 1; i <= NUM_BLOCKS_TO_BE_PASSED/2; i++) {
		pthread_mutex_lock(&the_mutex[threadId]);	// protect buffer
		while (buffer[threadId] == NULL) {	// If there is nothing in the buffer then wait
			//LOG_INFO("Consumer: %d waiting for producer \n",threadId);
			pthread_cond_wait(&condc[threadId], &the_mutex[threadId]);
		}
		//LOG_INFO("Consumer: %d done waiting \n",threadId);
		block = buffer[threadId];
		//LOG_INFO("Consumer: %d read the block \n",threadId);
		buffer[threadId] = NULL;
		pthread_cond_signal(&condp[threadId]);	// wake up consumer
		pthread_mutex_unlock(&the_mutex[threadId]);	// release the buffer
		//LOG_INFO("Consumer %d consumed the block %d\n", threadId, block->memBlock);
		freeMem(threadId, block);
		LOG_INFO("Consumer consumed the block %d", block->memBlock);
	}
	LOG_INFO("CONSUMER FINISHED");

	LOG_EPILOG();
	pthread_exit(0);
}


void* normalExec(void *threadID) {
	LOG_PROLOG();
	int threadId = (int*) threadID;
	LOG_INFO("normalExec: thread has id %d", threadId);
	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(50);
	LOG_INFO("In thread %d, the totalNumOfOps %d", (int)threadId, totalNumOfOps);
	Stack* stack = (Stack*) my_malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(11);

		//LOG_INFO("In thread %d, the flag %d\n", (int)threadId, flag);
		if (flag <= 7) {
			numOfAllocBlocks++;
			Block* block = allocate((int)threadId, 0);
			LOG_INFO("thread %d allocated the block %d with block number %d",(int)threadId, block->memBlock, block->threadId);
			stackPush(stack,block);
		}
		else {
			if (numOfAllocBlocks == 0) {
				//LOG_INFO("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				continue;
			}
			else {
				LOG_INFO("tester: threadId = %d: noOfAllocBlocks %d",(int) threadId, numOfAllocBlocks);
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
	LOG_INFO("FINISHED");
	LOG_EPILOG();
	pthread_exit(NULL);
}

int main() {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

	pthread_mutex_init(&the_mutex, NULL);
	pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
	pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */
	for (int i = 0; i < NUM_THREADS; i++) {
		buffer[i] = NULL;
	}

	//Wrapper wrapper = (Wrapper*) malloc(sizeof(Wrapper));
	//createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);
	//LOG_INFO("here...\n");
	globalHPStructure = (HPStructure*)my_malloc(sizeof(HPStructure));
	//LOG_INFO("initialised globalSruct\n");
	hpStructureCreate(globalHPStructure, NUM_THREADS, 5);
	//LOG_INFO("created globalSruct\n");
	createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);
	//LOG_INFO("created wait free pools\n");

	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		LOG_INFO("In main: creating thread %d", t);
		if (t < NUM_PRODUCERS)
			rc = pthread_create(&threads[t], NULL, producer, (void *)t);
		else if ((t >= NUM_PRODUCERS) && (t < NUM_PRODUCERS + NUM_NORMAL_THREADS))
			rc = pthread_create(&threads[t], NULL, normalExec, (void *)t);
		else
			rc = pthread_create(&threads[t], NULL, consumer, (void *)t);
		if (rc){
			LOG_INFO("ERROR; return code from pthread_create() is %d", rc);
			exit(-1);
		}
	}

	// waiting for threads to terminate
	void *status;
	for (int t = 0; t < NUM_THREADS; t++) {
		rc = pthread_join(threads[t], &status);
	}
	LOG_INFO("Test Client");
	LOG_CLOSE();
	pthread_exit(NULL);

}
