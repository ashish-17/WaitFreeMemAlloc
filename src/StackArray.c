#include "StackArray.h"
#include "Block.h"
#include "commons.h"
#include <pthread.h>

StackArrayElement* getStackArrayElement(const StackArray* stack, int index);

StackArrayElement* getStackArrayElement(const StackArray* stack, int index) {
	LOG_PROLOG();
	LOG_DEBUG("Get Stack Array Element in stack(%x) at index %d", stack, index);
	StackArrayElement* ptr = NULL;
    if (stack != NULL) {
        if ((index > stack->maxElements) || (index < 0)) {
            LOG_ERROR("Invalid index for stackArray");
        } else {
            ptr = (stack->elements + index);
        }
    } else {
        LOG_ERROR("Invalid memory address for stackArray");
    }

	LOG_EPILOG();
	return ptr;
}

StackArray* stackArrayCreate(int elementSize, int maxElements) {
	LOG_PROLOG();
	LOG_DEBUG("Create a stackArray with maximum %d elements and with element size %d bytes", maxElements, elementSize);

    StackArray* stack = (StackArray*)my_malloc(sizeof(StackArray));
    if (stack != NULL) {
        size_t sizeOfStackElements = sizeof(StackArrayElement) * (maxElements + 1);
        stack->elements = (StackArrayElement*)my_malloc(sizeOfStackElements);
        if (stack->elements != NULL) {
            memset(stack->elements, NULL, sizeOfStackElements);
            stack->top = stack->elements;
            stack->elementSize = elementSize;
            stack->maxElements = maxElements;
        } else {
            LOG_ERROR("Error allocating memory to elements in stackArray");
            my_free(stack);
            stack = NULL;
        }
    } else {
        LOG_ERROR("Error creating a stackArray");
    }

	LOG_EPILOG();
	return stack;
}

void stackArrayFree(StackArray *stack) {
	LOG_PROLOG();
	LOG_DEBUG("Free stack at address %x", stack);

    if (stack != NULL) {
        for (int i = 0; i < (stack->maxElements + 1); i++) {
            StackArrayElement* element = getStackArrayElement(stack, i);
            if (element != NULL) {
                if (element->value != NULL) {
                    my_free(element->value);
                    element->value = NULL;
                }
            } else {
                LOG_ERROR("Invalid stackArray element");
            }
        }

        my_free(stack->elements);
        stack->elements = NULL;
        stack->top = NULL;
        stack->elementSize = 0;
        stack->maxElements = 0;

        my_free(stack);
        stack = NULL;
    } else {
        LOG_ERROR("Invalid memory address for stackArray");
    }

	LOG_EPILOG();
}

bool stackArrayIsEmpty(const StackArrayElement *top, const StackArrayElement *baseAddress) {
	LOG_PROLOG();
	bool flag = false;

	if (top != NULL && baseAddress != NULL) {
        flag = (top == baseAddress);
        LOG_DEBUG("Stack array is %s empty", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory addresses for stack");
	}

	LOG_EPILOG();
	return flag;
}

bool StackArrayIsFull(const StackArrayElement *top, const StackArrayElement *baseAddress, int sizeOfStack) {
	LOG_PROLOG();
	bool flag = false;

	if (top != NULL && baseAddress != NULL) {
        flag = (top == (baseAddress + sizeOfStack));
        LOG_DEBUG("Stack array is %s full", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory addresses for stack");
	}

	LOG_EPILOG();
	return flag;
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;

	if (stack != NULL) {
        if (element != NULL) {
            if (StackArrayIsFull(stack->top, stack->elements, stack->maxElements)) {
                LOG_WARN("Push uncontended failed as array is full");
            } else {
                if (stack->top != NULL) {
                    stack->top->value = element;
                    stack->top++;
                    flag = true;
                } else {
                    LOG_ERROR("Top of stackArray is at an invalid position");
                }
            }
        } else {
            LOG_ERROR("Trying to push a NULL value in stackArray");
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

void* stackArrayPopUncontended(StackArray *stack) {
	LOG_PROLOG();
	void *ptr = NULL;
	if (stack != NULL) {
        if (stackArrayIsEmpty(stack->top, stack->elements)) {
            LOG_WARN("Pop uncontended failed as array is empty");
        } else {
            if (stack->top != NULL) {
                stack->top--;
                ptr = stack->top->value;
                stack->top->value = NULL;
            } else {
                LOG_ERROR("Top of stackArray is at an invalid position");
            }
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return ptr;
}

bool stackArrayPushContended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;
	if (stack != NULL) {
        if (element != NULL) {
            StackArrayElement *oldTop = stack->top;
            if (StackArrayIsFull(oldTop, stack->elements, stack->maxElements)) {
                LOG_WARN("Push contended failed as array is full");
            } else {
                void *nullptr = NULL;
                if (oldTop->value != NULL) {
                    LOG_WARN("Value of oldTop->value = %x, push contended failed as not null", oldTop->value);
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
                    if (stack->top > (stack->elements + stack->maxElements)) {
                        LOG_ERROR("Stack overflow");
                    }
                } else if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
                    if (stack->top > (stack->elements + stack->maxElements)) {
                        LOG_ERROR("Stack overflow");
                    }

                    LOG_DEBUG("Successful atomic_compare_exchange_strong");
                    flag = true;
                }
            }
        } else {
            LOG_ERROR("Trying to push a NULL value in stackArray");
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

void* stackArrayPopContended(StackArray *stack) {
	LOG_PROLOG();
	void *ptr = NULL;
	if (stack != NULL) {
        StackArrayElement *oldTop = stack->top;
        if (stackArrayIsEmpty(oldTop, stack->elements)) {
            LOG_WARN("Pop contended failed as array is empty");
        } else {
            LOG_DEBUG("Base address = %u, OldTop = %u, top = %u", stack->elements, oldTop, stack->top);
            if ((oldTop - 1) < stack->elements) {
                LOG_ERROR("Segmentation fault");
            }

            void *element = (oldTop - 1)->value;
            if (atomic_compare_exchange_strong(&(stack->top - 1)->value, &element, NULL)) {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                if (stack->top < stack->elements) {
                    LOG_ERROR("Segmentation fault");
                }
                ptr = element;
                LOG_DEBUG("Base address = %u, top = %u", stack->elements, oldTop-1)
                LOG_INFO("Pop contended successful (%d)", ((stack->top - stack->elements) / sizeof(StackArrayElement)));
            } else {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                if (stack->top < stack->elements) {
                    LOG_ERROR("Segmentation fault");
                }
                LOG_DEBUG("Base address = %u, top = %u", stack->elements, oldTop-1)
                LOG_WARN("Pop contended failed as element already popped by other thread");
            }
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return ptr;
}

/*
    Test code for stack array.
*/

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
            Block *block = createBlock(threadData->threadId, temp);
            success = stackArrayPushContended(threadData->stack, block);
            if (!success) {
                LOG_WARN("Push to block ID %d failed", temp);
                my_free(block);
                --i;
            } else {
                LOG_INFO("Push to block ID %d passed", temp);
                Block *poppedBlock = NULL;
                while (poppedBlock == NULL) {
                    poppedBlock = stackArrayPopContended(threadData->stack);
                    if (poppedBlock == NULL) {
                        LOG_INFO("Contended pop failed");
                    } else {
                        LOG_INFO("Popped mem block has id = %d", poppedBlock->memBlock);
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
            Block *block = stackArrayPopContended(threadData->stack);
            if (block == NULL) {
                --i;
                LOG_INFO("Contended pop failed");
            } else {
                LOG_INFO("Popped mem block has id = %d", block->memBlock);
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
            Block *block = createBlock(threadData->threadId, temp);
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
            Block *block = createBlock(threadData->threadId, temp);
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
    Block *block = NULL;
    StackArrayElement *element = NULL;
    for (int i = 0; i< stack->maxElements; ++i) {
        element = getStackArrayElement(stack, i);
        if (element != NULL) {
            block = (Block*)element->value;
            if (block != NULL) {
                if (blockVals[block->memBlock] == true) {
                    LOG_ERROR("Block added twice into stack - %d", block->memBlock);
                    break;
                }

                blockVals[block->memBlock] = true;
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
        if (!blockVals[block->memBlock]) {
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
        StackArray *stack = stackArrayCreate(sizeof(Block), numOfElements);
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

int stackmain() {
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
