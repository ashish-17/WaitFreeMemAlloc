#include "AtomicStampedReference.h"
#include "commons.h"
#include <stdatomic.h>

ReferenceIntegerPair* createReferenceIntegerPair(void* ref, int i) {
	ReferenceIntegerPair* pair = (ReferenceIntegerPair*)malloc(sizeof(ReferenceIntegerPair)); //added.. shouldn't we first free memory pointed by pair
	pair->reference = ref; pair->integer = i;
	return pair;
}

void createAtomicStampedReference(AtomicStampedReference* current, void* initialRef, int initialStamp) {
	//printf("In createAtomicStampedReference\n");
	current->atomicRef = (ReferenceIntegerPair*)malloc(sizeof(ReferenceIntegerPair));
	current->atomicRef->reference = initialRef;
	current->atomicRef->integer = initialStamp;
	//printf("leaving createAtomicStampedReference\n");
}

bool compareAndSet(AtomicStampedReference* current,
		void* expectedReference,
		void* newReference,
		int expectedStamp,
		int newStamp,
		int threadId) {
	/*
	 * Make a copy of current->atomicRef
	 * AtomicStampedReference* copy = current->atomicRef;
	 * free if atomic_*** sucesss
	 */
	ReferenceIntegerPair *copy = current->atomicRef;
	bool result =   (expectedReference == current->atomicRef->reference &&
			expectedStamp == current->atomicRef->integer &&
			((newReference == current->atomicRef->reference &&
					newStamp == current->atomicRef->integer)||
					atomic_compare_exchange_strong(&current->atomicRef, &current->atomicRef, createReferenceIntegerPair(newReference, newStamp)))); //not sure what we want
	if (result) {
		//printf("CAS: trying to clearHP\n");
		clearHazardPointer(globalHPStructure, threadId);
		//printf("cleared\n");
		//free(copy->reference);
		freeMemHP(globalHPStructure, threadId, copy);
		printf("---- thread = %d trying to free = %u\n", threadId, copy);
		return result;
	}
	else {
		clearHazardPointer(globalHPStructure, threadId);
		return result;
	}
}

void set(AtomicStampedReference* current, void* newReference, int newStamp) {
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
		free(current->atomicRef);
		current->atomicRef = createReferenceIntegerPair(newReference, newStamp);
		current->atomicRef->reference = newReference;
		current->atomicRef->integer = newStamp;

		/*
		 * free copy
		 */
	}
}
