#include "StackArray.h"

StackArrayElement* getStackArrayElement(StackArray* stack, int index)
{
	return (stack->elements + index);
}

void stackArrayCreate(StackArray *stack, int elementSize, int maxElements) {
	//printf("In stackCreate\n");
	stack->elements = (StackArrayElement*) malloc(sizeof(StackArrayElement) * (maxElements + 1));
	for (int i = 0; i < maxElements + 1; i++) {
		getStackArrayElement(stack,i)->value = NULL;
	}
	stack->top = stack->elements;
	stack->elementSize = elementSize;
	//printf("Inside create stack and size of chunk is %d\n", stack->elementSize);
	stack->maxElements = maxElements;
	//printf("leaving stackCreate\n");
}

void stackArrayFree(StackArray *stack) {

}

bool stackArrayIsEmpty(const StackArray *stack) {
	//return (stack->top == NULL);
	return (stack->top == getStackArrayElement(stack, 0));
}

bool StackArrayIsFull(const StackArray *stack) {
	return (stack->top == getStackArrayElement(stack, stack->maxElements));
	//return (stack->numberOfElements == stack->maxElements);
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {

	if (StackArrayIsFull(stack)) {
		return false;
	}

	stack->top->value = element;
	stack->top++;
	return true;
}

void* stackArrayPopUncontended(StackArray *stack) {
	if (stackArrayIsEmpty(stack)) {
		return NULL;
	}
	stack->top--;
	void *element = stack->top->value;
	stack->top->value = NULL;

	return element;
}

bool stackArrayPushContended(StackArray *stack, const void* element) {

	if (StackArrayIsFull(stack)) {
		printf("stack array is full\n");
		return false;
	}
	void *nullptr = NULL;
	/*if (stack->top == NULL) { // * ABA problem * might have to change top into AtomicStampedReference
		//printf("set top to stackArray the first time \n");
		atomic_compare_exchange_strong(&stack->top, &nullptr, stack->elements);
		//printf("stack->top = %u, stack->elements = %u \n", stack->top, stack->elements);
	}
	 */
	StackArrayElement *oldTop = stack->top;
	//StackArrayElement *newTop = NULL;
	if (oldTop->value != NULL) {
		//printf("oldTop->value was not null\n");
		atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
		return false;
	}
	//printf("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
	if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
		atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
		//printf("CAS successful = %d\n", status);
		//printf("after changing the top pointer\n");
		//printf("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
		return true;
	}
	else {
		//printf("CAS failed while updating top->value\n");
		return false;
	}
}

void* stackArrayPopContended(StackArray *stack) {

	StackArrayElement *oldTop = stack->top;
	if (stackArrayIsEmpty(stack)) {
		return NULL;
	}

	void *element = (oldTop - 1)->value;

	if (atomic_compare_exchange_strong(&(stack->top - 1)->value, &element, NULL)) {
		//if (stack->top == getStackArrayElement(stack, 0)) {
		//atomic_compare_exchange_strong(&stack->top, &oldTop, NULL);
		//}
		//else {
		atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
		//}
		return element;
	}
	else {
		//if (stack->top == getStackArrayElement(stack, 0)) {
		//atomic_compare_exchange_strong(&stack->top, &oldTop, NULL);
		//}
		//else {
		atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
		//}
		return NULL;
	}
}
