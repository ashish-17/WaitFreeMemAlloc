#include "commons.h"
#include "Stack.h"
#include "HazardPointer.h"

void stackCreate(Stack *stack, int elementSize)
{
	LOG_PROLOG();
	//printf("In stackCreate\n");
	stack->top = (AtomicStampedReference*) my_malloc(sizeof(AtomicStampedReference));
	createAtomicStampedReference(stack->top, NULL, 0);
	stack->elementSize = elementSize;
	//printf("Inside create stack and size of chunk is %d\n", stack->elementSize);
	stack->numberOfElements = 0;
	//printf("leaving stackCreate\n");
	LOG_EPILOG();
}

void stackFree(Stack *stack)
{

}

bool stackIsEmpty(const Stack *stack)
{
	LOG_PROLOG();
	bool flag;
	if (stack->top->atomicRef->reference == NULL)
		flag = true;
	else
		flag = false;
	LOG_EPILOG();
	return flag;
}

bool stackPush(Stack *stack, const void* element) {
	LOG_PROLOG();
	//printf("inside stackPush\n");
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	//printf("allocated a node\n");
	node->value = element;
	//printf("the elementSize in stackPush is %u\n",stack->elementSize);
	//printf("the element in stackPush is %u\n",element);
	//printf("CDSC\n");
	//printf("in stackPush the element is %d\n", element->value);

	//memcpy(node->value, element, stack->elementSize);
	//node->value = element;

	//printf("after memcpy node value = %u\n",node->value);
	node->next = (StackElement*)stack->top->atomicRef->reference;

	stack->top->atomicRef->reference = node;
	stack->numberOfElements++;
	//printf("reached here\n");
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

bool stackPushOwner(Stack *stack, const void* element, int threadId)
{
	LOG_PROLOG();
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	node->value = element;
	node->next = (StackElement*)stack->top->atomicRef->reference;
	//printf("stackPushOwner before setting HP\n");
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, stack->top->atomicRef);
	//printf("stackPushOwner: setting HP of thread %d for oldTop %u\n", threadId, oldTop);
	//printf("stack->top->atomicRef = %u\n", stack->top->atomicRef);
	//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
	//printf("oldTop = %u\n", oldTop);
	//AtomicStampedReference* oldTop = stack->top;
	//printf("stackPushOwner after setting HP\n");
	bool flag = compareAndSet(stack->top, oldTop->reference, node, oldTop->integer, (oldTop->integer + 1), threadId);
	LOG_EPILOG();
	return flag;
}

bool stackPushOther(Stack *stack, const void* element, ReferenceIntegerPair* oldTop, int otherThreadId, int threadId)
{
	LOG_PROLOG();
	StackElement *node = (StackElement*)my_malloc(sizeof(StackElement));
	node->value = element;
	node->next = (StackElement*)stack->top->atomicRef->reference;
	//printf("stackPushOther: threadId:%d going to call CAS\n", threadId);
	bool flag = compareAndSet(stack->top, NULL, node, oldTop->integer, (oldTop->integer + 1), threadId);
	LOG_EPILOG();
	return flag;
}

void* stackPopOwner(Stack* stack, int threadId)
{
	LOG_PROLOG();
	void *ptr = NULL;
	ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadId, stack->top->atomicRef);
	//printf("stackPopOwner: setting HP of thread %d for oldTop %u\n", threadId, oldTop);
	//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
	//AtomicStampedReference *oldTop = stack->top; //initially it was this .. then accordingly add atomicRef everywhere

	if(stack->top->atomicRef->reference == NULL){
		printf("stackPopOwner: stack was empty\n");
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

void* stackPopOther(Stack* stack, int otherThreadId, int threadIndex)
{
	LOG_PROLOG();
	void *ptr = NULL;
	//printf("stackPopOther:\n");
	//printf("stackPopOther: currentTop = %u, expected top = %u\n", stack->top->atomicRef->reference, (oldTop->reference));
	if (stack->top->atomicRef->reference == NULL) {
		//printf("stackPopOther: stack was already empty \n");
		ptr = NULL;
	}
	else {
		//printf("stackPopOther: victim's stack value\n");
		ReferenceIntegerPair *oldTop = setHazardPointer(globalHPStructure, threadIndex, stack->top->atomicRef);
		//printf("stackPopOther: setting HP of thread %d for oldTop %u\n", threadIndex, oldTop);
		//ReferenceIntegerPair *oldTop = (ReferenceIntegerPair*)getHazardPointer(globalHPStructure, threadId);
		//printf("stackPopOther: oldTop = %u\n",oldTop);
		StackElement *copy = (StackElement*)(oldTop->reference);
		StackElement *nextTopReference = ((StackElement*)(oldTop->reference))->next;
		if (nextTopReference == NULL) {
			//printf("stackPopOther: stack had only one chunk \n");
			clearHazardPointer(globalHPStructure, threadIndex);
			//printf("stackPopOther: clearing HP of thread %d nextTopRef was null (stack had one node)\n", threadIndex);
			ptr = NULL;
		}
		else if (compareAndSet(stack->top, oldTop->reference, ((StackElement*)oldTop->reference)->next, oldTop->integer, (oldTop->integer + 1), threadIndex)) {
			void* poppedItem = ((StackElement*)oldTop->reference)->value;
			printf("stackPopOther: threadid = %d inside CAS \n", threadIndex);
			my_free(copy);
			ptr = poppedItem;
		}
		else
		{
			printf("stackPopOther: CAS failed\n");
			ptr = NULL;
		}
	}
	LOG_EPILOG();
	return ptr;
}
