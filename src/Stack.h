#include "AtomicStampedReference.h"

typedef struct {
	void* value;
	StackElement* next;
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

StackElement* stackPop(Stack *stack);

bool stackPushOwner(Stack *stack, const void* element);

bool stackPushOther(Stack *stack, const void* element, AtomicStampedReference* oldTop);

void* stackPopOwner(Stack stack);

void* stackPopOther(Stack stack);

