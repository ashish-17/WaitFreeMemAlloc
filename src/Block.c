#include "Block.h"

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
