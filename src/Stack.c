#include "commons.h"
#include "Stack.h"
#include "HazardPointer.h"

HPStructure *globalHPStructure;

void stackCreate(Stack *stack, int elementSize)
{
	LOG_PROLOG();
	stack->top = (AtomicStampedReference*) my_malloc(sizeof(AtomicStampedReference));
	createAtomicStampedReference(stack->top, NULL, 0);
	stack->elementSize = elementSize;
	stack->numberOfElements = 0;
	LOG_EPILOG();
}

void stackFree(Stack *stack)
{
	LOG_PROLOG();
	if (stack != NULL) {
		while (!STACK_IS_EMPTY(stack)) {
			void *element = stackPop(stack);
			if (element != NULL) {
				my_free(element);
				element = NULL;
			}
			else {
				LOG_ERROR("Trying to free NULL pointer popped from stack");
			}
		}
		freeAtomicStampedReference(stack->top);
		stack->elementSize = 0;
		stack->numberOfElements = 0;
		my_free(stack->top);
		stack->top = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL stack pointer");
	}
	LOG_EPILOG();
}


bool stackPush(Stack *stack, void* element) {
	LOG_PROLOG();
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	node->value = element;
	node->next = (StackElement*)stack->top->atomicRef->reference;

	stack->top->atomicRef->reference = node;
	stack->numberOfElements++;
	LOG_EPILOG();
	return true;
}

void* stackPop(Stack *stack) {
	LOG_PROLOG();
	void *ptr;
	StackElement* oldTop = (StackElement*) stack->top->atomicRef->reference;
	if (oldTop == NULL) {
		ptr = NULL;
	}
	else {
		stack->top->atomicRef->reference = oldTop->next;
		void* nodeValue = oldTop->value;
		my_free(oldTop); // OK
		oldTop = NULL;
		stack->numberOfElements--;
		ptr = nodeValue;
	}
	LOG_EPILOG();
	return ptr;
}

bool stackPushOwner(Stack *stack, void* element, int threadId)
{
	LOG_PROLOG();
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	node->value = element;
	node->next = (StackElement*)stack->top->atomicRef->reference;
	//LOG_INFO("stackPushOwner before setting HP\n");
	LOG_INFO("global Structure is %u", globalHPStructure);
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, stack->top->atomicRef);

	bool flag = compareAndSet(stack->top, oldTop->reference, node, oldTop->integer, (oldTop->integer + 1), threadId);
	LOG_EPILOG();
	return flag;
}

bool stackPushOther(Stack *stack, void* element, ReferenceIntegerPair* oldTop, int threadId)
{
	LOG_PROLOG();
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	node->value = element;
	node->next = (StackElement*)stack->top->atomicRef->reference;
	bool flag = compareAndSet(stack->top, NULL, node, oldTop->integer, (oldTop->integer + 1), threadId);
	LOG_EPILOG();
	return flag;
}

void* stackPopOwner(Stack* stack, int threadId)
{
	LOG_PROLOG();
	void *ptr = NULL;
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, stack->top->atomicRef);

	if(stack->top->atomicRef->reference == NULL){
		LOG_INFO("stackPopOwner: stack was empty\n");
		ptr = NULL;
	}
	else {
		void *oldValue = ((StackElement*)oldTop->reference)->value;
		StackElement *copy = (StackElement*)(oldTop->reference);
		StackElement *nextTopReference = ((StackElement*)(oldTop->reference))->next;
		if (compareAndSet(stack->top, oldTop->reference, nextTopReference, oldTop->integer, (oldTop->integer+1), threadId)) {
			my_free(copy);
			ptr = oldValue;
		}
		else {
			//clearHazardPointer(globalHPStructure, threadId);
			LOG_INFO("stackPopOwner: CAS failed");
			ptr = NULL;
		}
	}
	LOG_EPILOG();
	return ptr;
}

void* stackPopOther(Stack* stack, int threadIndex)
{
	LOG_PROLOG();
	void *ptr = NULL;if (stack->top->atomicRef->reference == NULL) {
		ptr = NULL;
	}
	else {
		ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadIndex, stack->top->atomicRef);
		StackElement *copy = (StackElement*)(oldTop->reference);
		StackElement *nextTopReference = ((StackElement*)(oldTop->reference))->next;
		if (nextTopReference == NULL) {
			clearHazardPointer(globalHPStructure, threadIndex);
			ptr = NULL;
		}
		else if (compareAndSet(stack->top, oldTop->reference, ((StackElement*)oldTop->reference)->next, oldTop->integer, (oldTop->integer + 1), threadIndex)) {
			void* poppedItem = ((StackElement*)oldTop->reference)->value;
			LOG_INFO("stackPopOther: threadid = %d inside CAS \n", threadIndex);
			my_free(copy);
			ptr = poppedItem;
		}
		else
		{
			LOG_INFO("stackPopOther: CAS failed\n");
			ptr = NULL;
		}
	}
	LOG_EPILOG();
	return ptr;
}
