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
#include "Stack.h"

typedef struct {
	Stack* stack;
	int numOfBlocks;
} Chunk;

Chunk* createChunk(Chunk *chunk, int number);

bool isChunkEmpty(Chunk *chunk);

bool chunkHasSpace(Chunk *chunk);

Block* getFromChunk(Chunk *chunk);

bool putInChunk(Chunk *chunk, Block *block);

#endif /* WAITFREEMEMALLOC_SRC_CHUNK_H_ */
