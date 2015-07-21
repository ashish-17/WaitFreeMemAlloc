/*
 * TestStackArray.c
 *
 *  Created on: 13-Jul-2015
 *      Author: architaagarwal
 */

/*
    Test code for stack array.
 */

#include "../StackArray.h"
#include "../Block.h"
#include "../commons.h"
#include <pthread.h>


typedef struct _TestConfigStackArray {
	int numThreads;
	int numBlocksInStack;
} TestConfigStackArray;

typedef struct _StackArrayTestThreadData {
	StackArray *stack;
	TestConfigStackArray cfg;
	int threadId;
} StackArrayTestThreadData;

void testStackArrayPushAndPop(void *data) {
	LOG_PROLOG();
	StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
	if (threadData->stack != NULL) {
		int numOfBlocks = threadData->cfg.numBlocksInStack;
		bool success = false;
		for (int i = 0; i < numOfBlocks; i++) {
			int temp = (threadData->threadId * numOfBlocks + i);
			int *block = createBlock(sizeof(int));
			*block = temp;
			success = stackArrayPushContended(threadData->stack, block);
			if (!success) {
				LOG_WARN("Push to block ID %d failed", temp);
				my_free(block);
				--i;
			} else {
				LOG_INFO("Push to block ID %d passed", temp);
				int *poppedBlock = NULL;
				while (poppedBlock == NULL) {
					poppedBlock = stackArrayPopContended(threadData->stack);
					if (poppedBlock == NULL) {
						LOG_INFO("Contended pop failed");
					} else {
						LOG_INFO("Popped mem block has id = %d", *poppedBlock);
						my_free(poppedBlock);
						break;
					}
				}
			}
		}
	}

	LOG_EPILOG();
}

void testStackArrayOnlyPop(void *data) {
	LOG_PROLOG();
	StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
	if (threadData->stack != NULL) {
		int numOfBlocks = threadData->cfg.numBlocksInStack;

		for (int i = 0; i < numOfBlocks; i++) {
			int *block = stackArrayPopContended(threadData->stack);
			if (block == NULL) {
				--i;
				LOG_INFO("Contended pop failed");
			} else {
				LOG_INFO("Popped mem block has id = %d", *block);
				my_free(block);
			}
		}
	}

	LOG_EPILOG();
}

void testStackArrayOnlyPush(void *data) {
	LOG_PROLOG();
	StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
	if (threadData->stack != NULL) {
		int numOfBlocks = threadData->cfg.numBlocksInStack;
		bool success = false;
		for (int i = 0; i < numOfBlocks; i++) {
			int temp = (threadData->threadId * numOfBlocks + i);
			int *block = createBlock(sizeof(int));
			*block = temp;
			success = stackArrayPushContended(threadData->stack, block);
			if (!success) {
				LOG_WARN("Push to block ID %d failed", temp);
				my_free(block);
				--i;
			} else {
				LOG_INFO("Push to block ID %d passed", temp);
			}
		}
	}

	LOG_EPILOG();
}

void testStackArrayForcePush(void *data) {
	LOG_PROLOG();
	StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
	if (threadData->stack != NULL) {
		int numOfBlocks = threadData->cfg.numBlocksInStack;
		bool success = false;
		const int RETRY_FORCE_COUNT = 5;
		const int EXTRA_BLOCKS_COUNT = 5;
		for (int i = 0; i < (numOfBlocks + EXTRA_BLOCKS_COUNT); i++) {
			int temp = (threadData->threadId * numOfBlocks + i);
			int *block = createBlock(sizeof(int));
			*block = temp;
			success = stackArrayPushContended(threadData->stack, block);
			int retryCounts = 0;
			while (!success && (retryCounts < RETRY_FORCE_COUNT)) {
				success = stackArrayPushContended(threadData->stack, block);
				++retryCounts;
			}

			if (!success) {
				LOG_WARN("Push to block ID %d failed", temp);
				my_free(block);
			} else {
				LOG_INFO("Push to block ID %d passed in %d tries", temp, retryCounts);
			}
		}
	}

	LOG_EPILOG();
}

bool verifyDataOnStack(TestConfigStackArray cfg, StackArray *stack) {
	LOG_PROLOG();
	bool success = false;

	bool *blockVals = (bool*)my_malloc(sizeof(bool) * cfg.numBlocksInStack * cfg.numThreads);
	memset(blockVals, 0, sizeof(bool) * cfg.numBlocksInStack * cfg.numThreads);
	int *block = NULL;
	StackArrayElement *element = NULL;
	for (int i = 0; i< stack->maxElements; ++i) {
		element = getStackArrayElement(stack, i);
		LOG_INFO("element is %u", element);
		if (element != NULL) {
			block = element->value;
			LOG_INFO("came here");
			if (block != NULL) {
				if (blockVals[*block] == true) {
					LOG_ERROR("Block added twice into stack - %d", *block);
					break;
				}

				blockVals[*block] = true;
			} else {
				LOG_ERROR("Invalid block found on stack");
				break;
			}
		} else {
			LOG_ERROR("Invalid element found on stack");
			break;
		}
	}

	for (int j = 0; j < stack->maxElements; ++j) {
		if (!blockVals[*block]) {
			success = false;
			LOG_ERROR("Missing block on stack");
			break;
		} else if (!success) {
			success = true;
		}
	}

	my_free(blockVals);
	LOG_EPILOG();
	return success;
}

void testStackArray(TestConfigStackArray cfg) {
	int numOfElements = cfg.numThreads * cfg.numBlocksInStack;
	StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)my_malloc(cfg.numThreads * sizeof(StackArrayTestThreadData));
	if (threadData != NULL) {
		StackArray *stack = stackArrayCreate(sizeof(BLOCK_MEM), numOfElements);
		if (stack != NULL) {
			int rc = -1;
			int numThreads = 0;
			pthread_t *threads = (pthread_t*)my_malloc(sizeof(pthread_t) * cfg.numThreads);
			for (int t = 0; t < cfg.numThreads; t++) {
				threadData[t].stack = stack;
				threadData[t].threadId = t;
				threadData[t].cfg = cfg;
				LOG_DEBUG("Creating thread at Index %d", t);
				rc = pthread_create((threads + t), NULL, testStackArrayOnlyPush, (threadData + t));
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
			my_free(threads);

			bool test1Success = StackArrayIsFull(stack->top, stack->elements, stack->maxElements);
			if (test1Success) {
				LOG_INFO("Test 1 part 1 Successful");

				test1Success = verifyDataOnStack(cfg, stack);
				if (test1Success) {
					LOG_INFO("Test 1 part 2 Successful");
				} else {
					LOG_ERROR("Test 1 Part 2 failed");
				}

				threads = (pthread_t*)my_malloc(sizeof(pthread_t) * cfg.numThreads);
				for (int t = 0; t < cfg.numThreads; t++) {
					threadData[t].stack = stack;
					threadData[t].threadId = t;
					threadData[t].cfg = cfg;
					LOG_DEBUG("Creating thread at Index %d", t);
					rc = pthread_create((threads + t), NULL, testStackArrayOnlyPop, (threadData + t));
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

				my_free(threads);

				bool test2Success = stackArrayIsEmpty(stack->top, stack->elements);
				if (test2Success) {
					LOG_INFO("Test 2 Successful");
					/*
                    threads = (pthread_t*)my_malloc(sizeof(pthread_t) * cfg.numThreads);
                    for (int t = 0; t < cfg.numThreads; t++) {
                        threadData[t].stack = stack;
                        threadData[t].threadId = t;
                        threadData[t].cfg = cfg;
                        LOG_DEBUG("Creating thread at Index %d", t);
                        rc = pthread_create((threads + t), NULL, testStackArrayPushAndPop, (threadData + t));
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

                    my_free(threads);
					 */
					bool test3Success = stackArrayIsEmpty(stack->top, stack->elements);
					if (test3Success) {
						LOG_INFO("Test 3 Successful");

						// Test 4, Force push elements in the stack to test the stack overflow case.
						threads = (pthread_t*)my_malloc(sizeof(pthread_t) * cfg.numThreads);
						for (int t = 0; t < cfg.numThreads; t++) {
							threadData[t].stack = stack;
							threadData[t].threadId = t;
							threadData[t].cfg = cfg;
							LOG_DEBUG("Creating thread at Index %d", t);
							rc = pthread_create((threads + t), NULL, testStackArrayForcePush, (threadData + t));
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

						my_free(threads);

						LOG_INFO("Test 4 Successful");
					} else {
						LOG_ERROR("Test 3 Failed");
					}
				} else {
					LOG_ERROR("Test 2 Failed");
				}
			} else {
				LOG_ERROR("Test 1 Failed");
			}

			stackArrayFree(stack);
		}
	} else {
		LOG_ERROR("Unable to allocate memory to thread data");
	}
}

int testSAmain() {
	//int mainTestStackArray() {
	LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

	const int COUNT_TEST_CFG = 10;
	TestConfigStackArray cfg[COUNT_TEST_CFG];

	cfg[0].numBlocksInStack = 1;
	cfg[0].numThreads = 1;

	cfg[1].numBlocksInStack = 10;
	cfg[1].numThreads = 1;

	cfg[2].numBlocksInStack = 1;
	cfg[2].numThreads = 10;

	cfg[3].numBlocksInStack = 10;
	cfg[3].numThreads = 10;

	cfg[4].numBlocksInStack = 5;
	cfg[4].numThreads = 10;

	cfg[5].numBlocksInStack = 10;
	cfg[5].numThreads = 5;

	cfg[6].numBlocksInStack = 100;
	cfg[6].numThreads = 5;

	cfg[7].numBlocksInStack = 5;
	cfg[7].numThreads = 100;

	cfg[8].numBlocksInStack = 10;
	cfg[8].numThreads = 100;

	cfg[9].numBlocksInStack = 100;
	cfg[9].numThreads = 10;

	for (int i = 0; i < COUNT_TEST_CFG; ++i) {
		LOG_INFO("Test configuration %d, with %d threads and %d number of blocks per thread", (i+1), cfg[i].numThreads, cfg[i].numBlocksInStack);
		testStackArray(cfg[i]);
		LOG_INFO("Test configuration %d, passed", (i+1));
	}

	LOG_INFO("All tests passed");

	LOG_CLOSE();
	return 0;
}



