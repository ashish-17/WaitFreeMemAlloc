#include "../WaitFreePool.h"
#include "RandomGenerator.h"
#include "../commons.h"
#include <pthread.h>
# include "../Block.h"
#include "../Stack.h"
//#include "HazardPointer.h"
#include "CodeCorrectness.h"

typedef void (*ThreadFunc)(void*);

typedef struct _TestConfig {
	int numThreads;
	int numChunksPerThread;
	int chunkSize;
	int numDonationSteps;
	int numProducers;
	int numNormalThreads;
	int numBlocksToBePassed;
	int numBlocks;
	int numProducerGroups;  //config2
	int numProducerPerGroup; //config2

	ThreadFunc producer;
	ThreadFunc counsumer;
	ThreadFunc normalExec;
} TestConfig;

typedef struct _TestThreadData {
	pthread_mutex_t the_mutex;
	pthread_cond_t condc;
	pthread_cond_t condp;
	Block* buffer;
} TestThreadData;

typedef struct _ThreadStructure {
	int threadId;
	TestConfig *config;
	TestThreadData *threadData;
} ThreadStructure;

//HPStructure *globalHPStructure = NULL;

void* normalExec(void *data) {
	LOG_PROLOG();

	ThreadStructure* threadStructure = (ThreadStructure*)data;
	int threadId = threadStructure->threadId;
	TestConfig *cfg = threadStructure->config;
	TestThreadData *threadData = threadStructure->threadData;

	LOG_INFO("normalExec: thread has id %d", threadId);

	int numOfAllocBlocks = 0;
	int flag = 0; // 0 -> allocate 1 -> free

	srand(time(NULL));
	int totalNumOfOps = randint(cfg->numBlocks);
	LOG_INFO("TotalNumOfOps %d", totalNumOfOps);

	Stack* stack = (Stack*) my_malloc(sizeof(Stack));
	stackCreate(stack, sizeof(Block));

	while(totalNumOfOps > 0) {
		flag = randint(11);

		//LOG_INFO("In thread %d, the flag %d\n", (int)threadId, flag);
		if (flag <= 5) {
			numOfAllocBlocks++;
			Block* block = allocate(threadId, 0);
			bool flag = setFlagForAllocatedBlock(block->memBlock);
			if (!flag) {
				LOG_ERROR("Block %d was already allocated to some other thread", block->memBlock);
				break;
			}
			LOG_INFO("Allocated the block %d",block->memBlock);
			LOG_INFO("noOfAllocBlocks %d", numOfAllocBlocks);
			stackPush(stack,block);
		}
		else {
			if (numOfAllocBlocks == 0) {
				//LOG_INFO("tester: threadId = %d: noOfAllocBlocks %d\n",(int) threadId, numOfAllocBlocks);
				continue;
			}
			else {
				numOfAllocBlocks--;
				Block *block = stackPop(stack);
				//LOG_INFO("thread %d trying to free the block %d\n",(int)threadId, block->memBlock);
				freeMem(threadId, block);
				bool flag = clearFlagForAllocatedBlock(block->memBlock);
				if (!flag) {
					LOG_ERROR("Block %d was already free. Tried to free again", block->memBlock);
					break;
				}
				LOG_INFO("Freed the block %d\n",block->memBlock);
				LOG_INFO("noOfAllocBlocks %d", numOfAllocBlocks);
			}
		}
		totalNumOfOps--;
		//LOG_INFO("thread %d totalNumOfOps remaining %d\n",(int)threadId, totalNumOfOps);
	}

	LOG_INFO("Normal execution finished. Going to free allocated blocks");
	while (numOfAllocBlocks > 0) {
		numOfAllocBlocks--;
		Block *block = stackPop(stack);
		freeMem(threadId, block);
		LOG_INFO("Freed the block %d\n", block->memBlock);
	}
	LOG_INFO("FINISHED");
	LOG_EPILOG();
	pthread_exit(NULL);
}


// In this pair of Pd-Cr, a producer passes to two consumers alternatively.
// Therefore, number of threads become 3 times the producers.
void* producer1(void *data) {
	LOG_PROLOG();

	ThreadStructure* threadStructure = (ThreadStructure*)data;
	int threadId = threadStructure->threadId;
	TestConfig *cfg = threadStructure->config;
	TestThreadData *threadData = threadStructure->threadData;

	LOG_INFO("producer: thread has id %d", threadId);

	int con;
	for (int i = 1; i <= cfg->numBlocksToBePassed; i++) {
		Block* block = allocate(threadId, 1);
		bool flag = setFlagForAllocatedBlock(block->memBlock);
		if (!flag) {
			LOG_ERROR("Block %d was already allocated to some other thread", block->memBlock);
			break;
		}
		int con1 = 2*threadId + cfg->numProducers + cfg->numNormalThreads;
		int con2 = con1 + 1;

		if (i % 2 == 0) {
			con = con1;
		}
		else {
			con = con2;
		}
		//LOG_INFO("Producer: consumer chosen = %d\n", con);
		pthread_mutex_lock(&threadData[con].the_mutex);	// protect buffer
		while (threadData[con].buffer != NULL)	{	       // If there is something in the buffer then wait
			//LOG_INFO("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&threadData[con].condp, &threadData[con].the_mutex);
		}
		LOG_INFO("Producer %d passing block %d to thread %d", threadId, block->memBlock, con);
		threadData[con].buffer = block;
		pthread_cond_signal(&threadData[con].condc);	// wake up consumer
		pthread_mutex_unlock(&threadData[con].the_mutex);	// release the buffer
	}
	LOG_INFO("PRODUCER FINISHED");
	LOG_EPILOG();
	pthread_exit(0);
}

void* consumer1(void *data) {
	LOG_PROLOG();
	ThreadStructure* threadStructure = (ThreadStructure*)data;
	int threadId = threadStructure->threadId;
	TestConfig *cfg = threadStructure->config;
	TestThreadData *threadData = threadStructure->threadData;
	LOG_INFO("consumer: thread has id %d", threadId);
	Block *block;

	for(int i = 1; i <= cfg->numBlocksToBePassed/2; i++) {
		pthread_mutex_lock(&threadData[threadId].the_mutex);	// protect buffer
		while (threadData[threadId].buffer == NULL) {	// If there is nothing in the buffer then wait
			//LOG_INFO("Consumer: %d waiting for producer \n",threadId);
			pthread_cond_wait(&threadData[threadId].condc, &threadData[threadId].the_mutex);
		}
		//LOG_INFO("Consumer: %d done waiting \n",threadId);
		block = threadData[threadId].buffer;
		//LOG_INFO("Consumer: %d read the block \n",threadId);
		threadData[threadId].buffer = NULL;
		pthread_cond_signal(&threadData[threadId].condp);	// wake up consumer
		pthread_mutex_unlock(&threadData[threadId].the_mutex);	// release the buffer
		//LOG_INFO("Consumer %d consumed the block %d\n", threadId, block->memBlock);
		freeMem(threadId, block);
		LOG_INFO("Consumer consumed the block %d", block->memBlock);
		bool flag = clearFlagForAllocatedBlock(block->memBlock);
		if (!flag) {
			LOG_ERROR("Block %d was already free. Tried to free again", block->memBlock);
			break;
		}
	}
	LOG_INFO("CONSUMER FINISHED");

	LOG_EPILOG();
	pthread_exit(0);
}



// In this Pd-Cr, more than one producer passes blocks to single consumer
void producer2(void *data) {
	LOG_PROLOG();

	ThreadStructure* threadStructure = (ThreadStructure*)data;
	int threadId = threadStructure->threadId;
	TestConfig *cfg = threadStructure->config;
	TestThreadData *threadData = threadStructure->threadData;

	LOG_INFO("producer: thread has id %d", threadId);

	int group_no = (threadId) / cfg->numProducerPerGroup;
	//LOG_INFO("Producer = %d, group chosen = %d", threadID, group_no);
	int con = cfg->numProducers + cfg->numNormalThreads + group_no;

	//LOG_INFO("Producer = %d, consumer chosen = %d", threadID, con);

	for (int i = 1; i <= cfg->numBlocksToBePassed; i++) {

		Block* block = allocate(threadId, 1);
		bool flag = setFlagForAllocatedBlock(block->memBlock);
		if (!flag) {
			LOG_ERROR("Block %d was already allocated to some other thread", block->memBlock);
			break;
		}

		pthread_mutex_lock(&threadData[con].the_mutex);	// protect buffer
		while (threadData[con].buffer != NULL)	{	       // If there is something in the buffer then wait
			//LOG_INFO("Producer: %d waiting for consumer to consume \n",con);
			pthread_cond_wait(&threadData[con].condp, &threadData[con].the_mutex);
		}
		LOG_INFO("Producer %d passing block %d to thread %d", threadId, block->memBlock, con);
		threadData[con].buffer = block;
		pthread_cond_signal(&threadData[con].condc);	// wake up consumer
		pthread_mutex_unlock(&threadData[con].the_mutex);	// release the buffer
	}
	LOG_INFO("PRODUCER FINISHED");
	LOG_EPILOG();
	pthread_exit(0);
}

void* consumer2(void *data) {
	LOG_PROLOG();

	ThreadStructure* threadStructure = (ThreadStructure*)data;
	int threadId = threadStructure->threadId;
	TestConfig *cfg = threadStructure->config;
	TestThreadData *threadData = threadStructure->threadData;

	LOG_INFO("consumer: thread has id %d", threadId);
	Block *block;

	LOG_INFO("index value %d",cfg->numBlocksToBePassed * cfg->numProducerPerGroup);
	for(int i = 1; i <= cfg->numBlocksToBePassed * cfg->numProducerPerGroup; i++) {

		pthread_mutex_lock(&threadData[threadId].the_mutex);	// protect buffer
		while (threadData[threadId].buffer == NULL) {	// If there is nothing in the buffer then wait
			//LOG_INFO("Consumer: %d waiting for producer \n",threadId);
			pthread_cond_wait(&threadData[threadId].condc, &threadData[threadId].the_mutex);
		}
		//LOG_INFO("Consumer: %d done waiting \n",threadId);
		block = threadData[threadId].buffer;
		//LOG_INFO("Consumer: %d read the block \n",threadId);
		threadData[threadId].buffer = NULL;
		pthread_cond_signal(&threadData[threadId].condp);	// wake up consumer
		pthread_mutex_unlock(&threadData[threadId].the_mutex);	// release the buffer
		//LOG_INFO("Consumer %d consumed the block %d\n", threadId, block->memBlock);
		freeMem(threadId, block);
		LOG_INFO("Consumer consumed the block %d", block->memBlock);
		bool flag = clearFlagForAllocatedBlock(block->memBlock);
		if (!flag) {
			LOG_ERROR("Block %d was already free. Tried to free again", block->memBlock);
			break;
		}
	}
	LOG_INFO("CONSUMER FINISHED");

	LOG_EPILOG();
	pthread_exit(0);
}


void tester(TestConfig cfg) {

	LOG_PROLOG();

	TestThreadData *threadData = (TestThreadData*)my_malloc(cfg.numThreads * sizeof(TestThreadData));
	for (int i = 0; i < cfg.numThreads; ++i) {
		if (threadData + i) {
			pthread_mutex_init(&((threadData + i)->the_mutex), NULL);
			pthread_cond_init(&((threadData + i)->condc), NULL);
			pthread_cond_init(&((threadData + i)->condp), NULL);
			(threadData + i)->buffer = NULL;
		}
	}

	ThreadStructure *threadStructure = (ThreadStructure*)my_malloc(cfg.numThreads * sizeof(ThreadStructure));

	LOG_INFO("created all the arrays");

	createWaitFreePool(cfg.numBlocks, cfg.numThreads, cfg.chunkSize, cfg.numDonationSteps);
	hashTableCreate(cfg.numBlocks);


	int rc;
	pthread_t threads[cfg.numThreads];
	for (int t = 0; t < cfg.numThreads; t++) {
		threadStructure[t].config = &cfg; // (*(threadStructure + t)).config)
		threadStructure[t].threadId = t;
		threadStructure[t].threadData = threadData;
		LOG_INFO("In main: creating thread %d", t);
		if (t < cfg.numProducers)
			rc = pthread_create(&threads[t], NULL, cfg.producer, (threadStructure + t));
		else if ((t >= cfg.numProducers) && (t < cfg.numProducers + cfg.numNormalThreads))
			rc = pthread_create(&threads[t], NULL, cfg.normalExec, (threadStructure + t));
		else
			rc = pthread_create(&threads[t], NULL, cfg.counsumer, (threadStructure + t));
		if (rc){
			LOG_INFO("ERROR; return code from pthread_create() is %d", rc);
			exit(-1);
		}
	}

	// waiting for threads to terminate
	void *status;
	for (int t = 0; t < cfg.numThreads; t++) {
		rc = pthread_join(threads[t], &status);
	}

	for (int i = 0; i < cfg.numThreads; ++i) {
		if (threadData + i) {
			pthread_mutex_destroy(&((threadData + i)->the_mutex));
			pthread_cond_destroy(&((threadData + i)->condc));
			pthread_cond_destroy(&((threadData + i)->condp));
			(threadData + i)->buffer = NULL;
		}
	}

	my_free(threadData);
	threadData = NULL;
	hashTableFree(cfg.numBlocks);
	destroyWaitFreePool();

	LOG_EPILOG();
}

int TestWFPmain() {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

	// config1 has single producer passing to two deterministic consumers
	// Following are the various configurations for this scenario
	TestConfig config1;

	config1.numChunksPerThread = 2;
	config1.chunkSize = 3;
	config1.numDonationSteps = 2;
	config1.numProducerGroups = -1;
	config1.numProducerPerGroup = -1;
	config1.numProducers = 4;
	config1.numNormalThreads = 0;
	config1.numBlocksToBePassed = 20;
	config1.numThreads = (3 * config1.numProducers + config1.numNormalThreads);
	config1.numBlocks = (config1.numThreads * config1.numChunksPerThread * config1.chunkSize);
	config1.producer = producer1;
	config1.counsumer = consumer1;
	config1.normalExec = normalExec;

	tester(config1);
	LOG_INFO("Config 1.1 successful");

	config1.numChunksPerThread = 2;
	config1.chunkSize = 3;
	config1.numDonationSteps = 2;
	config1.numProducerGroups = -1;
	config1.numProducerPerGroup = -1;
	config1.numProducers = 4;
	config1.numNormalThreads = 0;
	config1.numBlocksToBePassed = 100;
	config1.numThreads = (3 * config1.numProducers + config1.numNormalThreads);
	config1.numBlocks = (config1.numThreads * config1.numChunksPerThread * config1.chunkSize);
	config1.producer = producer1;
	config1.counsumer = consumer1;
	config1.normalExec = normalExec;

	//tester(config1);
	LOG_INFO("Config 1.2 successful");

	config1.numChunksPerThread = 2;
	config1.chunkSize = 3;
	config1.numDonationSteps = 2;
	config1.numProducerGroups = -1;
	config1.numProducerPerGroup = -1;
	config1.numProducers = 4;
	config1.numNormalThreads = 0;
	config1.numBlocksToBePassed = 1000;
	config1.numThreads = (3 * config1.numProducers + config1.numNormalThreads);
	config1.numBlocks = (config1.numThreads * config1.numChunksPerThread * config1.chunkSize);
	config1.producer = producer1;
	config1.counsumer = consumer1;
	config1.normalExec = normalExec;

	//tester(config1);
	LOG_INFO("Config 1.3 successful");

	config1.numChunksPerThread = 2;
	config1.chunkSize = 3;
	config1.numDonationSteps = 2;
	config1.numProducerGroups = -1;
	config1.numProducerPerGroup = -1;
	config1.numProducers = 10;
	config1.numNormalThreads = 0;
	config1.numBlocksToBePassed = 10;
	config1.numThreads = (3 * config1.numProducers + config1.numNormalThreads);
	config1.numBlocks = (config1.numThreads * config1.numChunksPerThread * config1.chunkSize);
	config1.producer = producer1;
	config1.counsumer = consumer1;
	config1.normalExec = normalExec;

	tester(config1);
	LOG_INFO("Config 1.4 successful");

	// config2 had multiple producers sending to same consumer
	TestConfig config2;

	config2.numChunksPerThread = 2;
	config2.chunkSize = 3;
	config2.numDonationSteps = 2;
	config2.numProducerGroups = 1;
	config2.numProducerPerGroup = 2;
	config2.numProducers = config2.numProducerGroups * config2.numProducerPerGroup;
	config2.numNormalThreads = 0;
	config2.numBlocksToBePassed = 100;
	config2.numThreads = (config2.numProducers + config2.numNormalThreads + config2.numProducerGroups);
	config2.numBlocks = (config2.numThreads * config2.numChunksPerThread * config2.chunkSize);
	config2.producer = producer2;
	config2.counsumer = consumer2;
	config2.normalExec = normalExec;

	//tester(config2);
	LOG_INFO("Config 2.1 successful");

	config2.numChunksPerThread = 4;
	config2.chunkSize = 3;
	config2.numDonationSteps = 2;
	config2.numProducerGroups = 2;
	config2.numProducerPerGroup = 4;
	config2.numProducers = config2.numProducerGroups * config2.numProducerPerGroup;
	config2.numNormalThreads = 0;
	config2.numBlocksToBePassed = 100;
	config2.numThreads = (config2.numProducers + config2.numNormalThreads + config2.numProducerGroups);
	config2.numBlocks = (config2.numThreads * config2.numChunksPerThread * config2.chunkSize);
	config2.producer = producer2;
	config2.counsumer = consumer2;
	config2.normalExec = normalExec;

	//tester(config2);
	LOG_INFO("Config 2.2 successful");

	config2.numChunksPerThread = 4;
	config2.chunkSize = 3;
	config2.numDonationSteps = 2;
	config2.numProducerGroups = 5;
	config2.numProducerPerGroup = 4;
	config2.numProducers = config2.numProducerGroups * config2.numProducerPerGroup;
	config2.numNormalThreads = 0;
	config2.numBlocksToBePassed = 100;
	config2.numThreads = (config2.numProducers + config2.numNormalThreads + config2.numProducerGroups);
	config2.numBlocks = (config2.numThreads * config2.numChunksPerThread * config2.chunkSize);
	config2.producer = producer2;
	config2.counsumer = consumer2;
	config2.normalExec = normalExec;

	//tester(config2);
	LOG_INFO("Config 2.3 successful");

	// config3 tests a single normalExec with different consumers
	TestConfig config3;

	config3.numChunksPerThread = 2;
	config3.chunkSize = 3;
	config3.numDonationSteps = 2;
	config3.numProducerGroups = -1;
	config3.numProducerPerGroup = -1;
	config3.numProducers = 0;
	config3.numNormalThreads = 1;
	config3.numBlocksToBePassed = 0;
	config3.numThreads = (3 * config3.numProducers + config3.numNormalThreads);
	config3.numBlocks = (config3.numThreads * config3.numChunksPerThread * config3.chunkSize);
	config3.producer = producer1;
	config3.counsumer = consumer1;
	config3.normalExec = normalExec;

	//tester(config3);
	LOG_INFO("Config 3.1 successful");


	config3.numChunksPerThread = 2;
	config3.chunkSize = 3;
	config3.numDonationSteps = 2;
	config3.numProducerGroups = -1;
	config3.numProducerPerGroup = -1;
	config3.numProducers = 4;
	config3.numNormalThreads = 1;
	config3.numBlocksToBePassed = 10;
	config3.numThreads = (3 * config3.numProducers + config3.numNormalThreads);
	config3.numBlocks = (config3.numThreads * config3.numChunksPerThread * config3.chunkSize);
	config3.producer = producer1;
	config3.counsumer = consumer1;
	config3.normalExec = normalExec;

	//tester(config3);
	LOG_INFO("Config 3.2 successful");

	LOG_INFO("Test Client");
	LOG_CLOSE();
	pthread_exit(NULL);

}
