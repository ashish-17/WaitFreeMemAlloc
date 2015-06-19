#include "Chunk.h"

Chunk* createChunk(Chunk *chunk, int number) {
	chunk = (Chunk*) my_malloc(sizeof(Chunk));
	//printf("chunk ptr in createChunk= %u\n", chunk);
	//printf("chunk ptr= %u\n", chunk->stack);
	chunk->stack = (StackArray*) my_malloc(sizeof(StackArray));
	stackArrayCreate(chunk->stack, sizeof(Block), number);
	//printf("stack ptr= %u\n", chunk->stack);
	//chunk->numOfBlocks = number;

	return chunk;
}

bool isChunkEmpty(Chunk *chunk) {
	return (stackArrayIsEmpty(chunk->stack));
}

bool chunkHasSpace(Chunk *chunk) {
	return (!StackArrayIsFull(chunk->stack));
	/*if (chunk->numOfBlocks != chunk->stack->numberOfElements)
		return true;
	else
		return false;*/
}

Block* getFromChunkUncontended(Chunk *chunk) {
	return stackArrayPopUncontended(chunk->stack);
}

bool putInChunkUncontended(Chunk *chunk, Block *block) {
	printf("PutInChunkUC\n");
	bool res = stackArrayPushUncontended(chunk->stack, block);
	printf("chunksize %d\n", chunk->stack->maxElements);
	return res;
}

Block* getFromChunkContended(Chunk *chunk) {
	return stackArrayPopContended(chunk->stack);
}

bool putInChunkContended(Chunk *chunk, Block *block) {
	bool res = stackArrayPushContended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	return res;
}
