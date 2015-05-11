#include "Chunk.h"

void createChunk(Chunk *chunk, int number) {
	chunk = (Chunk*) malloc(sizeof(Chunk));
	chunk->value  = number;
}
