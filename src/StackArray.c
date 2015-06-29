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

bool stackArrayIsEmpty(const StackArray *stack) {
	LOG_PROLOG();
	bool flag = false;

	if (stack != NULL) {
        flag = (stack->top == getStackArrayElement(stack, 0));
        LOG_DEBUG("Stack array is %s empty", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

bool StackArrayIsFull(const StackArray *stack) {
	LOG_PROLOG();
	bool flag = false;

	if (stack != NULL) {
        flag = (stack->top == getStackArrayElement(stack, stack->maxElements));
        LOG_DEBUG("Stack array is %s full", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;

	if (stack != NULL) {
        if (element != NULL) {
            if (StackArrayIsFull(stack)) {
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
        if (stackArrayIsEmpty(stack)) {
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
            if (StackArrayIsFull(stack)) {
                LOG_WARN("Push contended failed as array is full");
            } else {
                void *nullptr = NULL;
                if (oldTop->value != NULL) {
                    LOG_WARN("Value of oldTop->value = %x, push contended failed as not null", oldTop->value);
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
                } else if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
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
        if (stackArrayIsEmpty(stack)) {
            LOG_WARN("Pop contended failed as array is empty");
        } else {
            void *element = (oldTop - 1)->value;
            if (atomic_compare_exchange_strong(&(stack->top - 1)->value, &element, NULL)) {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                ptr = element;
                LOG_INFO("Pop contended successful");
            } else {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                LOG_WARN("Pop contended failed as element already popped by other thread");
            }
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return ptr;
}

typedef struct _StackArrayTestThreadData {
    StackArray *stack;
    int threadId;
} StackArrayTestThreadData;

/*
    Test code for stack array.
*/
#define NUM_THREADS 20
#define NUM_BLKS_IN_STACK 10
void testStackArrayPushAndPop(void *data) {
    LOG_PROLOG();
    StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
    if (threadData->stack != NULL) {
        int numOfBlocks = NUM_BLKS_IN_STACK;

        for (int i = 0; i < numOfBlocks; i++) {
            int temp = (threadData->threadId * numOfBlocks + i);
            Block *block = createBlock(threadData->threadId, temp);
            LOG_INFO("thread = %d pushing the block = %d was successful = %d", threadData->threadId, temp, stackArrayPushContended(threadData->stack, block));
        }

        for (int i = 0; i < numOfBlocks; i++) {
            Block *block = stackArrayPopContended(threadData->stack);
            if (block == NULL) {
                LOG_INFO("Thread = %d, didn't get the block", threadData->threadId);
            } else {
                LOG_INFO("Thread = %d, block just popped = %d", threadData->threadId, block->memBlock);
            }
        }
    }

	LOG_EPILOG();
}

void testStackArrayOnlyPop(void *data) {
    LOG_PROLOG();
    StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)data;
    if (threadData->stack != NULL) {
        int numOfBlocks = NUM_BLKS_IN_STACK;

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
        int numOfBlocks = NUM_BLKS_IN_STACK;
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

bool verifyDataOnStack(StackArray *stack) {
    bool success = false;

    bool blockVals[NUM_THREADS * NUM_BLKS_IN_STACK] = {0};
    Block *block = NULL;
    StackArrayElement *element = NULL;
    for (int i = 0; i< stack->maxElements; ++i) {
        element = getStackArrayElement(stack, i);
        if (element != NULL) {
            block = (Block*)element->value;
            if (block != NULL) {
                if (blockVals[block->memBlock] == true) {
                    LOG_WARN("Block added twice into stack - %d", block->memBlock);
                    break;
                }

                blockVals[block->memBlock] = true;
            } else {
                LOG_WARN("Invalid block found on stack");
                break;
            }
        } else {
            LOG_WARN("Invalid element found on stack");
            break;
        }
    }

    for (int j = 0; j < stack->maxElements; ++j) {
        if (!blockVals[block->memBlock]) {
            success = false;
            break;
        } else if (!success) {
            success = true;
        }
    }

    return success;
}

int main() {
    LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

    int numOfElements = NUM_THREADS * NUM_BLKS_IN_STACK;
    StackArrayTestThreadData *threadData = (StackArrayTestThreadData*)malloc(NUM_THREADS * sizeof(StackArrayTestThreadData));
	StackArray *stack = stackArrayCreate(sizeof(Block), numOfElements);
    if (stack != NULL) {
        int rc = -1;
        int numThreads = 0;
        pthread_t threads[NUM_THREADS];
        for (int t = 0; t < NUM_THREADS; t++) {
            threadData[t].stack = stack;
            threadData[t].threadId = t;
            LOG_DEBUG("Creating thread at Index %d", t);
            rc = pthread_create(&threads[t], NULL, testStackArrayOnlyPush, (threadData + t));
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

        bool test1Success = StackArrayIsFull(stack);
        if (test1Success) {
            LOG_INFO("Test 1 part 1 Successful");
            test1Success = verifyDataOnStack(stack);
            if (test1Success) {
                LOG_INFO("Test 1 part 2 Successful");
            } else {
                LOG_WARN("Test 1 Part 2 failed");
            }

            pthread_t threads[NUM_THREADS];
            for (int t = 0; t < NUM_THREADS; t++) {
                threadData[t].stack = stack;
                threadData[t].threadId = t;
                LOG_DEBUG("Creating thread at Index %d", t);
                rc = pthread_create(&threads[t], NULL, testStackArrayOnlyPop, (threadData + t));
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

            bool test2Success = stackArrayIsEmpty(stack);
            if (test2Success) {
                LOG_INFO("Test 2 Successful");
            } else {
                LOG_WARN("Test 2 Failed");
            }
        } else {
            LOG_WARN("Test 1 Failed");
        }

        stackArrayFree(stack);
    }

    LOG_CLOSE();
    return 0;
}
