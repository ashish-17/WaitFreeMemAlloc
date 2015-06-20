#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

QueueElement* createNode(void *value) {
	log_msg_prolog("createNode");
	//printf("createNode blk ptr = %u\n", value);
	QueueElement *element = (QueueElement*) my_malloc(sizeof(QueueElement));
	element->value = value;
	element->next = NULL;
	log_msg_epilog("createNode");
	return element;
}

void queueCreate(Queue *queue, int elementSize) {
	log_msg_prolog("queueCreate");
	queue->head = createNode(NULL);
	queue->tail = queue->head;
	//printf("queueCreate: queuePtr = %u, headPtr = %u, TailPtr = %u \n", queue, queue->head, queue->tail);
	queue->elementSize  = elementSize;
	log_msg_epilog("queueCreate");
}

void queueFree(Queue *queue) {

}
/*
bool queueEnq1(Queue *queue, const void* element) {
	QueueElement *queueElement = createNode(element);
	printf("queueEnq1: value = %u, next ptr = %u\n", queueElement->value, queueElement->next);
	QueueElement *last = queue->tail;
	QueueElement *next = last->next;
	if (last == queue->tail) {
		if (next == NULL) {
			if (atomic_compare_exchange_strong(&last->next, &next, queueElement)) {
				atomic_compare_exchange_strong(&queue->tail, &last, queueElement);
				//printf("queueEnq: tailValue = %d, headValue = %d\n", ((Block*)(queue->tail->value))->memBlock, ((Block*)(queue->head->value))->memBlock);
				return true;
			}
		}
		else {
			atomic_compare_exchange_strong(&queue->tail, &last, next);
		}
	}
	return false;
}*/

bool queueEnq(Queue *queue, const void* element, int threadId) {
	log_msg_prolog("queueEnq");
	QueueElement *queueElement = createNode(element);
	//printf("queueEnq: value = %u, blkPtr = %u, next ptr = %u\n", queueElement->value, element, queueElement->next);
	QueueElement *last = queue->tail;
	QueueElement *next = last->next;
	bool flag = false;
	if (last == queue->tail) {
		if (next == NULL) {
			if (atomic_compare_exchange_strong(&last->next, &next, queueElement)) {
				atomic_compare_exchange_strong(&queue->tail, &last, queueElement);
				//printf("queueEnq: tailValue = %d, headValue = %d\n", ((Block*)(queue->tail->value))->memBlock, ((Block*)(queue->head->next->value))->memBlock);
				//printf("queueEnq: tailValue = %d \n", ((Block*)(queue->tail->value))->memBlock);
				flag = true;
			}
		}
		else {
			atomic_compare_exchange_strong(&queue->tail, &last, next);
		}
	}
	log_msg_epilog("queueEnq");
	return flag;
}

bool isQueueEmpty(Queue *queue) {
	log_msg_prolog("isQueueEmpty");
	bool flag = (queue->head == queue->tail);
	log_msg_epilog("isQueueEmpty");
	return flag;
}

void* queueDeq(Queue *queue, QueueElement *oldQueueHead, int threadId) {
	log_msg_prolog("queueDeq");
	void *ptr = NULL;
	//printf("queueDeq: queuePtr: %u, q->head: %u, q->tail: %u, q->head->next: %u \n",queue, queue->head, queue->tail, queue->head->next);
	QueueElement *first = oldQueueHead;
	QueueElement *last = queue->tail;
	QueueElement *next = first->next;
	//printf("queueDeq: firstPtr: %u, lastPtr: %u, first->next: %u \n",first, last, next);
	//printf("in deq \n");
	if (first == queue->head) { // someone else dequeued
		//printf("first == queue->head\n");
		if (first == last) {    // queue is empty
			clearHazardPointer(globalHPStructure, threadId);
			//printf("first == last\n");
			if (next == NULL) {
				ptr = NULL;
			}
			else {  // someone is enqueuing simultaneously
				//printf("someone is enqueuing simul \n");
				atomic_compare_exchange_strong(&queue->tail, &last, next);
				ptr = NULL;
			}
		}
		else {
			void *element = next->value;
			/*
			 * copy queue->head
			 * and if atomic_*** success to free copy pointer
			 */
			if (atomic_compare_exchange_strong(&queue->head, &first, next)) {
				//printf("dequeuing successful\n");
				clearHazardPointer(globalHPStructure, threadId);
				//printf("queueDeq: clearing HP of thread %d on successful dequeu\n", threadId);
				freeMemHP(globalHPStructure, threadId, first);
				//printf("queueDeq: thread = %d trying to free = %u\n", threadId, first);
				ptr = element;
			}
			else {
				clearHazardPointer(globalHPStructure, threadId);
				//printf("queueDeq: clearing HP of thread %d on failed dequeu\n", threadId);
				ptr = NULL;
			}
		}
	}
	else {
		clearHazardPointer(globalHPStructure, threadId);
	}
	log_msg_epilog("queueDeq");
	return ptr;
}
