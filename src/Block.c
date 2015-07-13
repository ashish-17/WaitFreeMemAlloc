#include "Block.h"

/*
Block* createBlock(int threadId, int number) {
	LOG_PROLOG();
	Block *block = (Block*)my_malloc(sizeof(Block));
	block->threadId = threadId;
	block->memBlock = number;
	LOG_EPILOG();
	return block;
}

void destroyBlock(Block *block) {
	LOG_PROLOG();
	if (block != NULL) {
		my_free(block);
		block = NULL;
	} else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}
 */

Block* createBlock(int nBytes) {
	LOG_PROLOG();
	Block *block = my_malloc(nBytes);
	if (block == NULL) {
		LOG_WARN("Allocated a null pointer for the block");
	}
	LOG_EPILOG();
	return block;
}

void destroyBlock(Block *block) {
	LOG_PROLOG();
	if (block != NULL) {
		my_free(block);
		block = NULL;
	} else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

void setBlockThreadIndex(Block *block, int threadIndex) {
	LOG_PROLOG();
	setIndex(block, threadIndex);
	LOG_EPILOG();
}

int getBlockThreadIndex(Block *block) {
	LOG_PROLOG();
	int threadIndex = getIndex(block);
	LOG_EPILOG();
	return threadIndex;
}
