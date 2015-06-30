#ifndef WAITFREEMEMALLOC_SRC_CODECORRECTNESS_H_
#define WAITFREEMEMALLOC_SRC_CODECORRECTNESS_H_

#include "commons.h"

void initHashTable(int numOfBlocks);

void clearHashTable(int numOfBlocks);

bool setFlagForAllocatedBlock(int blockNum);

bool clearFlagForAllocatedBlock(int blockNum);

#endif /* WAITFREEMEMALLOC_SRC_CODECORRECTNESS_H_ */
