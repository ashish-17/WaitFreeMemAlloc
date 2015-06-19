#include "WaitFreePool.h"
#include "commons.h"
#include "StackPool.h"
#include "QueuePool.h"
#include "Stack.h"
#include "Chunk.h"
#include "LocalPool.h"
#include "FreePool.h"
#include "FullPool.h"
#include "SharedPools.h"
#include <assert.h>
#include "HazardPointer.h"

typedef struct {
	AtomicStampedReference *helpers;
} Announce;

typedef struct {
	int noOfOps;
	int lastDonated;
	int numOfPassed;
	bool addInFreePoolC;
} Donor;

typedef struct{
	Donor *donors;
	int numOfDonors;
} Info;

typedef struct _Memory {
	StackPool* fullPool;
	StackPool* localPool;
	StackPool* freePoolUC;
	QueuePool* freePoolC;
	SharedQueuePools *sharedQueuePools;
	Announce* announce;
	Info* info;
	int m;
	int n;
	int c;
	int C;
} Memory;

Memory* memory = NULL;

//Chunk* doHelp(int threadToBeHelped, Chunk *stolenChunk, Helper *announceOfThreadToBeHelped);
Chunk* doHelp(int threadId, int threadToBeHelped, Chunk *stolenChunk, ReferenceIntegerPair *announceOfThreadToBeHelped);
bool donate(int threadId, Chunk *chunk);
Block* allocate(int threadId, bool toBePassed);
void freeMem(int threadId, Block *block);
Chunk* moveFromSharedQueuePools(int threadId);

/*Helper* getHelperEntry(int index) {
	return (memory->announce->helpers + index);
}*/

AtomicStampedReference* getHelperEntry(int index) {
	return (memory->announce->helpers + index);
}

Donor* getDonorEntry(int index) {
	return (memory->info->donors + index);
}

void createWaitFreePool(int m, int n, int c, int C) {

	memory = (Memory*)my_malloc(sizeof(Memory));

	int numOfThreads = n;
	int totalBlocks = m;
	int numOfBlocksPerChunk = c;
	int numOfChunks = m/c;
	int numOfChunksPerThread = numOfChunks/numOfThreads;

	//numOfThreads = n + 1;
	memory->fullPool = createFullPool(numOfThreads);
	memory->localPool = createLocalPool(numOfThreads);
	memory->freePoolUC = createFreePoolUC(numOfThreads);
	memory->freePoolC = createFreePoolC(numOfThreads);
	printf("created freePoolC\n");
	memory->sharedQueuePools = createSharedQueuePools(numOfThreads);
	printf("created SQP\n");
	numOfThreads = n;
	memory->m = m; memory->n = n; memory->c = c; memory->C = C;

	Chunk *chunk;
	Block *block;
	int blockNumber = 0;

	// Set-up of initial local pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(-1, blockNumber); //set the owner of the block to be -1 initially
				blockNumber++;
				putInChunkUncontended(chunk, block);
			}
			putInLocalPool(memory->localPool,j,chunk);
		}
	}
	printf("set up localPool with chunk\n");

	// Set-up of initial full pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < numOfChunksPerThread - 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(-1, blockNumber);
				blockNumber++;
				putInChunkUncontended(chunk, block);
			}
			printf("created the chunk for FP\n");
			putInOwnFullPool(memory->fullPool,j,chunk);
			//printf("fullPoolEmpty[%d] %u %u\n",j, isFullPoolEmpty(memory->fullPool,j), flag);
		}
	}
	printf("set up FullPools with chunks\n");

	//numOfThreads = n + 1;
	memory->announce = (Announce*)my_malloc(sizeof(Announce));
	memory->announce->helpers = (AtomicStampedReference*) my_malloc(sizeof(AtomicStampedReference)*numOfThreads);
	for(int i = 0; i < numOfThreads; i++) {
		AtomicStampedReference* helperEntry = getHelperEntry(i);
		bool* tempBoolObj = (bool*)my_malloc(sizeof(bool));
		*tempBoolObj = false;
		createAtomicStampedReference(helperEntry,tempBoolObj,0);
	}

	// Set up initial info array
	memory->info = (Info*)my_malloc(sizeof(Info));
	memory->info->donors = (Donor*) my_malloc(sizeof(Donor)*numOfThreads);
	memory->info->numOfDonors = numOfThreads;
	//memory->info->numOfPassed = 0;
	for(int i = 0; i < numOfThreads; i++) {
		Donor* donorEntry = getDonorEntry(i);
		donorEntry->lastDonated = i;
		donorEntry->noOfOps = 0;
		donorEntry->numOfPassed = 0;
		donorEntry->addInFreePoolC = false;
	}


}



Block* allocate(int threadId, bool toBePassed) {

	Chunk *stolenChunk;
	Block *block;
	int threadToBeHelped;
	bool addInFreePoolC = false;
	Donor *donor;

	//Helper *announceOfThreadToBeHelped;
	ReferenceIntegerPair *announceOfThreadToBeHelped;
	//printf("allocate: threadID = %d\n", threadId);
	if (toBePassed) {
		donor = getDonorEntry(threadId);
		donor->numOfPassed++;
		if (donor->numOfPassed %  memory->c == 1) {
			printf("allocate: threadId : %d, setting addInFreePoolC true\n", threadId);
			donor->addInFreePoolC = true;
		}
	}
	Chunk* chunk = getFromLocalPool(memory->localPool, threadId);
	//printf("allocate: threadID = %d, chunk ptr = %u\n", threadId, chunk);
	if (!isChunkEmpty(chunk)) {
		block = getFromChunkUncontended(chunk);
		block->threadId = threadId;
		putInLocalPool(memory->localPool, threadId, chunk);
		return block;
	}
	else {
		printf("allocate: threadId = %d: chunk in localPool is empty\n", threadId);
		donor = getDonorEntry(threadId);
		if (donor->addInFreePoolC) {
			printf("allocate: threadId = %d: putting in freePoolC\n", threadId);
			putInFreePoolC(memory->freePoolC, threadId, chunk);
			donor->addInFreePoolC = false;
		}
		else {
			putInFreePoolUC(memory->freePoolUC, threadId, chunk);
		}
		while (true) { // handling call to allocate again
			if (isFullPoolEmpty(memory->fullPool,threadId)) {
				printf("allocate: threadId = %d: *****fullPool is empty******\n", threadId);
				//getHelperEntry(threadId)->needHelp = true;
				*(bool*)getHelperEntry(threadId)->atomicRef->reference = true;
				stolenChunk = NULL;
				for(int i = 1; i <= memory->n; i++) {
					threadToBeHelped = (threadId + i) % memory->n;
					//announceOfThreadToBeHelped = getHelperEntry(threadToBeHelped);
					announceOfThreadToBeHelped = setHazardPointer(globalHPStructure, threadId, getHelperEntry(threadToBeHelped)->atomicRef);
					//if(announceOfThreadToBeHelped->needHelp) {
					if(*(bool*)announceOfThreadToBeHelped->reference) {
						printf("allocate: threadId = %d: going to help thread %d\n", threadId, threadToBeHelped);
						assert(globalHPStructure->topPointers[threadId] == 1);
						stolenChunk = doHelp(threadId, threadToBeHelped, stolenChunk, announceOfThreadToBeHelped);
						assert(globalHPStructure->topPointers[threadId] == 0);
					}
					else {
						assert(globalHPStructure->topPointers[threadId] == 1);
						clearHazardPointer(globalHPStructure, threadId);
						assert(globalHPStructure->topPointers[threadId] == 0);
					}
				}
				if (stolenChunk != NULL) {
					printf("allocate: threadId = %d: stolenChunk was not null. Putting in own fullPool\n", threadId);
					putInOwnFullPool(memory->fullPool, threadId, stolenChunk);
					printf("allocate: threadId = %d: stolenChunk was not null. Putting in own fullPool successful\n", threadId);
				}
			}

			while(true) {
				//printf("allocate: threadId = %d, isFullPoolEmpty = %d\n", threadId, isFullPoolEmpty(memory->fullPool,threadId));
				chunk = getFromOwnFullPool(memory->fullPool,threadId);
				//printf("allocate: threadId = %d, after removing a chunk, isFullPoolEmpty = %d\n", threadId, isFullPoolEmpty(memory->fullPool,threadId));
				assert(globalHPStructure->topPointers[threadId] == 0);
				if (chunk != NULL) {
					assert(globalHPStructure->topPointers[threadId] == 0);
					if (getDonorEntry(threadId)->noOfOps != memory->C) {
						//printf("allocate: threadId = %d: noOfOps = %d\n", threadId,getDonorEntry(threadId)->noOfOps);
						getDonorEntry(threadId)->noOfOps++;
						putInLocalPool(memory->localPool, threadId, chunk);
						block = getFromChunkUncontended(chunk);
						block->threadId = threadId;
						return block;
					}
					else {
						assert(globalHPStructure->topPointers[threadId] == 0);
						getDonorEntry(threadId)->noOfOps = 0;
						if (!donate(threadId, chunk)) {
							putInLocalPool(memory->localPool, threadId, chunk);
							block = getFromChunkUncontended(chunk);
							block->threadId = threadId;
							return block;
						}
						else {
							break;
						}
					}
				}
			} // while(true)
			printf("allocate: threadId = %d: calling allocate again\n", threadId);
			//return allocate(threadId);
		}
	}
}

//Chunk* doHelp(int threadToBeHelped, Chunk *stolenChunk, Helper *announceOfThreadToBeHelped) {
Chunk* doHelp(int threadId, int threadToBeHelped, Chunk *stolenChunk, ReferenceIntegerPair *announceOfThreadToBeHelped) {
	assert(globalHPStructure->topPointers[threadId] == 1);
	int i = 0;
	printf("doHelp: threadId: %d\n",threadId);
	//printf("doHelp: threadId: %d, current annTS: %d, old annTS: %d\n",threadId,getHelperEntry(threadToBeHelped)->atomicRef->integer,announceOfThreadToBeHelped->atomicRef->integer);
	//if (getHelperEntry(threadToBeHelped)->timestamp != announceOfThreadToBeHelped->timestamp)
	if (getHelperEntry(threadToBeHelped)->atomicRef->integer != announceOfThreadToBeHelped->integer) {
		printf("doHelp: threadId %d,somebody already helped threadToBeHelped = %d\n",threadId, threadToBeHelped);
		clearHazardPointer(globalHPStructure, threadId);
		return stolenChunk;
	}

	//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
	//AtomicStampedReference* oldTop = getStackThread(memory->fullPool, threadToBeHelped)->stack->top;
	if (stolenChunk == NULL) {
		printf("doHelp: threadId %d, stolenChunk is null\n", threadId);
		//printf("doHelp: threadId %d, top reference = %u\n", threadId, getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference);
		//printf("doHelp: threadId %d, current annTS = %d, old annTS = %d\n",threadId, getHelperEntry(threadToBeHelped)->atomicRef->integer, announceOfThreadToBeHelped->atomicRef->integer);
		assert(globalHPStructure->topPointers[threadId] == 1);
		while ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
			printf("doHelp: threadId %d, inside while\n",threadId);
			stolenChunk = getFromOtherFullPool(memory->fullPool, i, threadId);
			printf("doHelp: victim = %d\n",i);
			if (stolenChunk != NULL) {
				//printf("doHelp: dtealAttempt successful\n");
				break;
			}
			//printf("doHelp: dtealAttempt failed\n");
			i = (i + 1) % memory->n;
			if (i == 0) {
				assert(globalHPStructure->topPointers[threadId] == 1);
				stolenChunk = moveFromSharedQueuePools(threadId);
				printf("doHelp: treadId = %d, topPtr value = %d\n", threadId, globalHPStructure->topPointers[threadId]);
				assert(globalHPStructure->topPointers[threadId] == 1);
				//printf("allocate: threadID : %d, returned from moveFreomSQP with stolenChunk %u\n", threadId, stolenChunk);
				if (stolenChunk != NULL) {
					break;
				}
				printf("allocate: threadID : %d, returned from moveFreomSQP\n", threadId);
			}
		}
	}
	assert(globalHPStructure->topPointers[threadId] == 1);
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef);
	printf("doHelp: setting HP of thread %d for oldTop %u\n", threadId, oldTop);
	assert(globalHPStructure->topPointers[threadId] == 2);
	bool *tempBoolObj;
	if ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
		assert(globalHPStructure->topPointers[threadId] == 2);
		if (putInOtherFullPool(memory->fullPool, threadToBeHelped, stolenChunk, oldTop, threadId)) {
			assert(globalHPStructure->topPointers[threadId] == 1);
			tempBoolObj = (bool*)my_malloc(sizeof(bool));
			*tempBoolObj = false;
			if (!compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
				printf("doHelp: thread = %d, removing HP from annouce\n", threadId);
				my_free(tempBoolObj);
			}
			return NULL;
		}
	}
	else {
		assert(globalHPStructure->topPointers[threadId] == 2);
		clearHazardPointer(globalHPStructure, threadId);
		assert(globalHPStructure->topPointers[threadId] == 1);
		printf("doHelp: clearing HP of top thread %d when someone else helped\n", threadId);
	}
	tempBoolObj = (bool*)my_malloc(sizeof(bool));
	*tempBoolObj = false;
	if (!compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
		my_free(tempBoolObj);
	}
	assert(globalHPStructure->topPointers[threadId] == 0);
	return stolenChunk;
}

void freeMem(int threadId, Block *block) {
	assert(globalHPStructure->topPointers[threadId] == 0);
	//printf("freeMem: threadID = %d, entered freeMem\n", threadId);
	Chunk *chunk;
	if (block->threadId != threadId) {
		//printf("freeMem: threadid = %d, freeing block %d\n", threadId, block->memBlock);
		putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
		return;
	}
	chunk = getFromLocalPool(memory->localPool,threadId);
	//printf("freeMem: threadID = %d, chunk ptr = %u\n", threadId, chunk);
	if (chunkHasSpace(chunk)) {
		//printf("freeMem: threadId = %d: chunk in localPool has space\n", threadId);
		putInChunkUncontended(chunk,block);
		putInLocalPool(memory->localPool,threadId, chunk);
		//printf("freeMem: threadID = %d, chunk ptr = %u\n", threadId, chunk);
		//putInLocalPool(memory->localPool,threadId,chunk);
		return;
	}
	else {
		//printf("freeMem: threadID = %d, chunk in localPool doesn't have space\n", threadId);
		while(true) {
			if (getDonorEntry(threadId)->noOfOps == memory->C) { // have to donate
				getDonorEntry(threadId)->noOfOps = 0;
				if(donate(threadId, chunk)) { // donation successful
					chunk = getFromFreePoolUC(memory->freePoolUC, threadId); // get a chunk from freePoolUC
					if (chunk == NULL) { // not multiples of c were passed to other threads
						putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
						return;
					}
					else { // freePoolUC had chunks
						putInChunkUncontended(chunk, block);
						putInLocalPool(memory->localPool, threadId, chunk);
						return;
					}
				} // donation unsuccessful
				else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) { //move full chunk to fullPool
					chunk = getFromFreePoolUC(memory->freePoolUC, threadId);
					if (chunk == NULL) { // not multiples of c were passed to other threads
						putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
						return;
					}
					else {
						putInChunkUncontended(chunk, block);
						putInLocalPool(memory->localPool,threadId, chunk);
						return;
					}
				}
			} // donation is not needed
			else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) {
				getDonorEntry(threadId)->noOfOps++;
				chunk = getFromFreePoolUC(memory->freePoolUC, threadId);
				if (chunk == NULL) { // not multiples of c were passed to other threads
					putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
					return;
				}
				else {
					putInChunkUncontended(chunk, block);
					putInLocalPool(memory->localPool,threadId, chunk);
					return;
				}
			}
		}
	}
	assert(globalHPStructure->topPointers[threadId] == 0);
}

bool donate(int threadId, Chunk *chunk) {
	printf("donate: threadID %d\n", threadId);
	int i = (getDonorEntry(threadId)->lastDonated + 1) % (memory->n);
	bool *tempBoolObj;
	assert(globalHPStructure->topPointers[threadId] == 0);
	do {
		printf("donate: threadID %d, trying to donate to %d\n", threadId, i);
		ReferenceIntegerPair *announceOfThreadToBeHelped = setHazardPointer(globalHPStructure, threadId, getHelperEntry(i)->atomicRef);
		ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, getStackThread(memory->fullPool, i)->stack->top->atomicRef);
		assert(globalHPStructure->topPointers[threadId] == 2);
		printf("donate: setting HP of thread %d for oldTop %u\n", threadId, oldTop);
		//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
		//AtomicStampedReference* oldTop = getStackThread(memory->fullPool, i)->stack->top;
		//printf("donate: threadId = %d, oldTop.reference= %u, oldTop.TS = %d\n", threadId, oldTop->atomicRef->reference,oldTop->atomicRef->integer);
		//int oldTS = announceOfThreadToBeHelped->timestamp;
		int oldTS = announceOfThreadToBeHelped->integer;
		//	if (*(bool*)announceOfThreadToBeHelped->atomicRef->reference == true)
		if ((*(bool*)announceOfThreadToBeHelped->reference == true) && (getStackThread(memory->fullPool, i)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(i)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
			printf("donate: threadID %d, %d needed help\n", threadId, i);
			if (putInOtherFullPool(memory->fullPool, i, chunk, oldTop, threadId)) {
				//getHelperEntry(i)->compareAndSet(...);
				assert(globalHPStructure->topPointers[threadId] == 1);
				printf("donate: threadID %d, successfully donated to %d\n", threadId, i);
				tempBoolObj = (bool*)my_malloc(sizeof(bool));
				*tempBoolObj = false;
				if(!compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
					//clearHazardPointer(globalHPStructure, threadId);
					my_free(tempBoolObj);
				}
				assert(globalHPStructure->topPointers[threadId] == 0);
				getDonorEntry(threadId)->lastDonated = i;
				return true;
			}
			printf("donate: threadID %d, donation to %d failed: someone else helped\n", threadId, i);
			//getHelperEntry(i)->compareAndSet(...);
			tempBoolObj = (bool*)my_malloc(sizeof(bool));
			*tempBoolObj = false;
			if (!compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
				//clearHazardPointer(globalHPStructure, threadId);
				my_free(tempBoolObj);
			}
			assert(globalHPStructure->topPointers[threadId] == 0);
		}
		else {
			clearHazardPointer(globalHPStructure, threadId);
			clearHazardPointer(globalHPStructure, threadId);
			printf("donate: clearing HP of thread %d donation was not needed\n", threadId);
		}
		i = (i + 1) % (memory->n);
		assert(globalHPStructure->topPointers[threadId] == 0);
	} while(i != (getDonorEntry(threadId)->lastDonated + 1) % (memory->n));
	printf("returning from donate\n");
	return false;
}

Chunk* moveFromSharedQueuePools(int threadId) {
	assert(globalHPStructure->topPointers[threadId] == 1);
	printf("moveFromSQP: threadID: %d\n", threadId);
	int primThread = 0, secThread = 0;
	Block *block;
	Chunk* chunk;
	QueueElement *oldQueueHead;
	for (primThread = 0; primThread < memory->n; primThread++) {
		for (secThread = 0; secThread < memory->n; secThread++) {
			assert(globalHPStructure->topPointers[threadId] == 1);
			printf("moveFromSQP: threadID: %d primThread: %d, secThread: %d\n", threadId, primThread, secThread);
			block = getFromSharedQueuePools(memory->sharedQueuePools, threadId, primThread, secThread);
			assert(globalHPStructure->topPointers[threadId] == 1);
			//printf("moveFromSQP: threadID: %d block ptr : %u\n", threadId, block);
			if (block != NULL) {
				printf("block was not null\n");
				//printf("moveFromSQP: queuePtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue);
				//printf("moveFromSQP: HeadPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head);
				//printf("moveFromSQP: nextPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next);
				//printf("moveFromSQP: chunkPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next->value);
				oldQueueHead = setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				assert(globalHPStructure->topPointers[threadId] == 2);
				printf("moveFromSQP: setting HP of thread %d for oldQueueHEad %u\n", threadId, oldQueueHead);
				//getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					if (chunkHasSpace(oldQueueHead->next->value)) {
						if (putInChunkContended(getQueueThread(memory->freePoolC, primThread)->queue->head->next->value, block)) {
							clearHazardPointer(globalHPStructure, threadId);
							assert(globalHPStructure->topPointers[threadId] == 1);
							printf("moveFromSQP: clearing HP of thread %d block was inserted in chunk\n", threadId);
							continue; // now go to next secThread
						}
						else {
							// do sth with the removed block
							clearHazardPointer(globalHPStructure, threadId);
							printf("moveFromSQP: clearing HP of thread %d block was not inserted in chunk\n", threadId);
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							assert(globalHPStructure->topPointers[threadId] == 1);
						}
					}
					else { //chunk doesn't have space.try moving the chunk to fullPool
						assert(globalHPStructure->topPointers[threadId] == 2);
						chunk = getFromFreePoolC(memory->freePoolC, threadId, primThread, oldQueueHead);
						assert(globalHPStructure->topPointers[threadId] == 1);
						if (chunk != NULL) {
							printf("moveFromSQP: moving block %d to SQP %d\n",block->memBlock, threadId);
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							return chunk;
						}
						else {
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							// do sth with the removed block
						}
						assert(globalHPStructure->topPointers[threadId] == 1);
					}
				}
				else {
					clearHazardPointer(globalHPStructure, threadId);
					printf("moveFromSQP: clearing HP of thread %d queue wasn't empty\n", threadId);
					assert(globalHPStructure->topPointers[threadId] == 1);
					putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
				}
			}
			else {
				//clearHazardPointer(globalHPStructure, threadId);
				printf("moveFromSQP: threadID = %d threadblock was null\n", threadId);
				assert(globalHPStructure->topPointers[threadId] == 1);
				//printf("moveFromSQP: queuePtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue);
				//printf("moveFromSQP: HeadPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head);
				//printf("moveFromSQP: nextPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next);
				//printf("moveFromSQP: chunkPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next->value);
				oldQueueHead = setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				printf("moveFromSQP: setting HP of thread %d for oldQueuHEad %u\n", threadId, oldQueueHead);
				//oldQueueHead = getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					if (!chunkHasSpace(oldQueueHead->next->value)) {
						printf("moveFromSMP: threadId = %d block was null but chunk was full\n", threadId);
						chunk = getFromFreePoolC(memory->freePoolC, threadId, primThread, oldQueueHead);
						assert(globalHPStructure->topPointers[threadId] == 1);
						if (chunk != NULL) {
							return chunk;
						}
					}
					else {
						clearHazardPointer(globalHPStructure, threadId);
						printf("moveFromSQP: clearing HP of thread %d block block was null\n", threadId);
					}
				}
				else {
					clearHazardPointer(globalHPStructure, threadId);
					printf("moveFromSQP: clearing HP of thread %d block block was nnnnull\n", threadId);
				}
				assert(globalHPStructure->topPointers[threadId] == 1);
			}
			printf("moveFromSQP: threadID: %d came here\n", threadId);
			assert(globalHPStructure->topPointers[threadId] == 1);
		}
	}
	assert(globalHPStructure->topPointers[threadId] == 1);
	return NULL;
}
