#include "commons.h"
#include "Stack.h"

void stackCreate(Stack *stack, int elementSize)
{
	//printf("In stackCreate\n");
	stack->top = (AtomicStampedReference*) malloc(sizeof(AtomicStampedReference));
	createAtomicStampedReference(stack->top, NULL, 0);
	stack->elementSize = elementSize;
	//printf("Inside create stack and size of chunk is %d\n", stack->elementSize);
	stack->numberOfElements = 0;
	//printf("leaving stackCreate\n");
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

bool stackPush(Stack *stack, const void* element) {
	//printf("inside stackPush\n");
	StackElement *node = (StackElement*)malloc(sizeof(StackElement));
	//printf("allocated a node\n");
	node->value = malloc(stack->elementSize);
	//printf("the elementSize in stackPush is %u\n",stack->elementSize);
	//printf("the element in stackPush is %u\n",element);
	//printf("CDSC\n");
	//printf("in stackPush the element is %d\n", element->value);

	memcpy(node->value, element, stack->elementSize);
	//node->value = element;

	//printf("after memcpy node value = %u\n",node->value);
	node->next = (StackElement*)stack->top->atomicRef->reference;

	stack->top->atomicRef->reference = node;
	stack->numberOfElements++;
	//printf("reached here\n");
	return true;
}

void* stackPop(Stack *stack) {
	void* nodeValue = malloc(stack->elementSize);

	StackElement* oldTop =  (StackElement* )stack->top->atomicRef->reference;
	stack->top->atomicRef->reference = oldTop->next;

	memcpy(nodeValue, oldTop->value, stack->elementSize);

	free(oldTop->value);
	oldTop->value = NULL;

	free(oldTop);
	oldTop = NULL;

	stack->numberOfElements--;

	return nodeValue;
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

void* stackPopOwner(Stack* stack)
{
	AtomicStampedReference *oldTop = stack->top;
	if(stack->top->atomicRef->reference == NULL){
		printf("stackPopOwner: stack was empty\n");
	}
	void *oldValue = ((StackElement*)oldTop->atomicRef->reference)->value;
	StackElement *nextTopReference = ((StackElement*)(oldTop->atomicRef->reference))->next;
	if (compareAndSet(stack->top, oldTop->atomicRef->reference, nextTopReference, oldTop->atomicRef->integer, (oldTop->atomicRef->integer+1)))
		return oldValue;
	else {
		printf("stackPopOwner: CAS failed\n");
		return NULL;
	}
}



void* stackPopOther(Stack* stack)
{
	void *nodeValue = malloc(stack->elementSize);
	//printf("stackPopOther:\n");
	ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)stack->top->atomicRef;
	//printf("stackPopOther: oldTop = %u\n",oldTop);
	//printf("stackPopOther: currentTop = %u, expected top = %u\n", stack->top->atomicRef->reference, (oldTop->reference));
	if (stack->top->atomicRef->reference == NULL) {
		//printf("stackPopOther: stack was already empty \n");
		return NULL;
	}
	//printf("stackPopOther: victim's stack value")
	StackElement *nextTopReference = ((StackElement*)(oldTop->reference))->next;
	if (nextTopReference == NULL) {
		//printf("stackPopOther: stack had only one chunk \n");
		return NULL;
	}
	if (compareAndSet(stack->top, oldTop->reference, ((StackElement*)oldTop->reference)->next, oldTop->integer, (oldTop->integer + 1))) {
		//printf("stackPopOther: inside CAS \n");
		memcpy(nodeValue, ((StackElement*)oldTop->reference)->value, stack->elementSize);
		free(((StackElement*)oldTop->reference)->value);
		((StackElement*)oldTop->reference)->value = NULL;
		free(((StackElement*)oldTop->reference));
		return nodeValue;
	}
	else
	{
		printf("stackPopOther: CAS failed\n");
		free(nodeValue);
		return NULL;
	}
	/*void *oldValue = ((StackElement*)oldTop->atomicRef->reference)->value;
	StackElement *nextTopReference = ((StackElement*)(oldTop->atomicRef->reference))->next;
	if (nextTopReference == NULL)
		return NULL;
	printf("stackPopOther: before CAS \n");
	if (compareAndSet(stack->top, oldTop->atomicRef->reference, nextTopReference, oldTop->atomicRef->integer, (oldTop->atomicRef->integer+1)))
			return oldValue;
	else
		return NULL;*/
}
