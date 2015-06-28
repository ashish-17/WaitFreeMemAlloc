#include "Chunk.h"

Chunk* createChunk(Chunk *chunk, int number) {
	LOG_PROLOG();
	chunk = (Chunk*) my_malloc(sizeof(Chunk));
	//printf("chunk ptr in createChunk= %u\n", chunk);
	//printf("chunk ptr= %u\n", chunk->stack);
	chunk->stack = (StackArray*) my_malloc(sizeof(StackArray));
	stackArrayCreate(chunk->stack, sizeof(Block), number);
	//printf("stack ptr= %u\n", chunk->stack);
	//chunk->numOfBlocks = number;
	LOG_EPILOG();
	return chunk;
}

bool isChunkEmpty(Chunk *chunk) {
	LOG_PROLOG();
	bool flag = stackArrayIsEmpty(chunk->stack);
	LOG_EPILOG();
	return flag;
}

bool chunkHasSpace(Chunk *chunk) {
	LOG_PROLOG();
	bool flag = (!StackArrayIsFull(chunk->stack));
	LOG_EPILOG();
	return flag;
	/*if (chunk->numOfBlocks != chunk->stack->numberOfElements)
		return true;
	else
		return false;*/
}

Block* getFromChunkUncontended(Chunk *chunk) {
	LOG_PROLOG();
	void *ptr =  stackArrayPopUncontended(chunk->stack);
	LOG_EPILOG();
	return ptr;
}

bool putInChunkUncontended(Chunk *chunk, Block *block) {
	LOG_PROLOG();
	bool res = stackArrayPushUncontended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->maxElements);
	LOG_EPILOG();
	return res;
}

Block* getFromChunkContended(Chunk *chunk) {
	LOG_PROLOG();
	return stackArrayPopContended(chunk->stack);
	LOG_EPILOG();
}

bool putInChunkContended(Chunk *chunk, Block *block) {
	LOG_PROLOG();
	bool res = stackArrayPushContended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	LOG_EPILOG();
	return res;
}
