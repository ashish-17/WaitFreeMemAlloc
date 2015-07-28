/*
 * Chunk.h
 *
 *  Created on: May 8, 2015
 *      Author: Archita
 */

#ifndef WAITFREEMEMALLOC_SRC_CHUNK_H_
#define WAITFREEMEMALLOC_SRC_CHUNK_H_

#include "Block.h"
#include <stdbool.h>
#include "StackArray.h"

typedef struct {
	StackArray* stack;
	//int numOfBlocks;
} Chunk;

#define IS_CHUNK_EMPTY(chunk) (stackArrayIsEmpty(chunk->stack->top, chunk->stack->elements))
#define IS_CHUNK_FULL(chunk) (stackArrayIsFull(chunk->stack->top, chunk->stack->elements, chunk->stack->maxElements))
#define CHUNK_HAS_SPACE(chunk) (!IS_CHUNK_FULL(((Chunk*)chunk)))

//bool CHUNK_HAS_SPACE(Chunk *chunk);

Chunk* createChunk(int number);

void destroyChunk(Chunk* chunk);

BLOCK_MEM getFromChunkUncontended(Chunk *chunk);

bool putInChunkUncontended(Chunk *chunk, BLOCK_MEM block);

BLOCK_MEM getFromChunkContended(Chunk *chunk);

bool putInChunkContended(Chunk *chunk, BLOCK_MEM block);


#endif /* WAITFREEMEMALLOC_SRC_CHUNK_H_ */
