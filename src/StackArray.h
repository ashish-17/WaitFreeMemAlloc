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
	//	int numberOfElements; // number of elements currently in the stack
	int maxElements;
} StackArray;

StackArrayElement* getStackArrayElement(StackArray* stack, int index);

void stackArrayCreate(StackArray *stack, int elementSize, int maxElements);

void stackArrayFree(StackArray *stack);

bool stackArrayIsEmpty(const StackArray *stack);

bool StackArrayIsFull(const StackArray *stack);

bool stackArrayPushUncontended(StackArray *stack, const void* element);

void* stackArrayPopUncontended(StackArray *stack);

bool stackArrayPushContended(StackArray *stack, const void* element);

void* stackArrayPopContended(StackArray *stack);


#endif /* WAITFREEMEMALLOC_SRC_STACKARRAY_H_ */
