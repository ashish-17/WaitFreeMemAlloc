#include "../CircularQueue.h"
#include "../RandomGenerator.h""

typedef struct TestCircularQueueConfig {
    int elementSize;
    int numElements;
} TestCircularQueueConfig;

void* getTestElement(int elementSize) {
    LOG_PROLOG();

    void* element = my_malloc(elementSize);
    if (elementSize == sizeof(int)) {
        *((int*)element) = randint(100);
    } else if (elementSize == sizeof(double)) {
        *((double*)element) = randint(100) + 3.456;
    }

	LOG_EPILOG();
	return element;
}

bool testEnqueueMaxElements(CircularQueue* queue) {
    LOG_PROLOG();

    bool success = true;
    int maxElements = circularQueueGetMaxNumberOfElements(queue);
    for (int i = 0; i < maxElements; ++i) {
        if (!circularQueueEnq(queue, getTestElement(circularQueueGetElementSize(queue)))) {
            success = false;
            break;
        } else if (!success) {
            success = true;
        }
    }

	LOG_EPILOG();
	return success;
}

bool testDequeueMaxElements(CircularQueue* queue) {
    LOG_PROLOG();

    bool success = true;
    int maxElements = circularQueueGetMaxNumberOfElements(queue);
    for (int i = 0; i < maxElements; ++i) {
        void* element = circularQueueDeq(queue);
        if (element == NULL) {
            success = false;
            break;
        } else {
            my_free(element);
            if (!success) {
                success = true;
            }
        }
    }

	LOG_EPILOG();
	return success;
}

// First enqueue 50% elements, then dequeue other 50% half
bool testEnqDeq50PercentElements(CircularQueue* queue) {
    LOG_PROLOG();

    bool success = true;
    int maxElements = circularQueueGetMaxNumberOfElements(queue);
    int elementsToEnq = (maxElements / 2);
    int elementsToDeq = elementsToEnq;

    for (int i = 0; i < elementsToEnq; ++i) {
        if (!circularQueueEnq(queue, getTestElement(circularQueueGetElementSize(queue)))) {
            success = false;
            break;
        } else if (!success) {
            success = true;
        }
    }

    if (success) {
        for (int i = 0; i < elementsToDeq; ++i) {
            void* element = circularQueueDeq(queue);
            if (element == NULL) {
                success = false;
                break;
            } else {
                my_free(element);
                if (!success) {
                    success = true;
                }
            }
        }
    }

	LOG_EPILOG();
	return success;
}

bool doTestCircularQueue(TestCircularQueueConfig cfg) {
    LOG_PROLOG();

    bool success = false;
    CircularQueue* queue = circularQueueCreate(cfg.elementSize, cfg.numElements);
    if (queue != NULL) {
        if (testEnqueueMaxElements(queue)) {
            LOG_INFO("Test case 1 pass");

            // As the queue is full, try to enqueue one more item and note its failure
            if (circularQueueEnq(queue, getTestElement(circularQueueGetElementSize(queue)))) {
                LOG_ERROR("Test case 2 fail");
            } else {
                LOG_INFO("Test case 2 pass");
                if (testDequeueMaxElements(queue)) {
                    LOG_INFO("Test case 3 pass");

                    //As the queue should be empty now, so we try to dequeue another element and see the result
                    void* element = circularQueueDeq(queue);
                    if (element == NULL) {
                        LOG_INFO("Test case 4 pass");

                        if (testEnqDeq50PercentElements(queue)) {
                            LOG_INFO("Test case 5 pass");

                            // Now both head and tail should be in the middle of queue
                            // So we again try to enqueue max elements in the queue
                            if (testEnqueueMaxElements(queue)) {
                                LOG_INFO("Test case 6 pass");
                                // As the queue is full, try to enqueue one more item and note its failure
                                if (circularQueueEnq(queue, getTestElement(circularQueueGetElementSize(queue)))) {
                                    LOG_ERROR("Test case 7 fail");
                                } else {
                                    LOG_INFO("Test case 7 pass");
                                    if (testDequeueMaxElements(queue)) {
                                        LOG_INFO("Test case 8 pass");

                                        //As the queue should be empty now, so we try to dequeue another element and see the result
                                        void* element = circularQueueDeq(queue);
                                        if (element == NULL) {
                                            LOG_INFO("Test case 9 pass");
                                            success = true;

                                        } else {
                                            LOG_ERROR("Test case 9 fail");
                                        }
                                    } else {
                                        LOG_ERROR("Test case 8 fail");
                                    }
                                }
                            } else {
                                LOG_ERROR("Test case 6 fail");
                            }

                        } else {
                            LOG_ERROR("Test case 5 fail");
                        }
                    } else {
                        LOG_ERROR("Test case 4 fail");
                    }
                } else {
                    LOG_ERROR("Test case 3 fail");
                }
            }
        } else {
            LOG_ERROR("Test case 1 fail");
        }

        circularQueueFree(queue);
    } else {
        LOG_ERROR("Error creating queue");
    }

	LOG_EPILOG();
	return success;
}

int main() {
    LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

    #define COUNT_TESTS 10
    TestCircularQueueConfig cfg[COUNT_TESTS];
    cfg[0].elementSize = sizeof(int);
    cfg[0].numElements = 1;
    cfg[1].elementSize = sizeof(int);
    cfg[1].numElements = 2;
    cfg[2].elementSize = sizeof(int);
    cfg[2].numElements = 5;
    cfg[3].elementSize = sizeof(int);
    cfg[3].numElements = 10;
    cfg[4].elementSize = sizeof(int);
    cfg[4].numElements = 50;
    cfg[5].elementSize = sizeof(double);
    cfg[5].numElements = 1;
    cfg[6].elementSize = sizeof(double);
    cfg[6].numElements = 2;
    cfg[7].elementSize = sizeof(double);
    cfg[7].numElements = 5;
    cfg[8].elementSize = sizeof(double);
    cfg[8].numElements = 10;
    cfg[9].elementSize = sizeof(double);
    cfg[9].numElements = 50;

    for (int i = 0; i < COUNT_TESTS; ++i) {
        LOG_INFO("Start test for config - number of  elements = %d, element size = %d", cfg[i].numElements, cfg[i].elementSize);
        if (!doTestCircularQueue(cfg[i])) {
            LOG_ERROR("Test case failed");
        }
    }

    LOG_INFO("All test cases passed");

	LOG_CLOSE();
    return 0;
}
