#include "StackArray.h"
#include "commons.h"

StackArrayElement* getStackArrayElement(StackArray* stack, int index)
{
	LOG_PROLOG();
	StackArrayElement* ptr = (stack->elements + index);
	LOG_EPILOG();
	return ptr;
}

void stackArrayCreate(StackArray *stack, int elementSize, int maxElements) {
	LOG_PROLOG();
	//LOG_INFO("In stackCreate\n");
	stack->elements = (StackArrayElement*) my_malloc(sizeof(StackArrayElement) * (maxElements + 1));
	for (int i = 0; i < maxElements + 1; i++) {
		getStackArrayElement(stack,i)->value = NULL;
	}
	stack->top = stack->elements;
	stack->elementSize = elementSize;
	//LOG_INFO("Inside create stack and size of chunk is %d\n", stack->elementSize);
	stack->maxElements = maxElements;
	//LOG_INFO("leaving stackCreate\n");
	LOG_EPILOG();
}

void stackArrayFree(StackArray *stack) {
	LOG_PROLOG();
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
	LOG_EPILOG();
}

bool stackArrayIsEmpty(const StackArray *stack) {
	LOG_PROLOG();
	//return (stack->top == NULL);
	bool flag = (stack->top == getStackArrayElement(stack, 0));
	LOG_EPILOG();
	return flag;
}

bool StackArrayIsFull(const StackArray *stack) {
	LOG_PROLOG();
	bool flag = (stack->top == getStackArrayElement(stack, stack->maxElements));
	//return (stack->numberOfElements == stack->maxElements);
	LOG_EPILOG();
	return flag;
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag;
	if (StackArrayIsFull(stack)) {
		flag = false;
	}
	else {
		stack->top->value = element;
		stack->top++;
		flag = true;
	}
	LOG_EPILOG();
	return flag;
}

void* stackArrayPopUncontended(StackArray *stack) {
	LOG_PROLOG();
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
	LOG_EPILOG();
	return ptr;
}

bool stackArrayPushContended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;
	StackArrayElement *oldTop = stack->top;
	if (StackArrayIsFull(stack)) {
		LOG_INFO("stack array is full\n");
		flag = false;
	}
	else {
		void *nullptr = NULL;
		if (oldTop->value != NULL) {
			LOG_INFO("stackArrayPushContended: oldTop->value was not null");
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
			flag = false;
		}
		//LOG_INFO("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
		else if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
			atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
			//LOG_INFO("CAS successful = %d\n", status);
			//LOG_INFO("after changing the top pointer\n");
			//LOG_INFO("stack->top->value = %u, stack->top = %u\n", stack->top->value, stack->top);
			flag = true;
		}
	}
	LOG_EPILOG();
	return flag;
}

void* stackArrayPopContended(StackArray *stack) {
	LOG_PROLOG();
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
	LOG_EPILOG();
	return ptr;
}
