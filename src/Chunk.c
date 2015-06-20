#include "Chunk.h"

Chunk* createChunk(Chunk *chunk, int number) {
	log_msg_prolog("createChunk");
	chunk = (Chunk*) my_malloc(sizeof(Chunk));
	//printf("chunk ptr in createChunk= %u\n", chunk);
	//printf("chunk ptr= %u\n", chunk->stack);
	chunk->stack = (StackArray*) my_malloc(sizeof(StackArray));
	stackArrayCreate(chunk->stack, sizeof(Block), number);
	//printf("stack ptr= %u\n", chunk->stack);
	//chunk->numOfBlocks = number;
	log_msg_epilog("createChunk");
	return chunk;
}

bool isChunkEmpty(Chunk *chunk) {
	log_msg_prolog("isChunkEmpty");
	bool flag = stackArrayIsEmpty(chunk->stack);
	log_msg_epilog("isChunkEmpty");
	return flag;
}

bool chunkHasSpace(Chunk *chunk) {
	log_msg_prolog("chunkHasSpace");
	bool flag = (!StackArrayIsFull(chunk->stack));
	log_msg_epilog("chunkHasSpace");
	return flag;
	/*if (chunk->numOfBlocks != chunk->stack->numberOfElements)
		return true;
	else
		return false;*/
}

Block* getFromChunkUncontended(Chunk *chunk) {
	log_msg_prolog("getFromChunkUncontended");
	void *ptr =  stackArrayPopUncontended(chunk->stack);
	log_msg_epilog("getFromChunkUncontended");
	return ptr;
}

bool putInChunkUncontended(Chunk *chunk, Block *block) {
	log_msg_prolog("putInChunkUC");
	bool res = stackArrayPushUncontended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->maxElements);
	log_msg_epilog("putInChunkUC");
	return res;
}

Block* getFromChunkContended(Chunk *chunk) {
	log_msg_prolog("getFromChunkContended");
	return stackArrayPopContended(chunk->stack);
	log_msg_epilog("getFromChunkContended");
}

bool putInChunkContended(Chunk *chunk, Block *block) {
	log_msg_prolog("putInChunkContended");
	bool res = stackArrayPushContended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	log_msg_epilog("putInChunkContended");
	return res;
}
