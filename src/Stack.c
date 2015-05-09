#include "commons.h"
#include "Stack.h"

void stackCreate(Stack *stack, int elementSize)
{
	createAtomicStampedReference(stack->top, NULL, 0);
	stack->elementSize = elementSize;
	stack->numberOfElements = 0;
}

void stackFree(Stack *stack)
{

}

bool stackIsEmpty(const Stack *stack)
{
	if (stack->top->atomicRef->reference == NULL)
		return true;
	else
		return false;
}

bool stackPushOwner(Stack *stack, const void* element)
{
	StackElement *node = (StackElement*)malloc(sizeof(StackElement));
	node->value = malloc(stack->elementSize);
	memcpy(node->value, element, stack->elementSize);

	node->next = (StackElement*)stack->top->atomicRef->reference;

	AtomicStampedReference* oldTop = stack->top;

	return compareAndSet(stack->top, oldTop->atomicRef->reference, node, oldTop->atomicRef->integer, (oldTop->atomicRef->integer + 1));
}

bool stackPushOther(Stack *stack, const void* element, AtomicStampedReference* oldTop)
{
	StackElement *node = (StackElement*)malloc(sizeof(StackElement));
	node->value = malloc(stack->elementSize);
	memcpy(node->value, element, stack->elementSize);

	node->next = (StackElement*)stack->top->atomicRef->reference;

	return compareAndSet(stack->top, NULL, node, oldTop->atomicRef->integer, (oldTop->atomicRef->integer + 1));
}

void* stackPopOwner(Stack stack)
{
	AtomicStampedReference *oldTop = stack->top;
	StackElement *nextTopReference = ((StackElement*)(oldTop->atomicRef->reference))->next;
	if (compareAndSet(stack->top, oldTop->atomicRef->reference, nextTopReference, oldTop->atomicRef->integer, (oldTop->atomicRef->integer+1)))
		return ((StackElement*)oldTop->atomicRef->reference)->value;
	else
		return NULL;
}

void* stackPopOther(Stack stack,)
{
	AtomicStampedReference *oldTop = stack->top;
	StackElement *nextTopReference = ((StackElement*)(oldTop->atomicRef->reference))->next;
	if (nextTopReference == NULL)
		return NULL;
	if (compareAndSet(stack->top, oldTop->atomicRef->reference, nextTopReference, oldTop->atomicRef->integer, (oldTop->atomicRef->integer+1)))
			return ((StackElement*)oldTop->atomicRef->reference)->value;
	else
		return NULL;
}
