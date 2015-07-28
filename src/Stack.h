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

#define STACK_IS_EMPTY(stack) ((stack->top->atomicRef->reference == NULL))

void stackCreate(Stack *stack, int elementSize);

void stackFree(Stack *stack);

bool stackPush(Stack *stack, void* element);

void* stackPop(Stack *stack);

bool stackPushOwner(Stack *stack, void* element, int threadId);

bool stackPushOther(Stack *stack, void* element, ReferenceIntegerPair* oldTop, int threadId);

void* stackPopOwner(Stack *stack, int threadId);

void* stackPopOther(Stack *stack, int threadId);

#endif /* WAITFREEMEMALLOC_SRC_STACK_H_ */
