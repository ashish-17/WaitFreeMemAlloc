#ifndef WAITFREEMEMALLOC_SRC_UTILS_H_
#define WAITFREEMEMALLOC_SRC_UTILS_H_

#include <stdlib.h>
#include <stdbool.h>
#include "Block.h"

void * my_malloc(size_t nBytes);

void my_free(void *ptr);

bool isDirty(void* ptr);

void setDirty(void *ptr, bool isDirty);

Block* getBlockPtr(BLOCK_MEM ptr);

void setBlockPtr(BLOCK_MEM ptr, Block* block);

#endif /* WAITFREEMEMALLOC_SRC_UTILS_H_ */
