#ifndef WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_
#define WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_

#include "Block.h"
#include "commons.h"


void createWaitFreePool(int m, int n, int c, int C);

void destroyWaitFreePool();

BLOCK_MEM allocate(int threadId, bool toBePassed);

void freeMem(int threadId, BLOCK_MEM block);

void destroyWaitFreePool();

#endif /* WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_ */
