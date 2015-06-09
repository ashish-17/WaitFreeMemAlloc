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

/*typedef struct {
	bool needHelp;
	int timestamp;
} Helper;

typedef struct{
	Helper *helpers;
	int numOfHelpers;
} Announce;*/

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
Chunk* doHelp(int threadId, int threadToBeHelped, Chunk *stolenChunk, AtomicStampedReference *announceOfThreadToBeHelped);
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

	memory = (Memory*)malloc(sizeof(Memory));

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
	memory->sharedQueuePools = createSharedQueuePools(numOfThreads);

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

	// Set-up of initial full pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < numOfChunksPerThread - 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(-1, blockNumber);
				blockNumber++;
				putInChunkUncontended(chunk, block);
			}
			putInOwnFullPool(memory->fullPool,j,chunk);
		}
	}

	/*// Set up initial announce array
	memory->announce = (Announce*)malloc(sizeof(Announce));
	memory->announce->helpers = (Helper*) malloc(sizeof(Helper)*numOfThreads);
	memory->announce->numOfHelpers = numOfThreads;
	for(int i = 0; i < numOfThreads; i++) {
		Helper* helperEntry = getHelperEntry(i);
		helperEntry->needHelp = false;
		helperEntry->timestamp = 0;
	}
	 */

	//numOfThreads = n + 1;
	memory->announce = (Announce*)malloc(sizeof(Announce));
	memory->announce->helpers = (AtomicStampedReference*) malloc(sizeof(AtomicStampedReference)*numOfThreads);
	for(int i = 0; i < numOfThreads; i++) {
		AtomicStampedReference* helperEntry = getHelperEntry(i);
		bool* tempBoolObj = (bool*)malloc(sizeof(bool));
		*tempBoolObj = false;
		createAtomicStampedReference(helperEntry,tempBoolObj,0);
	}

	// Set up initial info array
	memory->info = (Info*)malloc(sizeof(Info));
	memory->info->donors = (Donor*) malloc(sizeof(Donor)*numOfThreads);
	memory->info->numOfDonors = numOfThreads;
	//memory->info->numOfPassed = 0;
	for(int i = 0; i < numOfThreads; i++) {
		Donor* donorEntry = getDonorEntry(i);
		donorEntry->lastDonated = i;
		donorEntry->noOfOps = 0;
		donorEntry->numOfPassed = 0;
		donorEntry->addInFreePoolC = false;
	}

	//memory->n = 2;
}



Block* allocate(int threadId, bool toBePassed) {

	Chunk *stolenChunk;
	Block *block;
	int threadToBeHelped;
	bool addInFreePoolC = false;
	Donor *donor;

	//Helper *announceOfThreadToBeHelped;
	AtomicStampedReference *announceOfThreadToBeHelped;
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
					announceOfThreadToBeHelped = getHelperEntry(threadToBeHelped);
					//if(announceOfThreadToBeHelped->needHelp) {
					if(*(bool*)announceOfThreadToBeHelped->atomicRef->reference) {
						printf("allocate: threadId = %d: going to help thread %d\n", threadId, threadToBeHelped);
						stolenChunk = doHelp(threadId, threadToBeHelped, stolenChunk, announceOfThreadToBeHelped);
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
				if (chunk != NULL) {
					if (getDonorEntry(threadId)->noOfOps != memory->C) {
						//printf("allocate: threadId = %d: noOfOps = %d\n", threadId,getDonorEntry(threadId)->noOfOps);
						getDonorEntry(threadId)->noOfOps++;
						putInLocalPool(memory->localPool, threadId, chunk);
						block = getFromChunkUncontended(chunk);
						block->threadId = threadId;
						return block;
					}
					else {
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
Chunk* doHelp(int threadId, int threadToBeHelped, Chunk *stolenChunk, AtomicStampedReference *announceOfThreadToBeHelped) {

	int i = 0;
	printf("doHelp: threadId: %d\n",threadId);
	//printf("doHelp: threadId: %d, current annTS: %d, old annTS: %d\n",threadId,getHelperEntry(threadToBeHelped)->atomicRef->integer,announceOfThreadToBeHelped->atomicRef->integer);
	//if (getHelperEntry(threadToBeHelped)->timestamp != announceOfThreadToBeHelped->timestamp)
	if (getHelperEntry(threadToBeHelped)->atomicRef->integer != announceOfThreadToBeHelped->atomicRef->integer) {
		printf("doHelp: threadId %d,somebody already helped threadToBeHelped = %d\n",threadId, threadToBeHelped);
		return stolenChunk;
	}

	AtomicStampedReference* oldTop = getStackThread(memory->fullPool, threadToBeHelped)->stack->top;
	if (stolenChunk == NULL) {
		//printf("doHelp: threadId %d, stolenChunk is null\n",threadId);
		//printf("doHelp: threadId %d, top reference = %u\n",threadId,getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference);
		//printf("doHelp: threadId %d, current annTS = %d, old annTS = %d\n",threadId, getHelperEntry(threadToBeHelped)->atomicRef->integer, announceOfThreadToBeHelped->atomicRef->integer);
		while ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->atomicRef->integer)) {
			//printf("doHelp: threadId %d, inside while\n",threadId);
			stolenChunk = getFromOtherFullPool(memory->fullPool, i);
			//printf("doHelp: victim = %d\n",i);
			if (stolenChunk != NULL) {
				//printf("doHelp: dtealAttempt successful\n");
				break;
			}
			//printf("doHelp: dtealAttempt failed\n");
			i = (i + 1) % memory->n;
			if (i == 0) {
				stolenChunk = moveFromSharedQueuePools(threadId);
				//printf("allocate: threadID : %d, returned from moveFreomSQP with stolenChunk %u\n", threadId, stolenChunk);
				if (stolenChunk != NULL) {
					break;
				}
				//printf("allocate: threadID : %d, returned from moveFreomSQP\n", threadId);
			}
		}
	}

	bool *tempBoolObj;
	if ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->atomicRef->integer)) {
		if (putInOtherFullPool(memory->fullPool, threadToBeHelped, stolenChunk, oldTop)) {
			tempBoolObj = (bool*)malloc(sizeof(bool));
			*tempBoolObj = false;
			compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1), threadId);
			return NULL;
		}
	}
	tempBoolObj = (bool*)malloc(sizeof(bool));
	*tempBoolObj = false;
	compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1), threadId);
	return stolenChunk;
}

void freeMem(int threadId, Block *block) {

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
}

bool donate(int threadId, Chunk *chunk) {
	printf("donate: threadID %d\n", threadId);
	int i = (getDonorEntry(threadId)->lastDonated + 1) % (memory->n);
	bool *tempBoolObj;

	do {
		printf("donate: threadID %d, trying to donate to %d\n", threadId, i);
		//Helper *announceOfThreadToBeHelped = getHelperEntry(i);
		AtomicStampedReference *announceOfThreadToBeHelped = getHelperEntry(i);
		AtomicStampedReference* oldTop = getStackThread(memory->fullPool, i)->stack->top;
		//printf("donate: threadId = %d, oldTop.reference= %u, oldTop.TS = %d\n", threadId, oldTop->atomicRef->reference,oldTop->atomicRef->integer);
		//int oldTS = announceOfThreadToBeHelped->timestamp;
		int oldTS = announceOfThreadToBeHelped->atomicRef->integer;
		//	if (*(bool*)announceOfThreadToBeHelped->atomicRef->reference == true)
		if ((*(bool*)announceOfThreadToBeHelped->atomicRef->reference == true) && (getStackThread(memory->fullPool, i)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(i)->atomicRef->integer == announceOfThreadToBeHelped->atomicRef->integer)) {
			printf("donate: threadID %d, %d needed help\n", threadId, i);
			if (putInOtherFullPool(memory->fullPool, i, chunk, oldTop)) {
				//getHelperEntry(i)->compareAndSet(...);
				printf("donate: threadID %d, successfully donated to %d\n", threadId, i);
				tempBoolObj = (bool*)malloc(sizeof(bool));
				*tempBoolObj = false;
				compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1), threadId);
				getDonorEntry(threadId)->lastDonated = i;
				return true;
			}
			printf("donate: threadID %d, donation to %d failed: someone else helped\n", threadId, i);
			//getHelperEntry(i)->compareAndSet(...);
			tempBoolObj = (bool*)malloc(sizeof(bool));
			*tempBoolObj = false;
			compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1), threadId);
		}
		i = (i + 1) % (memory->n);
	} while(i != (getDonorEntry(threadId)->lastDonated + 1) % (memory->n));
	return false;
}

Chunk* moveFromSharedQueuePools(int threadId) {
	//printf("moveFromSQP: threadID: %d\n", threadId);
	int primThread = 0, secThread = 0;
	Block *block;
	Chunk* chunk;
	QueueElement *oldQueueHead;
	for (primThread = 0; primThread < memory->n; primThread++) {
		for (secThread = 0; secThread < memory->n; secThread++) {
			//printf("moveFromSQP: threadID: %d primThread: %d, secThread: %d\n", threadId, primThread, secThread);
			block = getFromSharedQueuePools(memory->sharedQueuePools, primThread, secThread);
			//printf("moveFromSQP: threadID: %d block ptr : %u\n", threadId, block);
			if (block != NULL) {
				//printf("block was not null\n");
				//printf("moveFromSQP: queuePtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue);
				//printf("moveFromSQP: HeadPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head);
				//printf("moveFromSQP: nextPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next);
				//printf("moveFromSQP: chunkPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next->value);
				setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				oldQueueHead = getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					if (chunkHasSpace(oldQueueHead->next->value)) {
						if (putInChunkContended(getQueueThread(memory->freePoolC, primThread)->queue->head->next->value, block)) {
							continue; // now go to next secThread
						}
						else {
							// do sth with the removed block
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
						}
					}
					else { //chunk doesn't have space.try moving the chunk to fullPool
						chunk = getFromFreePoolC(memory->freePoolC, primThread, oldQueueHead);
						if (chunk != NULL) {
							printf("moveFromSQP: moving block %d to SQP %d\n",block->memBlock, threadId);
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							return chunk;
						}
						else {
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							// do sth with the removed block
						}
					}
				}
				else {
					putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
				}
			}
			else {
				//printf("block was null\n");
				//printf("moveFromSQP: queuePtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue);
				//printf("moveFromSQP: HeadPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head);
				//printf("moveFromSQP: nextPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next);
				//printf("moveFromSQP: chunkPtr = %u\n", getQueueThread(memory->freePoolC, primThread)->queue->head->next->value);
				setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				oldQueueHead = getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					if (!chunkHasSpace(oldQueueHead->next->value)) {
						//printf("moveFromSMP: threadId = %d block was null but chunk was full\n", threadId);
						chunk = getFromFreePoolC(memory->freePoolC, primThread, oldQueueHead);
						if (chunk != NULL) {
							return chunk;
						}
					}
				}
			}
			//printf("moveFromSQP: threadID: %d came here\n", threadId);
		}
	}
	return NULL;
}
