#ifndef WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_
#define WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_

#include "stdio.h"
#include "Stack.h"
#include "Chunk.h"
#include "LocalPool.h"
#include "FreePool.h"
#include "FullPool.h"
#include "pool.h"
#include "Block.h"

bool createWaitFreePool(int m, int n, int c, int C);

Block* allocate(int threadId);

void freeMem(int threadId, Block* block);

#endif /* WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_ */
