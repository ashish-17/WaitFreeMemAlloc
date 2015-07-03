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
	LOG_PROLOG();
	AtomicStampedReference* ptr = (memory->announce->helpers + index);
	LOG_EPILOG();
	return ptr;
}

Donor* getDonorEntry(int index) {
	LOG_PROLOG();
	Donor *ptr = (memory->info->donors + index);
	LOG_EPILOG();
	return ptr;
}

void createWaitFreePool(int m, int n, int c, int C) {
	LOG_PROLOG();
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
	LOG_INFO("created freePoolC\n");
	memory->sharedQueuePools = createSharedQueuePools(numOfThreads);
	LOG_INFO("created SQP\n");
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
	//LOG_INFO("set up localPool with chunk\n");

	// Set-up of initial full pools
	for(int j = 0; j < numOfThreads ; j++) {
		for(int i = 0; i < numOfChunksPerThread - 1; i++) {
			chunk = createChunk(chunk,numOfBlocksPerChunk);
			for(int k = 0; k < numOfBlocksPerChunk; k++) {
				block = createBlock(-1, blockNumber);
				blockNumber++;
				putInChunkUncontended(chunk, block);
			}
			//LOG_INFO("created the chunk for FP\n");
			putInOwnFullPool(memory->fullPool,j,chunk);
			//LOG_INFO("fullPoolEmpty[%d] %u %u\n",j, isFullPoolEmpty(memory->fullPool,j), flag);
		}
	}
	//LOG_INFO("set up FullPools with chunks\n");

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
	for(int i = 0; i < numOfThreads; i++) {
		Donor* donorEntry = getDonorEntry(i);
		donorEntry->lastDonated = i;
		donorEntry->noOfOps = 0;
		donorEntry->numOfPassed = 0;
		donorEntry->addInFreePoolC = false;
	}
	LOG_EPILOG();
}

void destroyWaitFreePool() {
	LOG_PROLOG();
	if (memory != NULL) {
		destroyFullPool(memory->fullPool);
		LOG_INFO("FullPool successfully destroyed");
		destroyFreePoolUC(memory->freePoolUC);
		LOG_INFO("FreePoolUC successfully destroyed");
		destroyFreePoolC(memory->freePoolC);
		LOG_INFO("FreePoolC successfully destroyed");
		destroyLocalPool(memory->localPool);
		LOG_INFO("LocalPool successfully destroyed");
		destroySharedQueuePools(memory->sharedQueuePools);
		LOG_INFO("SharedQueuePools successfully destroyed");

		if (memory->announce != NULL) {
			if (memory->announce->helpers != NULL) {
				for (int i = 0; i < memory->n; i++) {
					freeAtomicStampedReference(getHelperEntry(i));
				}
				my_free(memory->announce->helpers);
				memory->announce->helpers = NULL;
			}
			else {
				LOG_ERROR("Trying to free helper pointer of announce array which is a NULL pointer");
			}
			my_free(memory->announce);
			memory->announce = NULL;
		}
		else {
			LOG_ERROR("Trying to free announce array which is a NULL pointer");
		}
		LOG_INFO("Announce array successfully destroyed");

		if (memory->info != NULL) {
			if (memory->info->donors != NULL) {
				my_free(memory->info->donors);
				memory->info->donors = NULL;
			}
			else {
				LOG_ERROR("Trying to free donor pointer of info array which is a NULL pointer");
			}
			memory->info->numOfDonors = 0;
			my_free(memory->info);
			memory->info = NULL;
		}
		else {
			LOG_ERROR("Trying to free info array which is a NULL pointer");
		}
		LOG_INFO("Info array successfully destroyed");

		my_free(memory);
		memory = NULL;
	}
	else {
		LOG_ERROR("Trying to free memory pointer which is a NULL pointer");
	}

	LOG_EPILOG();
}

Block* allocate(int threadId, bool toBePassed) {
	LOG_PROLOG();
	Chunk *stolenChunk;
	Block *block = NULL;
	int threadToBeHelped;
	bool addInFreePoolC = false;
	Donor *donor;

	//Helper *announceOfThreadToBeHelped;
	ReferenceIntegerPair *announceOfThreadToBeHelped;
	//LOG_INFO("allocate: threadID = %d\n", threadId);
	if (toBePassed) {
		donor = getDonorEntry(threadId);
		donor->numOfPassed++;
		if (donor->numOfPassed %  memory->c == 1) {
			LOG_INFO("allocate: setting addInFreePoolC true");
			donor->addInFreePoolC = true;
		}
	}
	Chunk* chunk = getFromLocalPool(memory->localPool, threadId);
	//LOG_INFO("allocate: threadID = %d, chunk ptr = %u\n", threadId, chunk);
	if (!isChunkEmpty(chunk)) {
		block = getFromChunkUncontended(chunk);
		block->threadId = threadId;
		putInLocalPool(memory->localPool, threadId, chunk);
		LOG_EPILOG();
		return block;
	}
	else {
		LOG_INFO("allocate: chunk in localPool is empty");
		donor = getDonorEntry(threadId);
		if (donor->addInFreePoolC) {
			//LOG_INFO("allocate: threadId = %d: putting in freePoolC\n", threadId);
			putInFreePoolC(memory->freePoolC, threadId, chunk);
			donor->addInFreePoolC = false;
		}
		else {
			putInFreePoolUC(memory->freePoolUC, threadId, chunk);
		}
		while (true) { // handling call to allocate again
			if (isFullPoolEmpty(memory->fullPool,threadId)) {
				LOG_INFO("allocate: *****fullPool is empty******");
				//getHelperEntry(threadId)->needHelp = true;
				*(bool*)getHelperEntry(threadId)->atomicRef->reference = true;
				stolenChunk = NULL;
				for(int i = 1; i <= memory->n; i++) {
					threadToBeHelped = (threadId + i) % memory->n;
					announceOfThreadToBeHelped = setHazardPointer(globalHPStructure, threadId, getHelperEntry(threadToBeHelped)->atomicRef);
					if(*(bool*)announceOfThreadToBeHelped->reference) {
						LOG_INFO("allocate: going to help thread %d", threadToBeHelped);
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
					//LOG_INFO("allocate: threadId = %d: stolenChunk was not null. Putting in own fullPool\n", threadId);
					putInOwnFullPool(memory->fullPool, threadId, stolenChunk);
					//LOG_INFO("allocate: threadId = %d: stolenChunk was not null. Putting in own fullPool successful\n", threadId);
				}
			}

			while(true) {
				//LOG_INFO("allocate: threadId = %d, isFullPoolEmpty = %d\n", threadId, isFullPoolEmpty(memory->fullPool,threadId));
				chunk = getFromOwnFullPool(memory->fullPool,threadId);
				//LOG_INFO("allocate: threadId = %d, after removing a chunk, isFullPoolEmpty = %d\n", threadId, isFullPoolEmpty(memory->fullPool,threadId));
				assert(globalHPStructure->topPointers[threadId] == 0);
				if (chunk != NULL) {
					assert(globalHPStructure->topPointers[threadId] == 0);
					if (getDonorEntry(threadId)->noOfOps != memory->C) {
						//LOG_INFO("allocate: threadId = %d: noOfOps = %d\n", threadId,getDonorEntry(threadId)->noOfOps);
						getDonorEntry(threadId)->noOfOps++;
						putInLocalPool(memory->localPool, threadId, chunk);
						block = getFromChunkUncontended(chunk);
						block->threadId = threadId;
						LOG_EPILOG();
						return block;
					}
					else {
						assert(globalHPStructure->topPointers[threadId] == 0);
						getDonorEntry(threadId)->noOfOps = 0;
						if (!donate(threadId, chunk)) {
							putInLocalPool(memory->localPool, threadId, chunk);
							block = getFromChunkUncontended(chunk);
							block->threadId = threadId;
							LOG_EPILOG();
							return block;
						}
						else {
							break;
						}
					}
				}
			} // while(true)
			LOG_INFO("allocate: calling allocate again");
			//return allocate(threadId);
		}
	}
}

//Chunk* doHelp(int threadToBeHelped, Chunk *stolenChunk, Helper *announceOfThreadToBeHelped) {
Chunk* doHelp(int threadId, int threadToBeHelped, Chunk *stolenChunk, ReferenceIntegerPair *announceOfThreadToBeHelped) {
	LOG_PROLOG();
	assert(globalHPStructure->topPointers[threadId] == 1);
	int i = 0;
	//LOG_INFO("doHelp: threadId: %d\n",threadId);
	//LOG_INFO("doHelp: threadId: %d, current annTS: %d, old annTS: %d\n",threadId,getHelperEntry(threadToBeHelped)->atomicRef->integer,announceOfThreadToBeHelped->atomicRef->integer);
	//if (getHelperEntry(threadToBeHelped)->timestamp != announceOfThreadToBeHelped->timestamp)
	if (getHelperEntry(threadToBeHelped)->atomicRef->integer != announceOfThreadToBeHelped->integer) {
		LOG_INFO("doHelp: somebody already helped threadToBeHelped = %d", threadToBeHelped);
		clearHazardPointer(globalHPStructure, threadId);
		LOG_EPILOG();
		return stolenChunk;
	}

	//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
	//AtomicStampedReference* oldTop = getStackThread(memory->fullPool, threadToBeHelped)->stack->top;
	if (stolenChunk == NULL) {
		LOG_INFO("doHelp: stolenChunk is null");
		//LOG_INFO("doHelp: threadId %d, top reference = %u\n", threadId, getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference);
		//LOG_INFO("doHelp: threadId %d, current annTS = %d, old annTS = %d\n",threadId, getHelperEntry(threadToBeHelped)->atomicRef->integer, announceOfThreadToBeHelped->atomicRef->integer);
		assert(globalHPStructure->topPointers[threadId] == 1);
		while ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
			//LOG_INFO("doHelp: inside while");
			stolenChunk = getFromOtherFullPool(memory->fullPool, i, threadId);
			LOG_INFO("doHelp: victim = %d",i);
			if (stolenChunk != NULL) {
				LOG_INFO("stealAttempt successful");
				break;
			}
			//LOG_INFO("doHelp: dtealAttempt failed\n");
			i = (i + 1) % memory->n;
			if (i == 0) {
				assert(globalHPStructure->topPointers[threadId] == 1);
				stolenChunk = moveFromSharedQueuePools(threadId);
				//LOG_INFO("doHelp: treadId = %d, topPtr value = %d\n", threadId, globalHPStructure->topPointers[threadId]);
				assert(globalHPStructure->topPointers[threadId] == 1);
				//LOG_INFO("allocate: threadID : %d, returned from moveFreomSQP with stolenChunk %u\n", threadId, stolenChunk);
				if (stolenChunk != NULL) {
					LOG_INFO("!!!!!!!!!!!!!!got a chunk from SQP");
					break;
				}
				LOG_INFO("doHelp: returned from moveFreomSQP");
			}
		}
	}
	assert(globalHPStructure->topPointers[threadId] == 1);
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef);
	//LOG_INFO("doHelp: setting HP of thread %d for oldTop %u\n", threadId, oldTop);
	assert(globalHPStructure->topPointers[threadId] == 2);
	bool *tempBoolObj;
	if ((getStackThread(memory->fullPool, threadToBeHelped)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(threadToBeHelped)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
		assert(globalHPStructure->topPointers[threadId] == 2);
		if (putInOtherFullPool(memory->fullPool, threadToBeHelped, stolenChunk, oldTop, threadId)) {
			LOG_INFO("successfully helped thread %d", threadToBeHelped);
			assert(globalHPStructure->topPointers[threadId] == 1);
			tempBoolObj = (bool*)my_malloc(sizeof(bool));
			*tempBoolObj = false;
			if (!compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
				//LOG_INFO("doHelp: thread = %d, removing HP from annouce\n", threadId);
				my_free(tempBoolObj);
			}
			LOG_EPILOG();
			return NULL;
		}
	}
	else {
		assert(globalHPStructure->topPointers[threadId] == 2);
		clearHazardPointer(globalHPStructure, threadId);
		assert(globalHPStructure->topPointers[threadId] == 1);
		//LOG_INFO("doHelp: clearing HP of top thread %d when someone else helped\n", threadId);
	}

	LOG_INFO("thread %d already got help",threadToBeHelped);
	tempBoolObj = (bool*)my_malloc(sizeof(bool));
	*tempBoolObj = false;
	if (!compareAndSet(getHelperEntry(threadToBeHelped),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
		my_free(tempBoolObj);
	}
	assert(globalHPStructure->topPointers[threadId] == 0);
	LOG_EPILOG();
	return stolenChunk;
}

void freeMem(int threadId, Block *block) {
	LOG_PROLOG();
	assert(globalHPStructure->topPointers[threadId] == 0);
	//LOG_INFO("freeMem: threadID = %d, entered freeMem\n", threadId);
	Chunk *chunk;
	if (block->threadId != threadId) {
		//LOG_INFO("freeMem: threadid = %d, freeing block %d\n", threadId, block->memBlock);
		putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
		LOG_EPILOG();
		return;
	}
	chunk = getFromLocalPool(memory->localPool,threadId);
	//LOG_INFO("freeMem: threadID = %d, chunk ptr = %u\n", threadId, chunk);
	if (chunkHasSpace(chunk)) {
		//LOG_INFO("freeMem: threadId = %d: chunk in localPool has space\n", threadId);
		putInChunkUncontended(chunk,block);
		putInLocalPool(memory->localPool,threadId, chunk);
		//LOG_INFO("freeMem: threadID = %d, chunk ptr = %u\n", threadId, chunk);
		//putInLocalPool(memory->localPool,threadId,chunk);
		LOG_EPILOG();
		return;
	}
	else {
		//LOG_INFO("freeMem: threadID = %d, chunk in localPool doesn't have space\n", threadId);
		while(true) {
			if (getDonorEntry(threadId)->noOfOps == memory->C) { // have to donate
				getDonorEntry(threadId)->noOfOps = 0;
				if(donate(threadId, chunk)) { // donation successful
					chunk = getFromFreePoolUC(memory->freePoolUC, threadId); // get a chunk from freePoolUC
					if (chunk == NULL) { // not multiples of c were passed to other threads
						putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
						LOG_EPILOG();
						return;
					}
					else { // freePoolUC had chunks
						putInChunkUncontended(chunk, block);
						putInLocalPool(memory->localPool, threadId, chunk);
						LOG_EPILOG();
						return;
					}
				} // donation unsuccessful
				else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) { //move full chunk to fullPool
					chunk = getFromFreePoolUC(memory->freePoolUC, threadId);
					if (chunk == NULL) { // not multiples of c were passed to other threads
						putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
						LOG_EPILOG();
						return;
					}
					else {
						putInChunkUncontended(chunk, block);
						putInLocalPool(memory->localPool,threadId, chunk);
						LOG_EPILOG();
						return;
					}
				}
			} // donation is not needed
			else if (putInOwnFullPool(memory->fullPool, threadId, chunk)) {
				getDonorEntry(threadId)->noOfOps++;
				chunk = getFromFreePoolUC(memory->freePoolUC, threadId);
				if (chunk == NULL) { // not multiples of c were passed to other threads
					putInSharedQueuePools(memory->sharedQueuePools, block->threadId, threadId, block);
					LOG_EPILOG();
					return;
				}
				else {
					putInChunkUncontended(chunk, block);
					putInLocalPool(memory->localPool,threadId, chunk);
					LOG_EPILOG();
					return;
				}
			}
		}
	}
	assert(globalHPStructure->topPointers[threadId] == 0);
}

bool donate(int threadId, Chunk *chunk) {
	LOG_PROLOG();
	//LOG_INFO("donate: threadID %d\n", threadId);
	int i = (getDonorEntry(threadId)->lastDonated + 1) % (memory->n);
	bool *tempBoolObj;
	assert(globalHPStructure->topPointers[threadId] == 0);
	do {
		LOG_INFO("donate: trying to donate to %d", i);
		ReferenceIntegerPair *announceOfThreadToBeHelped = setHazardPointer(globalHPStructure, threadId, getHelperEntry(i)->atomicRef);
		ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, getStackThread(memory->fullPool, i)->stack->top->atomicRef);
		assert(globalHPStructure->topPointers[threadId] == 2);
		int oldTS = announceOfThreadToBeHelped->integer;
		//	if (*(bool*)announceOfThreadToBeHelped->atomicRef->reference == true)
		if ((*(bool*)announceOfThreadToBeHelped->reference == true) && (getStackThread(memory->fullPool, i)->stack->top->atomicRef->reference == NULL) && (getHelperEntry(i)->atomicRef->integer == announceOfThreadToBeHelped->integer)) {
			LOG_INFO("donate: %d needed help", i);
			if (putInOtherFullPool(memory->fullPool, i, chunk, oldTop, threadId)) {
				//getHelperEntry(i)->compareAndSet(...);
				assert(globalHPStructure->topPointers[threadId] == 1);
				LOG_INFO("donate: successfully donated to %d", i);
				tempBoolObj = (bool*)my_malloc(sizeof(bool));
				*tempBoolObj = false;
				if(!compareAndSet(getHelperEntry(i),announceOfThreadToBeHelped->reference, tempBoolObj, announceOfThreadToBeHelped->integer, (announceOfThreadToBeHelped->integer + 1), threadId)) {
					//clearHazardPointer(globalHPStructure, threadId);
					my_free(tempBoolObj);
				}
				assert(globalHPStructure->topPointers[threadId] == 0);
				getDonorEntry(threadId)->lastDonated = i;
				LOG_EPILOG();
				return true;
			}
			LOG_INFO("donate: donation to %d failed: someone else helped", i);
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
			LOG_INFO("donate: clearing HP of thread. Donation was not needed");
		}
		i = (i + 1) % (memory->n);
		assert(globalHPStructure->topPointers[threadId] == 0);
	} while(i != (getDonorEntry(threadId)->lastDonated + 1) % (memory->n));
	LOG_EPILOG();
	return false;
}

Chunk* moveFromSharedQueuePools(int threadId) {
	LOG_PROLOG();
	assert(globalHPStructure->topPointers[threadId] == 1);
	//LOG_INFO("moveFromSQP: threadID: %d\n", threadId);
	int primThread = 0, secThread = 0;
	Block *block;
	Chunk* chunk;
	QueueElement *oldQueueHead;
	for (primThread = 0; primThread < memory->n; primThread++) {
		for (secThread = 0; secThread < memory->n; secThread++) {
			assert(globalHPStructure->topPointers[threadId] == 1);
			LOG_INFO("moveFromSQP: primThread: %d, secThread: %d", primThread, secThread);
			block = getFromSharedQueuePools(memory->sharedQueuePools, threadId, primThread, secThread);
			assert(globalHPStructure->topPointers[threadId] == 1);
			//LOG_INFO("moveFromSQP: threadID: %d block ptr : %u\n", threadId, block);
			if (block != NULL) {
				LOG_INFO("moveFromSharedQueuePools: block was was taken out from sec queue of prim thread");
				oldQueueHead = setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				assert(globalHPStructure->topPointers[threadId] == 2);
				//LOG_INFO("moveFromSQP: setting HP of thread %d for oldQueueHEad %u\n", threadId, oldQueueHead);
				//getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					LOG_INFO("Prim queue had free chunks");
					if (chunkHasSpace(oldQueueHead->next->value)) {
						if (putInChunkContended(getQueueThread(memory->freePoolC, primThread)->queue->head->next->value, block)) {
							clearHazardPointer(globalHPStructure, threadId);
							assert(globalHPStructure->topPointers[threadId] == 1);
							LOG_INFO("moveFromSharedQueuePools: Block was inserted in chunk");
							continue; // now go to next secThread
						}
						else {
							// do sth with the removed block
							clearHazardPointer(globalHPStructure, threadId);
							LOG_INFO("Chunk had space but someone else simul put the block in the chunk. Putting the removed block in my own queue of prim thread");
							putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
							assert(globalHPStructure->topPointers[threadId] == 1);
						}
					}
					else { //chunk doesn't have space.try moving the chunk to fullPool
						assert(globalHPStructure->topPointers[threadId] == 2);
						LOG_INFO("chunk didn't have space. Putting the removed block in my own queue of prim thread");
						putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
						chunk = getFromFreePoolC(memory->freePoolC, threadId, primThread, oldQueueHead);
						assert(globalHPStructure->topPointers[threadId] == 1);
						if (chunk != NULL) {
							LOG_EPILOG();
							return chunk;
						}
						assert(globalHPStructure->topPointers[threadId] == 1);
					}
				} // prim queue had free chunks
				else {
					clearHazardPointer(globalHPStructure, threadId);
					LOG_INFO("Prim Queue had no chunks. Putting the removed block in my own queue of prim thread");
					assert(globalHPStructure->topPointers[threadId] == 1);
					putInSharedQueuePools(memory->sharedQueuePools, primThread, threadId, block);
				}
			}
			else { //block is null
				//clearHazardPointer(globalHPStructure, threadId);
				LOG_INFO("block couldn't be taken out from sec queue of prim SQP");
				assert(globalHPStructure->topPointers[threadId] == 1);
				oldQueueHead = setHazardPointer(globalHPStructure, threadId, getQueueThread(memory->freePoolC, primThread)->queue->head);
				//LOG_INFO("moveFromSharedQueuePools: setting HP. of thread %d for oldQueuHEad %u\n", threadId, oldQueueHead);
				//oldQueueHead = getHazardPointer(globalHPStructure, threadId);
				if (!isQueueEmpty(getQueueThread(memory->freePoolC, primThread)->queue)) {
					if (!chunkHasSpace(oldQueueHead->next->value)) {
						LOG_INFO("moveFromSMP: trying to pop a chunk from prim queue");
						chunk = getFromFreePoolC(memory->freePoolC, threadId, primThread, oldQueueHead);
						assert(globalHPStructure->topPointers[threadId] == 1);
						if (chunk != NULL) {
							LOG_EPILOG();
							return chunk;
						}
					}
					else {
						clearHazardPointer(globalHPStructure, threadId);
						//LOG_INFO("moveFromSQP: clearing HP of thread %d block block was null\n", threadId);
					}
				}
				else {
					clearHazardPointer(globalHPStructure, threadId);
					//LOG_INFO("moveFromSQP: clearing HP of thread %d block block was nnnnull\n", threadId);
				}
				assert(globalHPStructure->topPointers[threadId] == 1);
			} // block is null
			LOG_INFO("moveFromSharedQueuePools: came here");
			assert(globalHPStructure->topPointers[threadId] == 1);
		} // sec thread
	} // prim thread
	assert(globalHPStructure->topPointers[threadId] == 1);
	LOG_EPILOG();
	return NULL;
}
