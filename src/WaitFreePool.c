#include "WaitFreePool.h"

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
} Donor;

typedef struct{
	Donor *donors;
	int numOfDonors;
} Info;

typedef struct _Memory {
	Pool* fullPool;
	Pool* localPool;
	Pool* freePool;
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
Block* allocate(int threadId);
void freeMem(int threadId, Block *block);

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
	memory->freePool = createFreePool(numOfThreads);

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
				block = createBlock(blockNumber);
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
				block = createBlock(blockNumber);
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
	for(int i = 0; i < numOfThreads; i++) {
		Donor* donorEntry = getDonorEntry(i);
		donorEntry->lastDonated = i;
		donorEntry->noOfOps = 0;
	}

	//memory->n = 2;
}



Block* allocate(int threadId) {

	Chunk *stolenChunk;
	int threadToBeHelped;
	//Helper *announceOfThreadToBeHelped;
	AtomicStampedReference *announceOfThreadToBeHelped;
	//printf("allocate: threadID = %d\n", threadId);
	Chunk* chunk = getFromLocalPool(memory->localPool, threadId);
	//printf("allocate: threadID = %d, chunk ptr = %u\n", threadId, chunk);
	if (!isChunkEmpty(chunk)) {
		Block *block = getFromChunkUncontended(chunk);
		putInLocalPool(memory->localPool, threadId, chunk);
		return block;
	}
	else {
		//printf("allocate: threadId = %d: chunk in localPool is empty\n", threadId);
		putInFreePool(memory->freePool,threadId,chunk);
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
						return getFromChunkUncontended(chunk);
					}
					else {
						getDonorEntry(threadId)->noOfOps = 0;
						if (!donate(threadId, chunk)) {
							putInLocalPool(memory->localPool, threadId, chunk);
							return getFromChunkUncontended(chunk);
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
	printf("doHelp: threadId: %d, current annTS: %d, old annTS: %d\n",threadId,getHelperEntry(threadToBeHelped)->atomicRef->integer,announceOfThreadToBeHelped->atomicRef->integer);
	//if (getHelperEntry(threadToBeHelped)->timestamp != announceOfThreadToBeHelped->timestamp)
	if (getHelperEntry(threadToBeHelped)->atomicRef->integer != announceOfThreadToBeHelped->atomicRef->integer) {
		printf("doHelp: threadId %d,somebody already helped threadToBeHelped = %d\n",threadId, threadToBeHelped);
		return stolenChunk;
	}

	AtomicStampedReference* oldTop = getThread(memory->fullPool, threadToBeHelped)->stack->top;
	if (stolenChunk == NULL) {
		//printf("doHelp: threadId %d, stolenChunk is null\n",threadId);
		//printf("doHelp: threadId %d, top reference = %u\n",threadId,getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference);
		//printf("doHelp: threadId %d, current annTS = %d, old annTS = %d\n",threadId, getHelperEntry(threadToBeHelped)->atomicRef->integer, announceOfThreadToBeHelped->atomicRef->integer);
		while ((getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->atomicRef->integer)) {
			printf("doHelp: threadId %d, inside while\n",threadId);
			stolenChunk = getFromOtherFullPool(memory->fullPool, i);
			//printf("doHelp: victim = %d\n",i);
			if (stolenChunk != NULL) {
				//printf("doHelp: dtealAttempt successful\n");
				break;
			}
			//printf("doHelp: dtealAttempt failed\n");
			i = (i + 1) % memory->n;
		}
	}

	bool *tempBoolObj;
	if ((getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->atomicRef->integer)) {
		if (putInOtherFullPool(memory->fullPool, threadToBeHelped, stolenChunk, oldTop)) {
			tempBoolObj = (bool*)malloc(sizeof(bool));
			*tempBoolObj = false;
			compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1));
			return NULL;
		}
	}
	tempBoolObj = (bool*)malloc(sizeof(bool));
	*tempBoolObj = false;
	compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1));
	return stolenChunk;
}

void freeMem(int threadId, Block *block) {

	//printf("freeMem: threadID = %d, entered freeMem\n", threadId);
	Chunk *chunk = getFromLocalPool(memory->localPool,threadId);
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
			if (getDonorEntry(threadId)->noOfOps == memory->C) {
				getDonorEntry(threadId)->noOfOps = 0;
				if(donate(threadId, chunk)) {
					chunk = getFromFreePool(memory->freePool, threadId);
					putInChunkUncontended(chunk, block);
					putInLocalPool(memory->localPool, threadId, chunk);
					return;
				}
				else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) {
					chunk = getFromFreePool(memory->freePool, threadId);
					putInChunkUncontended(chunk, block);
					putInLocalPool(memory->localPool,threadId, chunk);
					return;
				}
			}
			else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) {
				getDonorEntry(threadId)->noOfOps++;
				chunk = getFromFreePool(memory->freePool, threadId);
				putInChunkUncontended(chunk, block);
				putInLocalPool(memory->localPool,threadId, chunk);
				return;
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
		AtomicStampedReference* oldTop = getThread(memory->fullPool, i)->stack->top;
		printf("donate: threadId = %d, oldTop.reference= %u, oldTop.TS = %d\n", threadId, oldTop->atomicRef->reference,oldTop->atomicRef->integer);
		//int oldTS = announceOfThreadToBeHelped->timestamp;
		int oldTS = announceOfThreadToBeHelped->atomicRef->integer;
		if (*(bool*)announceOfThreadToBeHelped->atomicRef->reference == true) {
			printf("donate: threadID %d, %d needed help\n", threadId, i);
			if (putInOtherFullPool(memory->fullPool, i, chunk, oldTop)) {
				//getHelperEntry(i)->compareAndSet(...);
				printf("donate: threadID %d, successfully donated to %d\n", threadId, i);
				tempBoolObj = (bool*)malloc(sizeof(bool));
				*tempBoolObj = false;
				compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1));
				getDonorEntry(threadId)->lastDonated = i;
				return true;
			}
			printf("donate: threadID %d, donation to %d failed: someone else helped\n", threadId, i);
			//getHelperEntry(i)->compareAndSet(...);
			tempBoolObj = (bool*)malloc(sizeof(bool));
			*tempBoolObj = false;
			compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->atomicRef->reference, tempBoolObj, announceOfThreadToBeHelped->atomicRef->integer, (announceOfThreadToBeHelped->atomicRef->integer + 1));
		}
		i = (i + 1) % (memory->n);
	} while(i != (getDonorEntry(threadId)->lastDonated + 1) % (memory->n));
	return false;
}

