#include "Block.h"

Block* createBlock(int number) {
	Block *block = (Block*)malloc(sizeof(Block));
	block->memBlock = number;
	return block;
}
