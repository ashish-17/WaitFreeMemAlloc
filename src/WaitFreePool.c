#include "WaitFreePool.h"

Helper* getHelperEntry(Announce *announce, int index)
{
	return (announce->helpers + index);
}

void createWaitFreePool(Pool *fullPool, Pool *localPool, Pool *freePool, Announce *announce, int n, int m, int c) {

	int numOfThreads = n;
	int totalBlocks = m;
	int numOfBlocksPerChunk = c;
	int numOfChunks = m/c;
	int numOfChunksPerThread = numOfChunks/numOfThreads;

	fullPool = createFullPool(numOfThreads);
	localPool = createLocalPool(numOfThreads);
	freePool = createFreePool(numOfThreads);

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
			putInLocalPool(localPool,j,chunk);
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
			putInOwnFullPool(fullPool,j,chunk);
		}
	}

	// Set up initial announce array
	announce = (Announce*)malloc(sizeof(Announce));
	announce->helpers = (Helper*) malloc(sizeof(Helper)*numOfThreads);
	announce->numberOfHelpers = numOfThreads;
	for(int i = 0; i < numOfThreads; i++) {
		Helper* helperEntry = getHelperEntry(announce,i);
		helperEntry->needHelp = false;
		helperEntry->timestamp = 0;
	}

	// Set up initial info array
}

Block allocate(int threadId) {
	/*Chunk* chunk = getFromLocalPool(localPool, threadId);
	if (!isChunkEmpty(chunk)) {
		Block *block = getFromChunk(chunk);
		putInLocalPool(localPool, threadId, chunk);
		return block;
	}
	else {
		putInFreePool(freePool,threadId,chunk);
		if (isFullPoolEmpty(fullPool,threadId)) {

		}
	}*/
}
