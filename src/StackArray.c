#include "StackArray.h"
#include "commons.h"

StackArrayElement* getStackArrayElement(StackArray* stack, int index)
{
	log_msg_prolog("getStackArrayElement");
	StackArrayElement* ptr = (stack->elements + index);
	log_msg_epilog("getStackArrayElement");
	return ptr;
}

void stackArrayCreate(StackArray *stack, int elementSize, int maxElements) {
	log_msg_prolog("stackArrayCreate");
	//printf("In stackCreate\n");
	stack->elements = (StackArrayElement*) my_malloc(sizeof(StackArrayElement) * (maxElements + 1));
	for (int i = 0; i < maxElements + 1; i++) {
		getStackArrayElement(stack,i)->value = NULL;
	}
	stack->top = stack->elements;
	stack->elementSize = elementSize;
	//printf("Inside create stack and size of chunk is %d\n", stack->elementSize);
	stack->maxElements = maxElements;
	//printf("leaving stackCreate\n");
	log_msg_epilog("stackArrayCreate");
}

void stackArrayFree(StackArray *stack) {
	log_msg_prolog("stackArrayFree");
	for (int i = 0; i < stack->maxElements + 1; i++) {
		StackArrayElement* element = getStackArrayElement(stack, i);
		if (element != NULL && element->value != NULL) {
			my_free(element->value);
			element->value = NULL;
		}
	}

	my_free(stack->elements);
	stack->elements = NULL;
	stack->top = NULL;
	stack->elementSize = 0;
	stack->maxElements = 0;
	log_msg_epilog("stackArrayFree");
}

bool stackArrayIsEmpty(const StackArray *stack) {
	log_msg_prolog("stackArrayIsEmpty");
	//return (stack->top == NULL);
	bool flag = (stack->top == getStackArrayElement(stack, 0));
	log_msg_epilog("stackArrayIsEmpty");
	return flag;
}

bool StackArrayIsFull(const StackArray *stack) {
	log_msg_prolog("StackArrayIsFull");
	bool flag = (stack->top == getStackArrayElement(stack, stack->maxElements));
	//return (stack->numberOfElements == stack->maxElements);
	log_msg_epilog("StackArrayIsFull");
	return flag;
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {
	log_msg_prolog("stackArrayPushUncontended");
	bool flag;
	if (StackArrayIsFull(stack)) {
		flag = false;
	}
	else {
		stack->top->value = element;
		stack->top++;
		flag = true;
	}
	log_msg_epilog("stackArrayPushUncontended");
	return flag;
}

void* stackArrayPopUncontended(StackArray *stack) {
	log_msg_prolog("stackArrayPopUncontended");
	void *ptr = NULL;
	if (stackArrayIsEmpty(stack)) {
		ptr = NULL;
	}
	else {
		stack->top--;
		void *element = stack->top->value;
		stack->top->value = NULL;
		ptr = element;
	}
	log_msg_epilog("stackArrayPopUncontended");
	return ptr;
}

bool stackArrayPushContended(StackArray *stack, const void* element) {
	log_msg_prolog("stackArrayPushContended");
	bool flag = false;
	if (StackArrayIsFull(stack)) {
		printf("stack array is full\n");
		flag = false;
	}
	else {
		void *nullptr = NULL;
		StackArrayElement *oldTop = stack->top;
		if (oldTop->value != NULL) {
			//printf("oldTop->value was not null\n");
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
			flag = false;
		}
		//printf("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
		else if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
			//printf("CAS successful = %d\n", status);
			//printf("after changing the top pointer\n");
			//printf("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
			flag = true;
		}
		else {
			//printf("CAS failed while updating top->value\n");
			flag = false;
		}
	}
	log_msg_epilog("stackArrayPushContended");
	return flag;
}

void* stackArrayPopContended(StackArray *stack) {
	log_msg_prolog("stackArrayPopContended");
	void *ptr = NULL;
	StackArrayElement *oldTop = stack->top;
	if (stackArrayIsEmpty(stack)) {
		ptr = NULL;
	}
	else {
		void *element = (oldTop - 1)->value;
		if (atomic_compare_exchange_strong(&(stack->top - 1)->value, &element, NULL)) {
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
			ptr = element;
		}
		else {
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
			ptr = NULL;
		}
	}
	log_msg_epilog("stackArrayPopContended");
	return ptr;
}
