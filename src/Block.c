#include "Block.h"

Block* createBlock(int threadId, int number) {
	log_msg_prolog("createBlock");
	Block *block = (Block*)my_malloc(sizeof(Block));
	block->threadId = threadId;
	block->memBlock = number;
	log_msg_epilog("createBlock");
	return block;
}
