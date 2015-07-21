#include "Block.h"
#include "commons.h"

struct Block {
	BLOCK_MEM ptr;
	int threadIndex;
};

BLOCK_MEM createBlock(int nBytes) {
	LOG_PROLOG();
	Block* block = my_malloc(sizeof(Block));
	if (block!= NULL) {
		block->ptr = my_malloc(nBytes);
		if (block->ptr != NULL) {
			setBlockPtr(block->ptr, block);
		} else {
			LOG_ERROR("Allocated a NULL pointer");
		}

		block->threadIndex = INVALID_THREAD_INDEX;
	} else {
		LOG_ERROR("Allocated a NULL pointer");
	}

	LOG_EPILOG();
	return block->ptr;
}

void destroyBlock(BLOCK_MEM ptr) {
	LOG_PROLOG();

	if (ptr != NULL) {
		Block* block = getBlockPtr(ptr);
		if (block!= NULL) {
			my_free(block);
		} else {
			LOG_ERROR("Allocated a NULL pointer");
		}

		my_free(ptr);
	} else {
		LOG_ERROR("Allocated a NULL pointer");
	}

	LOG_EPILOG();
}

void setBlockThreadIndex(BLOCK_MEM ptr, int threadIndex) {
	LOG_PROLOG();

	if (ptr != NULL) {
		Block* block = getBlockPtr(ptr);
		if (block!= NULL) {
			block->threadIndex = threadIndex;
		} else {
			LOG_ERROR("Allocated a NULL pointer");
		}
	} else {
		LOG_ERROR("Allocated a NULL pointer");
	}
	LOG_EPILOG();
}

int getBlockThreadIndex(BLOCK_MEM ptr) {
	LOG_PROLOG();

	int threadIndex = INVALID_THREAD_INDEX;
	if (ptr != NULL) {
		Block* block = getBlockPtr(ptr);
		if (block!= NULL) {
			threadIndex = block->threadIndex;
		} else {
			LOG_ERROR("Allocated a NULL pointer");
		}
	} else {
		LOG_ERROR("Allocated a NULL pointer");
	}

	LOG_EPILOG();
	return threadIndex;
}
