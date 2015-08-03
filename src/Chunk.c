#include "Chunk.h"
#include "mini-logger/logger.h"
#include "utils.h"

Chunk* createChunk(int number) {
	LOG_PROLOG();
	Chunk* chunk = (Chunk*) my_malloc(sizeof(Chunk));

	LOG_INFO("chunk ptr = %u", chunk);
	chunk->stack = stackArrayCreate(sizeof(BLOCK_MEM), number);

	LOG_EPILOG();
	return chunk;
}

void destroyChunk(Chunk* chunk) {
	LOG_PROLOG();
	if (chunk != NULL) {
		stackArrayFree(chunk->stack);
		chunk->stack = NULL;
		my_free(chunk);
		chunk = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

/*bool CHUNK_HAS_SPACE(Chunk *chunk) {
	return (!(stackArrayIsFull(chunk->stack->top, chunk->stack->elements, chunk->stack->maxElements)));
}
*/
BLOCK_MEM getFromChunkUncontended(Chunk *chunk) {
	LOG_PROLOG();
	void *ptr =  stackArrayPopUncontended(chunk->stack);
	LOG_EPILOG();
	return ptr;
}

bool putInChunkUncontended(Chunk *chunk, BLOCK_MEM block) {
	LOG_PROLOG();
	bool res = stackArrayPushUncontended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->maxElements);
	LOG_EPILOG();
	return res;
}

BLOCK_MEM getFromChunkContended(Chunk *chunk) {
	LOG_PROLOG();
	return stackArrayPopContended(chunk->stack);
	LOG_EPILOG();
}

bool putInChunkContended(Chunk *chunk, BLOCK_MEM block) {
	LOG_PROLOG();
	bool res = stackArrayPushContended(chunk->stack, block);
	//printf("chunksize %d\n", chunk->stack->numberOfElements);
	LOG_EPILOG();
	return res;
}
