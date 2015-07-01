#include "CodeCorrectness.h"
#include "RandomGenerator.h"
#include <pthread.h>

bool *hashTable;
pthread_mutex_t *locks;

void hashTableCreate(int numOfBlocks) {
	LOG_PROLOG();
	hashTable = (bool*) my_malloc(sizeof(bool) * numOfBlocks);
	locks = (pthread_mutex_t*) my_malloc(sizeof(pthread_mutex_t) * numOfBlocks);
	for (int i = 0; i < numOfBlocks; i++) {
		hashTable[i] = false;
		pthread_mutex_init(&locks[i], NULL);
	}
	LOG_EPILOG();
}

void hashTableFree(int numOfBlocks) {
	LOG_PROLOG();
	if (hashTable != NULL) {
		my_free(hashTable);
		hashTable = NULL;
	}
	else {
		LOG_ERROR("Trying to free hashTable which was a NULL pointer");
	}
	if (locks != NULL) {
		for (int i = 0; i < numOfBlocks; i++) {
			pthread_mutex_destroy(&locks[i]);
		}
		my_free(locks);
		locks = NULL;
	}
	else {
		LOG_ERROR("Trying to free locks which was a NULL pointer");
	}
	LOG_EPILOG();
}


bool setFlagForAllocatedBlock(int blockNum) {
	LOG_PROLOG();
	bool flag = false;
	pthread_mutex_lock (&locks[blockNum]);
	if (hashTable[blockNum]) {
		flag = false;
	}
	else {
		hashTable[blockNum] = true;
		flag = true;
	}
	pthread_mutex_unlock(&locks[blockNum]);
	LOG_EPILOG();
	return flag;
}

bool clearFlagForAllocatedBlock(int blockNum) {
	LOG_PROLOG();
	bool flag;
	pthread_mutex_lock (&locks[blockNum]);
	if (!hashTable[blockNum]) {
		flag = false;
	}
	else {
		hashTable[blockNum] = false;
		flag = true;
	}
	pthread_mutex_unlock(&locks[blockNum]);
	LOG_EPILOG();
	return flag;
}

typedef struct _CodeCorrectnessTestStructure {
	int threadId;
	int numOfBlocks;
} CodeCorrectnessTestStructure;


void testCodeCorrectness(CodeCorrectnessTestStructure *codeCorrectDS) {
	LOG_PROLOG();
	int threadId = codeCorrectDS->threadId;
	int numOfBlocks = codeCorrectDS->numOfBlocks;

	for (int i = 0; i < 20; i++) {
		int allocBlock = randint(numOfBlocks + 1);
		LOG_INFO("thread %d allocated block %d", threadId, allocBlock);
		bool success = setFlagForAllocatedBlock(allocBlock);
		if (success) {
			LOG_INFO("thread %d successfully set the flag for block %d", threadId, allocBlock);
			if (clearFlagForAllocatedBlock(allocBlock)) {
				LOG_INFO("thread %d successfully cleared the flag for block %d", threadId, allocBlock);
			}
			else {
				LOG_ERROR("thread %d could not clear the flag for block %d", threadId, allocBlock);
			}
		}
		else {
			LOG_WARN("thread %d could not set the flag for block %d", threadId, allocBlock);
		}
	}
	LOG_EPILOG();
}

void thrmain() {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

	int NUM_THREADS = 100;
	int NUM_BLOCKS = 20;
	hashTableCreate(NUM_BLOCKS);
	pthread_t threads[NUM_THREADS];
	CodeCorrectnessTestStructure *codeCorrectDS = (CodeCorrectnessTestStructure*)malloc(NUM_THREADS * sizeof(CodeCorrectnessTestStructure));

	int rc = -1;
	int numThreads = 0;

	for (int t = 0; t < NUM_THREADS; t++) {
		codeCorrectDS[t].numOfBlocks = NUM_BLOCKS;
		codeCorrectDS[t].threadId = t;
		LOG_DEBUG("Creating thread at Index %d", t);
		rc = pthread_create(&threads[t], NULL, testCodeCorrectness, (codeCorrectDS + t));
		if (rc){
			LOG_ERROR("Error creating thread with error code %d", rc);
			break;
		} else {
			numThreads++;
		}
	}

	void *status;
	for (int t = 0; t < numThreads; t++) {
		rc = pthread_join(threads[t], &status);
	}
	hashTableFree(NUM_BLOCKS);
	LOG_CLOSE();
}
