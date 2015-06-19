#ifndef WAITFREEMEMALLOC_SRC_UTILS_H_
#define WAITFREEMEMALLOC_SRC_UTILS_H_

#include"stdlib.h"
#include"commons.h"

void * my_malloc(size_t nBytes);

void my_free(void *ptr);

bool isDirty(void* ptr);

void setDirty(void *ptr, bool isDirty);

#endif /* WAITFREEMEMALLOC_SRC_UTILS_H_ */
