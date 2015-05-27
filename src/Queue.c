#include "Queue.h"

QueueElement* createNode(void *value) {
	//printf("createNode blk ptr = %u\n", value);
	QueueElement *element = (QueueElement*) malloc(sizeof(QueueElement));
	element->value = value;
	element->next = NULL;
	return element;
}

void queueCreate(Queue *queue, int elementSize) {
	queue->head = createNode(NULL);
	queue->tail = queue->head;
	queue->elementSize  = elementSize;
}

void queueFree(Queue *queue) {

}

bool queueEnq(Queue *queue, const void* element) {
	QueueElement *queueElement = createNode(element);
	//printf("queueEnq: value = %u, next ptr = %u\n", queueElement->value, queueElement->next);
	QueueElement *last = queue->tail;
	QueueElement *next = last->next;
	if (last == queue->tail) {
		if (next == NULL) {
			if (atomic_compare_exchange_strong(&last->next, &next, queueElement)) {
				atomic_compare_exchange_strong(&queue->tail, &last, queueElement);
		//		printf("queueEnq: tailvalue = %u, next ptr = %u\n", queue->tail->value, queue->tail->next);
				return true;
			}
		}
		else {
			atomic_compare_exchange_strong(&queue->tail, &last, next);
		}
	}
	return false;
}

void* queueDeq(Queue *queue) {
	QueueElement *first = queue->head;
	QueueElement *last = queue->tail;
	QueueElement *next = first->next;
	//printf("in deq \n");
	if (first == queue->head) { // someone else dequeued
		//printf("first == queue->head\n");
		if (first == last) {    // queue is empty
			//printf("first == last\n");
			if (next == NULL) {
				return NULL;
			}
			else {  // someone is enqueuing simultaneously
				//printf("someone is enqueuing simul \n");
				atomic_compare_exchange_strong(&queue->tail, &last, next);
				return NULL;
			}
		}
		else {
			void *element = next->value;
			if (atomic_compare_exchange_strong(&queue->head, &first, next)) {
				//printf("dequeuing successful\n");
				return element;
			}
			else {
				return NULL;
			}
		}
	}
}
