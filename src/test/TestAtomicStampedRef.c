#include "../AtomicStampedRef.h"
#include <pthread.h>

#define THREAD_CAS_SUCCESS 1

typedef struct TestConfigAtomicStampedRef {
    int numThreads;
    int* threadSucessStatus;
} TestConfigAtomicStampedRef;

typedef struct TestDataAtomicStampedRef {
    AtomicStampedRef *ref;
    int threadIndex;
    TestConfigAtomicStampedRef* cfg;
} TestDataAtomicStampedRef;

void* testAtomicValChange(void* value) {
    LOG_PROLOG();

    TestDataAtomicStampedRef* testData = (TestDataAtomicStampedRef*)value;
    if (testData != NULL) {
        if (testData->ref != NULL) {
            void* oldRef = getRef(testData->ref);
            int oldStamp = getStamp(testData->ref);

            int* newRef = (int*)my_malloc(sizeof(int));
            *newRef = testData->threadIndex;

            int newStamp = (oldStamp + 1);

            LOG_DEBUG("Old Ref = %x", getRef(testData->ref));
            CASResult result = cmpAndSet(testData->ref, oldRef, newRef, oldStamp, newStamp);
            if (result.success) {
                testData->cfg->threadSucessStatus[testData->threadIndex] = THREAD_CAS_SUCCESS;
                LOG_DEBUG("New Atomic Ref = %x", getRef(testData->ref));
                LOG_DEBUG("Old ref = %x, newRef = %x, oldStamp = %x, newStamp = %x, oldData = %d, data = %d", oldRef, newRef, oldStamp, newStamp, *((int*)oldRef), *newRef);

                //Free the memory in the old reference
                my_free(result.ref);
            } else {

                //CAS was unsuccessful, so free the newly created reference.
                my_free(result.ref);
            }
        }
    }

    LOG_EPILOG();
    return NULL;
}

void doTestAtomicStampedRef(TestConfigAtomicStampedRef* cfg) {
    LOG_PROLOG();

    if (cfg != NULL) {
        TestDataAtomicStampedRef* testData = (TestDataAtomicStampedRef*)my_malloc(sizeof(TestDataAtomicStampedRef) * cfg->numThreads);
        if (testData != NULL) {
            pthread_t *threads = (pthread_t*)my_malloc(sizeof(pthread_t) * cfg->numThreads);
            if (threads != NULL) {
                int numThreads = 0;
                int rc = 0;
                int* newRef = (int*)my_malloc(sizeof(int));
                *newRef = -1;
                AtomicStampedRef* atomicRef = createAtomicStampedRef(newRef, 0);
                for (int t = 0; t < cfg->numThreads; t++) {
                    LOG_DEBUG("Creating thread at Index %d", t);
                    testData[t].cfg = cfg;
                    testData[t].threadIndex = t;
                    testData[t].ref = atomicRef;
                    rc = pthread_create((threads + t), NULL, testAtomicValChange, (testData + t));
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
                threads= NULL;

                void* sharedRef = destroyAtomicStampedRef(testData[0].ref);
                my_free(sharedRef);

                my_free(testData);
                testData = NULL;
            } else {
                LOG_ERROR("Error creating threads");
            }
        } else {
            LOG_ERROR("Unable to allocate memory for test data");
        }
    }

    LOG_EPILOG();
}

int countSuccess(int* threadSuccessStatus, int numThreads) {
    LOG_PROLOG();

    int count = 0;
    for (int i = 0; i < numThreads; ++i) {
        if ((threadSuccessStatus + i) != NULL) {
            if (*(threadSuccessStatus + i) == THREAD_CAS_SUCCESS) {
                ++count;
            }
        } else {
            LOG_ERROR("Invalid thread");
        }
    }

    LOG_EPILOG();
    return count;
}

int masdasdasdasin() {
    LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

    #define COUNT_TEST_CONFIG 10
    TestConfigAtomicStampedRef cfg[COUNT_TEST_CONFIG];
    cfg[0].numThreads = 1;
    cfg[1].numThreads = 2;
    cfg[2].numThreads = 3;
    cfg[3].numThreads = 4;
    cfg[4].numThreads = 50;
    cfg[5].numThreads = 60;
    cfg[6].numThreads = 700;
    cfg[7].numThreads = 80;
    cfg[8].numThreads = 90;
    cfg[9].numThreads = 1000;

    for (int i = 0; i < COUNT_TEST_CONFIG; ++i) {
        cfg[i].threadSucessStatus = (int*)my_malloc(sizeof(int) * cfg[i].numThreads);
        memset(cfg[i].threadSucessStatus, 0, sizeof(int) * cfg[i].numThreads);

        doTestAtomicStampedRef(cfg + i);
    }

    for (int i = 0; i < COUNT_TEST_CONFIG; ++i) {
        LOG_INFO("Thread Success status = %d / %d", countSuccess(cfg[i].threadSucessStatus, cfg[i].numThreads), cfg[i].numThreads)
        my_free(cfg[i].threadSucessStatus);
    }

	LOG_CLOSE();
	return 0;
}
