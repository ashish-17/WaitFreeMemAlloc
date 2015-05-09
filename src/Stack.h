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

void stackCreate(struct Stack *stack, int elementSize);

void stackFree(struct Stack *stack);

bool stackIsEmpty(const struct Stack *stack);

bool stackPush(struct Stack *stack, const void* element);

StackElement* stackPop(struct Stack *stack);

bool stackPushOwner(Stack *stack, const void* element);

bool stackPushOther(Stack *stack, const void* element, AtomicStampedReference* oldTop);

void* stackPopOwner(Stack stack);

void* stackPopOther(Stack stack);

