#include "StackArray.h"
#include "Block.h"
#include "commons.h"
#include <pthread.h>

StackArrayElement* getStackArrayElement(const StackArray* stack, int index) {
	LOG_PROLOG();
	LOG_DEBUG("Get Stack Array Element in stack(%x) at index %d", stack, index);
	LOG_INFO("stack->elements ptr %u", stack->elements);
	LOG_INFO("stack->top ptr %u", stack->top);
	StackArrayElement* ptr = NULL;
    if (stack != NULL) {
        if ((index > stack->maxElements) || (index < 0)) {
            LOG_ERROR("Invalid index for stackArray");
        } else {
            ptr = (stack->elements + index);
            LOG_INFO("stack->elements ptr  at index %d is %u", index, ptr);
        }
    } else {
        LOG_ERROR("Invalid memory address for stackArray");
    }

	LOG_EPILOG();
	return ptr;
}

StackArray* stackArrayCreate(int elementSize, int maxElements) {
	LOG_PROLOG();
	LOG_DEBUG("Create a stackArray with maximum %d elements and with element size %d bytes", maxElements, elementSize);

    StackArray* stack = (StackArray*)my_malloc(sizeof(StackArray));
    if (stack != NULL) {
        size_t sizeOfStackElements = sizeof(StackArrayElement) * (maxElements + 1);
        stack->elements = (StackArrayElement*)my_malloc(sizeOfStackElements);
        LOG_INFO("stack->elements ptr %u", stack->elements);
        if (stack->elements != NULL) {
            memset(stack->elements, NULL, sizeOfStackElements);
            stack->top = stack->elements;
            LOG_INFO("stack->top ptr %u", stack->top);
            LOG_INFO("stack->elementSize ptr %u", stack->elementSize);
            LOG_INFO("stack->maxElements ptr %u", stack->maxElements);
            stack->elementSize = elementSize;
            stack->maxElements = maxElements;
        } else {
            LOG_ERROR("Error allocating memory to elements in stackArray");
            my_free(stack);
            stack = NULL;
        }
    } else {
        LOG_ERROR("Error creating a stackArray");
    }
    LOG_INFO("here");
	LOG_EPILOG();
	return stack;
}

void stackArrayFree(StackArray *stack) {
	LOG_PROLOG();
	LOG_DEBUG("Free stack at address %x", stack);

    if (stack != NULL) {
        for (int i = 0; i < (stack->maxElements + 1); i++) {
            StackArrayElement* element = getStackArrayElement(stack, i);
            if (element != NULL) {
                if (element->value != NULL) {
                    my_free(element->value);
                    element->value = NULL;
                }
            } else {
                LOG_ERROR("Invalid stackArray element");
            }
        }

        my_free(stack->elements);
        stack->elements = NULL;
        stack->top = NULL;
        stack->elementSize = 0;
        stack->maxElements = 0;

        my_free(stack);
        stack = NULL;
    } else {
        LOG_ERROR("Invalid memory address for stackArray");
    }

	LOG_EPILOG();
}

bool stackArrayIsEmpty(const StackArrayElement *top, const StackArrayElement *baseAddress) {
	LOG_PROLOG();
	bool flag = false;

	if (top != NULL && baseAddress != NULL) {
        flag = (top == baseAddress);
        LOG_DEBUG("Stack array is %s empty", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory addresses for stack");
	}

	LOG_EPILOG();
	return flag;
}

bool StackArrayIsFull(const StackArrayElement *top, const StackArrayElement *baseAddress, int sizeOfStack) {
	LOG_PROLOG();
	bool flag = false;

	if (top != NULL && baseAddress != NULL) {
        flag = (top == (baseAddress + sizeOfStack));
        LOG_DEBUG("Stack array is %s full", flag ? "" : "not");
	} else {
        LOG_ERROR("Invalid memory addresses for stack");
	}

	LOG_EPILOG();
	return flag;
}

bool stackArrayPushUncontended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;

	if (stack != NULL) {
        if (element != NULL) {
            if (StackArrayIsFull(stack->top, stack->elements, stack->maxElements)) {
                LOG_WARN("Push uncontended failed as array is full");
            } else {
                if (stack->top != NULL) {
                    stack->top->value = element;
                    stack->top++;
                    flag = true;
                } else {
                    LOG_ERROR("Top of stackArray is at an invalid position");
                }
            }
        } else {
            LOG_ERROR("Trying to push a NULL value in stackArray");
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

void* stackArrayPopUncontended(StackArray *stack) {
	LOG_PROLOG();
	void *ptr = NULL;
	if (stack != NULL) {
        if (stackArrayIsEmpty(stack->top, stack->elements)) {
            LOG_WARN("Pop uncontended failed as array is empty");
        } else {
            if (stack->top != NULL) {
                stack->top--;
                ptr = stack->top->value;
                stack->top->value = NULL;
            } else {
                LOG_ERROR("Top of stackArray is at an invalid position");
            }
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return ptr;
}

bool stackArrayPushContended(StackArray *stack, const void* element) {
	LOG_PROLOG();
	bool flag = false;
	if (stack != NULL) {
        if (element != NULL) {
            StackArrayElement *oldTop = stack->top;
            if (StackArrayIsFull(oldTop, stack->elements, stack->maxElements)) {
                LOG_WARN("Push contended failed as array is full");
            } else {
                void *nullptr = NULL;
                if (oldTop->value != NULL) {
                    LOG_WARN("Value of oldTop->value = %x, push contended failed as not null", oldTop->value);
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1); //finish the job of other and return
                    if (stack->top > (stack->elements + stack->maxElements)) {
                        LOG_ERROR("Stack overflow");
                    }
                } else if (atomic_compare_exchange_strong(&stack->top->value, &nullptr, element)) {
                    atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop + 1);
                    if (stack->top > (stack->elements + stack->maxElements)) {
                        LOG_ERROR("Stack overflow");
                    }

                    LOG_DEBUG("Successful atomic_compare_exchange_strong");
                    flag = true;
                }
            }
        } else {
            LOG_ERROR("Trying to push a NULL value in stackArray");
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return flag;
}

void* stackArrayPopContended(StackArray *stack) {
	LOG_PROLOG();
	void *ptr = NULL;
	if (stack != NULL) {
        StackArrayElement *oldTop = stack->top;
        if (stackArrayIsEmpty(oldTop, stack->elements)) {
            LOG_WARN("Pop contended failed as array is empty");
        } else {
            LOG_DEBUG("Base address = %u, OldTop = %u, top = %u", stack->elements, oldTop, stack->top);
            if ((oldTop - 1) < stack->elements) {
                LOG_ERROR("Segmentation fault");
            }

            void *element = (oldTop - 1)->value;
            if (atomic_compare_exchange_strong(&(stack->top - 1)->value, &element, NULL)) {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                if (stack->top < stack->elements) {
                    LOG_ERROR("Segmentation fault");
                }
                ptr = element;
                LOG_DEBUG("Base address = %u, top = %u", stack->elements, oldTop-1)
                LOG_INFO("Pop contended successful (%d)", ((stack->top - stack->elements) / sizeof(StackArrayElement)));
            } else {
                atomic_compare_exchange_strong(&stack->top, &oldTop, oldTop - 1);
                if (stack->top < stack->elements) {
                    LOG_ERROR("Segmentation fault");
                }
                LOG_DEBUG("Base address = %u, top = %u", stack->elements, oldTop-1)
                LOG_WARN("Pop contended failed as element already popped by other thread");
            }
        }
	} else {
        LOG_ERROR("Invalid memory address for stackArray");
	}

	LOG_EPILOG();
	return ptr;
}

