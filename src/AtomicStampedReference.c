#include "AtomicStampedReference.h"
#include "commons.h"
#include <stdatomic.h>

ReferenceIntegerPair* createReferenceIntegerPair(void* ref, int i) {
	//changed the header.. now returning a pointer instead of void
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
		int newStamp) {
	return  (expectedReference == current->atomicRef->reference &&
			expectedStamp == current->atomicRef->integer &&
			((newReference == current->atomicRef->reference &&
					newStamp == current->atomicRef->integer)||
					atomic_compare_exchange_strong(&current->atomicRef, &current->atomicRef, createReferenceIntegerPair(newReference, newStamp)))); //not sure what we want

}

void set(AtomicStampedReference* current, void* newReference, int newStamp) {
	if (newReference != current->atomicRef->reference || newStamp != current->atomicRef->integer)
	{
		current->atomicRef = createReferenceIntegerPair(newReference, newStamp);
		current->atomicRef->reference = newReference;
		current->atomicRef->integer = newStamp;
	}
}
