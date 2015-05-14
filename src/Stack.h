#ifndef WAITFREEMEMALLOC_SRC_STACK_H_
#define WAITFREEMEMALLOC_SRC_STACK_H_

#include "AtomicStampedReference.h"

typedef struct _StackElement {
	void* value;
	struct _StackElement* next;
} StackElement;

typedef struct {
	 AtomicStampedReference *top;
	 StackElement* elements;

	int elementSize;
	int numberOfElements;
} Stack;

void stackCreate(Stack *stack, int elementSize);

void stackFree(Stack *stack);

bool stackIsEmpty(const Stack *stack);

bool stackPush(Stack *stack, const void* element);

void* stackPop(Stack *stack);

bool stackPushOwner(Stack *stack, const void* element);

bool stackPushOther(Stack *stack, const void* element, AtomicStampedReference* oldTop);

void* stackPopOwner(Stack *stack);

void* stackPopOther(Stack *stack);

#endif /* WAITFREEMEMALLOC_SRC_STACK_H_ */
