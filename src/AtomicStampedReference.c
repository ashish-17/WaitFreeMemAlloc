#include "AtomicStampedReference.h"
#include "commons.h"
#include <stdatomic.h>
#include "HazardPointer.h"

ReferenceIntegerPair* createReferenceIntegerPair(void* ref, int i) {
	LOG_PROLOG();
	ReferenceIntegerPair* pair = (ReferenceIntegerPair*)my_malloc(sizeof(ReferenceIntegerPair)); //added.. shouldn't we first free memory pointed by pair
	pair->reference = ref; pair->integer = i;
	LOG_EPILOG();
	return pair;
}

void freeReferenceIntegerPair(ReferenceIntegerPair* ptr) {
	LOG_PROLOG();
	if (ptr != NULL) {
		ptr->integer = 0;
		if (ptr->reference != NULL) {
			my_free(ptr->reference);
			ptr->reference = NULL;
			}
		else {
			LOG_ERROR("Trying to free reference pointer of RIP which was a NULL pointer");
		}
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

void createAtomicStampedReference(AtomicStampedReference* current, void* initialRef, int initialStamp) {
	LOG_PROLOG();
	current->atomicRef = (ReferenceIntegerPair*)my_malloc(sizeof(ReferenceIntegerPair));
	current->atomicRef->reference = initialRef;
	current->atomicRef->integer = initialStamp;
	LOG_EPILOG();
	//printf("leaving createAtomicStampedReference\n");
}

void freeAtomicStampedReference(AtomicStampedReference* ptr) {
	LOG_PROLOG();
	if (ptr != NULL) {
		freeReferenceIntegerPair(ptr->atomicRef);
		ptr->atomicRef = NULL;
	}
	else {
		LOG_ERROR("Trying to free NULL pointer");
	}
	LOG_EPILOG();
}

bool compareAndSet(AtomicStampedReference* current,
		void* expectedReference,
		void* newReference,
		int expectedStamp,
		int newStamp,
		int threadId) {
	LOG_PROLOG();
	/*
	 * Make a copy of current->atomicRef
	 * AtomicStampedReference* copy = current->atomicRef;
	 * free if atomic_*** sucesss
	 */
	ReferenceIntegerPair *copy = current->atomicRef;
	/*bool result =   (expectedReference == current->atomicRef->reference &&
			expectedStamp == current->atomicRef->integer &&
			((newReference == current->atomicRef->reference &&
					newStamp == current->atomicRef->integer)||
					atomic_compare_exchange_strong(&current->atomicRef, &current->atomicRef, createReferenceIntegerPair(newReference, newStamp)))); //not sure what we want
	 */
	bool result =   (expectedReference == current->atomicRef->reference &&
			expectedStamp == current->atomicRef->integer &&
			((newReference == current->atomicRef->reference &&
					newStamp == current->atomicRef->integer)||
					atomic_compare_exchange_strong(&current->atomicRef, &copy, createReferenceIntegerPair(newReference, newStamp)))); //not sure what we want


	if (result) {
		LOG_INFO("compareAndSet: trying to clearHP");
		clearHazardPointer(globalHPStructure, threadId);
		LOG_INFO("compareAndSet: clearing HP on CAS successful");
		//free(copy->reference);
		freeMemHP(globalHPStructure, threadId, copy);
		//LOG_INFO("compareAndSet: trying to free = %u\n", threadId, copy);
		//return result;
	}
	else {
		clearHazardPointer(globalHPStructure, threadId);
		LOG_INFO("compareAndSet: clearing HP on CAS failure");
		//return result;
	}
	LOG_EPILOG();
	return result;
}

void set(AtomicStampedReference* current, void* newReference, int newStamp) {
	LOG_PROLOG();
	if (newReference != current->atomicRef->reference || newStamp != current->atomicRef->integer)
	{
		/*
		 * Make a copy of current->atomicRef
		 * AtomicStampedReference* copy = current->atomicRef;
		 * free if atomic_*** sucesss
		 */
		//ReferenceIntegerPair *copy = current->atomicRef;
		/*if(current->atomicRef->reference != NULL) {
			free(current->atomicRef->reference);
		}*/
		my_free(current->atomicRef);
		current->atomicRef = createReferenceIntegerPair(newReference, newStamp);
		current->atomicRef->reference = newReference;
		current->atomicRef->integer = newStamp;

		/*
		 * free copy
		 */
	}
	LOG_EPILOG();
}
