#include "WaitFreePool.h"
#include "RandomGenerator.h"
#include "commons.h"
#include <pthread.h>
# include "Block.h"
#include "Stack.h"
#include "HazardPointer.h"

HPStructure *globalHPStructure = NULL;

#define NUM_THREADS 2
#define ARRAY_LOOP_CTR 10

void* tester1(void *threadId) {
	printf("In thread %d\n", (int)threadId);
	for (int i = 1; i <= ARRAY_LOOP_CTR; i++) {
		int *element = (int*)malloc(sizeof(int));
		printf("thread %d allocated element ptr = %u\n", (int*)threadId, element);
		freeMemHP(globalHPStructure,(int*)threadId,element);
	}
}



int main() {

	globalHPStructure = (HPStructure*)malloc(sizeof(HPStructure));
	hpStructureCreate(globalHPStructure, NUM_THREADS, 1);
	printf("successfully initialized\n");
	int rc;
	pthread_t threads[NUM_THREADS];
	for (int t = 0; t < NUM_THREADS; t++) {
		printf("In main: creating thread %d\n", t);
		rc = pthread_create(&threads[t], NULL, tester1, (void *)t);
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
