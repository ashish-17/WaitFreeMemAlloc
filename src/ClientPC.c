#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"


#define NUM_THREADS 3   // 3
#define NUM_BLOCKS  18   // 24
#define CHUNK_SIZE 3
#define NUM_DONATION_STEPS 10


#define MAX 10000000000			/* Numbers to produce */
pthread_mutex_t the_mutex[NUM_THREADS];
pthread_cond_t condc[NUM_THREADS], condp[NUM_THREADS];
Block* buffer[NUM_THREADS];

void* producer(void *threadID) {
	int threadId = (int*) threadID;

	srand(time(NULL));

	for (int i = 1; i <= NUM_BLOCKS + 2; i++) {
		Block* block = allocate(threadID, 1);
		int con;

		//while ((con = randint(NUM_THREADS)) == 0);
		//int con = 1 + randint(NUM_THREADS - 1);
		while (true) {
			con = randint(NUM_THREADS);
			if (con != 0) {
				break;
			}
		}
		//printf("Producer: consumer chosen = %d\n", con);
		pthread_mutex_lock(&the_mutex[con]);	/* protect buffer */
		while (buffer[con] != NULL)	{	       /* If there is something in the buffer then wait */
			 //printf("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&condp[con], &the_mutex[con]);
		}
		printf("Producer passing block %d to thread %d\n", block->memBlock, con);
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


int main() {

	pthread_mutex_init(&the_mutex, NULL);
	pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
	pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */
	for (int i = 0; i < NUM_THREADS; i++) {
		buffer[i] = NULL;
	}

	//Wrapper wrapper = (Wrapper*) malloc(sizeof(Wrapper));
	createWaitFreePool(NUM_BLOCKS, NUM_THREADS, CHUNK_SIZE, NUM_DONATION_STEPS);

	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		if (t == 0)
			rc = pthread_create(&threads[t], NULL, producer, (void *)t);
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
