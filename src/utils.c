#include"utils.h"
#include"pthread.h"

typedef struct {
	unsigned int isDirty : 1;
	unsigned int unusedBits:31;
} hp_malloc_header;

void * my_malloc(size_t nBytes) {
	printf("Trying to alloc memory %d bytes (%d)\n", nBytes, pthread_self());
	void* ptr = malloc(nBytes + sizeof(hp_malloc_header));
	printf("Successfully allocated %d bytes at %u (%d)\n", (nBytes + sizeof(hp_malloc_header)), ptr, pthread_self());
	memset(ptr, 0, sizeof(hp_malloc_header));
	char *tmp = ptr;
	tmp += sizeof(hp_malloc_header);
	printf("Data ptr after alloc at %u (%d)\n", tmp, pthread_self());

	return tmp;
}

void my_free(void *ptr) {
	printf("Trying to free memory %u (%d)\n", ptr, pthread_self());
	char* tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	printf("Original memory to free %u (%d)\n", tmp, pthread_self());
	free(tmp);
	printf("Memory successfully freed %u (%d)\n", tmp, pthread_self());
}

bool isDirty(void* ptr) {
	printf("Trying to check if address - %u is dirty - (%d)", ptr, pthread_self());
	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	hp_malloc_header *header = (hp_malloc_header*)tmp;
	printf("Value at original address - %u is %d - (%d)", tmp, header->isDirty, pthread_self());
	return header->isDirty;
}

void setDirty(void *ptr, bool isDirty) {
	printf("Trying to set dirty - %d at address - %u - (%d)",isDirty, ptr, pthread_self());
	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	hp_malloc_header *header = (hp_malloc_header*)tmp;
	header->isDirty = isDirty;
	printf("Dirty bit set successfully at %u (%d)", header, pthread_self());
}
