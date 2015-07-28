#ifndef WAITFREEMEMALLOC_SRC_STACKARRAY_H_
#define WAITFREEMEMALLOC_SRC_STACKARRAY_H_

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct _StackArrayElement {
	void* value;
} StackArrayElement;

typedef struct {
	StackArrayElement *top;
	StackArrayElement* elements;

	int elementSize;
	int maxElements;
} StackArray;

StackArrayElement* getStackArrayElement(const StackArray* stack, int index);

StackArray* stackArrayCreate(int elementSize, int maxElements);

void stackArrayFree(StackArray *stack);

bool stackArrayIsEmpty(const StackArrayElement *top, const StackArrayElement *baseAddress);

bool stackArrayIsFull(const StackArrayElement *top, const StackArrayElement *baseAddress, int sizeOfStack);

bool stackArrayPushUncontended(StackArray *stack, void* element);

void* stackArrayPopUncontended(StackArray *stack);

bool stackArrayPushContended(StackArray *stack, const void* element);

void* stackArrayPopContended(StackArray *stack);


#endif /* WAITFREEMEMALLOC_SRC_STACKARRAY_H_ */
