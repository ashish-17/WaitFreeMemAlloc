#include "Queue.h"
#include "Block.h"
#include "HazardPointer.h"

// Single-Enqueuer Multi-Dequeuer

QueueElement* createNode(void *value) {
	LOG_PROLOG();
	//printf("createNode blk ptr = %u\n", value);
	QueueElement *element = (QueueElement*) my_malloc(sizeof(QueueElement));
	element->value = value;
	element->next = NULL;
	LOG_EPILOG();
	return element;
}

void freeNode(QueueElement* element) {
	LOG_PROLOG();
	element->value = NULL;
	element->next = NULL;
	my_free(element);
	element = NULL;
	LOG_EPILOG();
}

void queueCreate(Queue *queue, int elementSize) {
	LOG_PROLOG();
	queue->head = createNode(NULL);
	queue->tail = queue->head;
	//printf("queueCreate: queuePtr = %u, headPtr = %u, TailPtr = %u \n", queue, queue->head, queue->tail);
	queue->elementSize  = elementSize;
	LOG_EPILOG();
}

void queueFree(Queue *queue) {
	LOG_PROLOG();
	if (queue != NULL) {
		while (!isQueueEmpty(queue)) {
			void *value = queueDeqUC(queue);
			if (value != NULL) {
				my_free(value);
				value = NULL;
			}
			else {
				LOG_ERROR("Trying to free NULL pointer pooped out from queue");
			}
		}
		queue->elementSize = 0;
		queue->tail = NULL;
		freeNode(queue->head); // freeing sentinel node
		queue->head = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

void queueEnqUC(Queue *queue, void* element) {
	LOG_PROLOG();
	QueueElement *queueElement = createNode(element);
	queue->tail->next = queueElement;
	queue->tail = queue->tail->next;
	LOG_EPILOG();
}

bool queueEnqC(Queue *queue, const void* element, int threadId) {
	LOG_PROLOG();
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
	LOG_EPILOG();
	return flag;
}

bool isQueueEmpty(Queue *queue) {
	LOG_PROLOG();
	bool flag = (queue->head == queue->tail);
	LOG_EPILOG();
	return flag;
}

void* queueDeqUC(Queue *queue) {
	LOG_PROLOG();
	void *ptr = NULL;
	if (queue->head == queue->tail) {
		ptr = NULL;
	}
	else {
		QueueElement *oldNode = queue->head;
		queue->head = queue->head->next;
		ptr = queue->head->value;
		queue->head = NULL;
		freeNode(oldNode);
		oldNode = NULL;
	}
	LOG_EPILOG();
	return ptr;
}

void* queueDeqC(Queue *queue, QueueElement *oldQueueHead, int threadId) {
	LOG_PROLOG();
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
	LOG_EPILOG();
	return ptr;
}
