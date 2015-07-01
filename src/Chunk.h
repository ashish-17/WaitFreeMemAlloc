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

Chunk* createChunk(Chunk *chunk, int number);

void destroyChunk(Chunk* chunk);

bool isChunkEmpty(Chunk *chunk);

bool chunkHasSpace(Chunk *chunk);

Block* getFromChunkUncontended(Chunk *chunk);

bool putInChunkUncontended(Chunk *chunk, Block *block);

Block* getFromChunkContended(Chunk *chunk);

bool putInChunkContended(Chunk *chunk, Block *block);


#endif /* WAITFREEMEMALLOC_SRC_CHUNK_H_ */
