#ifndef WAITFREEMEMALLOC_SRC_BLOCK_H_
#define WAITFREEMEMALLOC_SRC_BLOCK_H_

#include "commons.h"
/*
typedef struct {
	int threadId;
	int memBlock;
}Block;*/

#define Block void

Block* createBlock(int nBytes);

void destroyBlock(Block *block);

void setBlockThreadIndex(Block *block, int threadIndex);

int getBlockThreadIndex(Block *block);


#endif /* WAITFREEMEMALLOC_SRC_BLOCK_H_ */
