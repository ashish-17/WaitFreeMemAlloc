#include "Chunk.h"

Chunk* createChunk(Chunk *chunk, int number) {
	chunk = (Chunk*) malloc(sizeof(Chunk));
	//printf("chunk ptr in createChunk= %u\n", chunk);
	//printf("chunk ptr= %u\n", chunk->stack);
	chunk->stack = (Stack*)malloc(sizeof(Stack));
	stackCreate(chunk->stack, sizeof(Block));
	//printf("stack ptr= %u\n", chunk->stack);
	chunk->numOfBlocks = number;

	return chunk;
}

bool isChunkEmpty(Chunk *chunk) {
	return (stackIsEmpty(chunk->stack));
}

bool chunkHasSpace(Chunk *chunk) {
	if (chunk->numOfBlocks != chunk->stack->numberOfElements)
		return true;
	else
		return false;
}

Block* getFromChunk(Chunk *chunk) {
	return stackPop(chunk->stack);
}

bool putInChunk(Chunk *chunk, Block *block) {
	bool res = stackPush(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	return res;
}
