#include "Chunk.h"
#include "logger.h"

Chunk* createChunk(Chunk *chunk, int number) {
	LOG_PROLOG();
	chunk = (Chunk*) my_malloc(sizeof(Chunk));
	//printf("chunk ptr in createChunk= %u\n", chunk);
	LOG_INFO("chunk ptr = %u", chunk);
	chunk->stack = stackArrayCreate(sizeof(BLOCK_MEM), number);
	//printf("stack ptr= %u\n", chunk->stack);
	//chunk->numOfBlocks = number;
	LOG_EPILOG();
	return chunk;
}

void destroyChunk(Chunk* chunk) {
	LOG_PROLOG();
	if (chunk != NULL) {
		stackArrayFree(chunk->stack);
		chunk->stack = NULL;
		my_free(chunk);
		chunk = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

bool isChunkEmpty(Chunk *chunk) {
	LOG_PROLOG();
	bool flag = stackArrayIsEmpty(chunk->stack->top, chunk->stack->elements);
	LOG_EPILOG();
	return flag;
}

bool chunkHasSpace(Chunk *chunk) {
	LOG_PROLOG();
	bool flag = (!StackArrayIsFull(chunk->stack->top, chunk->stack->elements, chunk->stack->maxElements));
	LOG_EPILOG();
	return flag;
	/*if (chunk->numOfBlocks != chunk->stack->numberOfElements)
		return true;
	else
		return false;*/
}

BLOCK_MEM getFromChunkUncontended(Chunk *chunk) {
	LOG_PROLOG();
	void *ptr =  stackArrayPopUncontended(chunk->stack);
	LOG_EPILOG();
	return ptr;
}

bool putInChunkUncontended(Chunk *chunk, BLOCK_MEM block) {
	LOG_PROLOG();
	bool res = stackArrayPushUncontended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->maxElements);
	LOG_EPILOG();
	return res;
}

BLOCK_MEM getFromChunkContended(Chunk *chunk) {
	LOG_PROLOG();
	return stackArrayPopContended(chunk->stack);
	LOG_EPILOG();
}

bool putInChunkContended(Chunk *chunk, BLOCK_MEM block) {
	LOG_PROLOG();
	bool res = stackArrayPushContended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	LOG_EPILOG();
	return res;
}
