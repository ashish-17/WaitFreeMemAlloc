#include"utils.h"
#include <pthread.h>
#include <time.h>
#include "logger.h"
#include <string.h>

extern int errno;

typedef struct {
	unsigned int isDirty;
	void* block;
} hp_malloc_header;


bool isDirty(void* ptr) {
	LOG_PROLOG();
	LOG_DEBUG("Memory block to be checked dirty %x", ptr);

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	LOG_DEBUG("Actual Memory block that will be checked dirty %x", tmp);

	hp_malloc_header *header = (hp_malloc_header*)tmp;
	bool flag = header->isDirty;
	LOG_DEBUG("Value of dirty bit %d", flag);

	LOG_EPILOG();
	return flag;
}

void setDirty(void *ptr, bool isDirty) {
	LOG_PROLOG();
	LOG_DEBUG("Memory block to be set(%d) dirty %x", isDirty, ptr);

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	LOG_DEBUG("Actual Memory block that will be set(%d) dirty %x", isDirty, tmp);

	hp_malloc_header *header = (hp_malloc_header*)tmp;
	LOG_DEBUG("Old value of dirty bit %d", header->isDirty);

	header->isDirty = isDirty;
	LOG_EPILOG();
}

Block* getBlockPtr(BLOCK_MEM ptr) {
	LOG_PROLOG();

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);

	hp_malloc_header *header = (hp_malloc_header*)tmp;
	Block *block = header->block;

	LOG_EPILOG();
	return block;
}

void setBlockPtr(BLOCK_MEM ptr, Block* block) {
	LOG_PROLOG();

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);

	hp_malloc_header *header = (hp_malloc_header*)tmp;

	header->block = block;
	LOG_EPILOG();
}


void * my_malloc(size_t nBytes) {
	LOG_PROLOG();

	size_t sizeOfBlock = nBytes + sizeof(hp_malloc_header);
	void* ptr = malloc(sizeOfBlock);
	if (ptr != NULL) {
		memset(ptr, 0, sizeof(hp_malloc_header));

		LOG_DEBUG("Pointer to allocated memory(%d bytes) %x", sizeOfBlock, ptr);

		char* tmp = ptr;
		tmp += sizeof(hp_malloc_header);
		ptr = tmp;
		LOG_DEBUG("Pointer(%d bytes) returned to caller %x", nBytes, ptr);
	} else {
		int err = errno;        // Preserve the errno from the failed malloc().
		LOG_WARN("malloc failed : %s", strerror(err)); // Log the error corresponding to the failed malloc()
	}

	LOG_EPILOG();
	return ptr;
}

void my_free(void *ptr) {
	LOG_PROLOG();
	LOG_DEBUG("Pointer to be freed %x", ptr);

	char* tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	LOG_DEBUG("Pointer to actual memory being freed %x", tmp);

	free(tmp);

	LOG_EPILOG();
}

