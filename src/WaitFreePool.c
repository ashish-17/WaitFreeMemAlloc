#include "WaitFreePool.h"

typedef struct {
	bool needHelp;
	int timestamp;
} Helper;

typedef struct{
	Helper *helpers;
	int numOfHelpers;
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

Chunk* doHelp(int threadToBeHelped, Chunk *stolenChunk, Helper *announceOfThreadToBeHelped);
bool donate(int threadId, Chunk *chunk);

Helper* getHelperEntry(int index) {
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

	memory->fullPool = createFullPool(numOfThreads);
	memory->localPool = createLocalPool(numOfThreads);
	memory->freePool = createFreePool(numOfThreads);
	memory->m = m; memory->n = n; memory->c = c; memory->C = C;

	Chunk *chunk;
	Block *block;

	// Set-up of initial local pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(j*numOfThreads + i*numOfChunksPerThread + k);
				putInChunk(chunk, block);
			}
			putInLocalPool(memory->localPool,j,chunk);
		}
	}

	// Set-up of initial full pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < numOfChunksPerThread - 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(j*numOfThreads + i*numOfChunksPerThread + k);
				putInChunk(chunk, block);
			}
			putInOwnFullPool(memory->fullPool,j,chunk);
		}
	}

	// Set up initial announce array
	memory->announce = (Announce*)malloc(sizeof(Announce));
	memory->announce->helpers = (Helper*) malloc(sizeof(Helper)*numOfThreads);
	memory->announce->numOfHelpers = numOfThreads;
	for(int i = 0; i < numOfThreads; i++) {
		Helper* helperEntry = getHelperEntry(memory->announce,i);
		helperEntry->needHelp = false;
		helperEntry->timestamp = 0;
	}

	// Set up initial info array
	memory->info = (Info*)malloc(sizeof(Info));
	memory->info->donors = (Donor*) malloc(sizeof(Donor)*numOfThreads);
	memory->info->numOfDonors = numOfThreads;
	for(int i = 0; i < numOfThreads; i++) {
		Donor* donorEntry = getDonorEntry(memory->info,i);
		donorEntry->lastDonated = 0;
		donorEntry->noOfOps = 0;
	}
}

Block* allocate(int threadId) {

	Chunk *stolenChunk;
	int threadToBeHelped;
	Helper *announceOfThreadToBeHelped;

	Chunk* chunk = getFromLocalPool(memory->localPool, threadId);
	if (!isChunkEmpty(chunk)) {
		Block *block = getFromChunk(chunk);
		putInLocalPool(memory->localPool, threadId, chunk);
		return block;
	}
	else {
		putInFreePool(memory->freePool,threadId,chunk);
		if (isFullPoolEmpty(memory->fullPool,threadId)) {
			getHelperEntry(threadId)->needHelp = true;
			stolenChunk = NULL;
			for(int i = 0; i < memory->n; i++) {
				threadToBeHelped = (threadId + i) % memory->n;
				announceOfThreadToBeHelped = getHelperEntry(threadToBeHelped);
				if(announceOfThreadToBeHelped->needHelp) {
					stolenChunk = doHelp(threadId, threadToBeHelped, stolenChunk, announceOfThreadToBeHelped);
				}
			}
			if (stolenChunk != NULL) {
				putInOwnFullPool(memory->fullPool, threadId, stolenChunk);
			}
		}

		while(true) {
			chunk = getFromOwnFullPool(memory->fullPool,threadId);
			if (chunk != NULL) {
				if (getDonorEntry(threadId)->noOfOps != C) {
					getDonorEntry(threadId)->noOfOps++;
					putInLocalPool(memory->localPool, threadId, chunk);
					return getFromChunk(chunk);
				}
			}
			else {
				getDonorEntry(threadId)->noOfOps = 0;
				if (!donate(threadId, chunk)) {
					putInLocalPool(memory->localPool, threadId, chunk);
					return getFromChunk(chunk);
				}
				else {
					break;
				}
			}
		}
	}
	allocate(threadId);
}

Chunk* doHelp(int threadToBeHelped, Chunk *stolenChunk, Helper *announceOfThreadToBeHelped) {

	int i = 0;

	if (getHelperEntry(threadToBeHelped)->timestamp != announceOfThreadToBeHelped->timestamp)
		return stolenChunk;

	AtomicStampedReference oldTop = getThread(memory->fullPool, threadToBeHelped)->stack->top;
	if (stolenChunk == NULL) {
		while ((getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->integer == announceOfThreadToBeHelped->timestamp)) {
			stolenChunk = getFromOtherFullPool(memory->fullPool, i);
			if (stolenChunk != NULL) {
				break;
			}
			i = (i + 1) % memory->n;
		}
	}

	if ((getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->integer == announceOfThreadToBeHelped->timestamp)) {
		if (putInOtherFullPool(memory->fullPool, threadToBeHelped, stolenChunk, oldTop)) {
			compareAndSet(....);
			return NULL;
		}
	}
	compareAndSet(...);
	return stolenChunk;
}

void freeMem(int threadId, Block *block) {
	Chunk *chunk;

	getFromLocalPool(memory->localPool,threadId);
	if (isChunkEmpty(chunk)) {
		putInChunk(chunk,block);
		putInLocalPool(memory->localPool,threadId,chunk);
		return;
	}
	else {
		while(true) {
			if (getDonorEntry(threadId)->noOfOps == memory->C) {
				getDonorEntry(threadId)->noOfOps = 0;
				if(donate(chunk)) {
					chunk = getFromFreePool(memory->freePool, threadId);
					putInChunk(chunk, block);
					putInLocalPool(memory->localPool, threadId, chunk);
				}
			}
			else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) {
				getDonorEntry(threadId)->noOfOps++;
				chunk = getFromFreePool(memory->freePool);
				putInChunk(chunk, block);
				putInLocalPool(memory->localPool,threadId, chunk);
			}
		}
	}
}

bool donate(int threadId, Chunk *chunk) {
	int i = (getDonorEntry(threadId)->lastDonated + 1) % memory->n;
	do {
		Helper *announceOfThreadToBeHelped = getHelperEntry(i);
		AtomicStampedReference oldTop = getThread(memory->fullPool, i)->stack->top;
		int oldTS = announceOfThreadToBeHelped->timestamp;
		if (announceOfThreadToBeHelped->needHelp == true) {
			if (putInOtherFullPool(memory->fullPool, i, chunk, oldTop)) {
				getHelperEntry(i)->compareAndSet(...);
				getDonorEntry(threadId)->lastDonated = i;
				return true;
			}
			getHelperEntry(i)->compareAndSet(...);
		}
		i = (i + 1) % memory->n;
	} while(i != (getDonorEntry(threadId)->lastDonated + 1) % memory->n);
	return false;
}

