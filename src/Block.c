#include "Block.h"

Block* createBlock(int threadId, int number) {
	Block *block = (Block*)malloc(sizeof(Block));
	block->threadId = threadId;
	block->memBlock = number;
	return block;
}
