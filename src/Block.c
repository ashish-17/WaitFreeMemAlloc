#include "Block.h"

Block* createBlock(int threadId, int number) {
	Block *block = (Block*)my_malloc(sizeof(Block));
	block->threadId = threadId;
	block->memBlock = number;
	return block;
}
