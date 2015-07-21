#ifndef WAITFREEMEMALLOC_SRC_BLOCK_H_
#define WAITFREEMEMALLOC_SRC_BLOCK_H_

#define INVALID_THREAD_INDEX -1

typedef struct Block Block;
typedef void* BLOCK_MEM;

BLOCK_MEM createBlock(int nBytes);

void destroyBlock(BLOCK_MEM ptr);

void setBlockThreadIndex(BLOCK_MEM ptr, int threadIndex);

int getBlockThreadIndex(BLOCK_MEM ptr);

#endif /* WAITFREEMEMALLOC_SRC_BLOCK_H_ */
