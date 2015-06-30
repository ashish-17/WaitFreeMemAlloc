#ifndef WAITFREEMEMALLOC_SRC_BLOCK_H_
#define WAITFREEMEMALLOC_SRC_BLOCK_H_

#include "commons.h"

typedef struct {
	int threadId;
	int memBlock;
}Block;

Block* createBlock(int threadId, int number);

void destroyBlock(Block *block) ;

#endif /* WAITFREEMEMALLOC_SRC_BLOCK_H_ */
