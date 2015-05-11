#ifndef WAITFREEMEMALLOC_SRC_BLOCK_H_
#define WAITFREEMEMALLOC_SRC_BLOCK_H_

#include "commons.h"

typedef struct {
	int memBlock;
}Block;

Block* createBlock(int number);

#endif /* WAITFREEMEMALLOC_SRC_BLOCK_H_ */
