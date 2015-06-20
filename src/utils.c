#include"utils.h"
#include <pthread.h>
#include <time.h>

typedef struct {
	unsigned int isDirty : 1;
	unsigned int unusedBits:31;
} hp_malloc_header;

void * my_malloc(size_t nBytes) {
	log_msg_prolog("my_malloc");
	//printf("Trying to alloc memory %d bytes (%d)\n", nBytes, pthread_self());
	void* ptr = malloc(nBytes + sizeof(hp_malloc_header));
	//printf("Successfully allocated %d bytes at %u (%d)\n", (nBytes + sizeof(hp_malloc_header)), ptr, pthread_self());
	memset(ptr, 0, sizeof(hp_malloc_header));
	char *tmp = ptr;
	tmp += sizeof(hp_malloc_header);
	//printf("Data ptr after alloc at %u (%d)\n", tmp, pthread_self());
	log_msg_epilog("my_malloc");
	return tmp;
}

void my_free(void *ptr) {
	log_msg_prolog("my_free");
	log_msg("my_free: Trying to free memory %u", ptr);
	char* tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	log_msg("my_free: Original memory to free %u", tmp);
	free(tmp);
	log_msg("my_free: Memory successfully freed %u", tmp);
	log_msg_epilog("my_free");
}

bool isDirty(void* ptr) {
	log_msg_prolog("isDirty");
	//printf("Trying to check if address - %u is dirty - (%d)", ptr, pthread_self());
	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	hp_malloc_header *header = (hp_malloc_header*)tmp;
	//printf("Value at original address - %u is %d - (%d)", tmp, header->isDirty, pthread_self());
	bool flag = header->isDirty;
	log_msg_epilog("isDirty");
	return flag;
}

void setDirty(void *ptr, bool isDirty) {
	log_msg_prolog("setDirty");
	log_msg("setDirty: Trying to set dirty - %d at address - %u", isDirty, ptr);
	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	hp_malloc_header *header = (hp_malloc_header*)tmp;
	header->isDirty = isDirty;
	log_msg("setDirty: Dirty bit set successfully at %u", header);
	log_msg_epilog("setDirty");
}


