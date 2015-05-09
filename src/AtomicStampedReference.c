#include "AtomicStampedReference.h"
#include "stdatomic.h"

struct ReferenceIntegerPair* createReferenceIntegerPair(void* ref, int i) { //changed the header.. now returning a pointer instead of void
	struct ReferenceIntegerPair* pair = (struct ReferenceIntegerPair*)malloc(sizeof(struct ReferenceIntegerPair)); //added.. shouldn't we first free memory pointed by pair
	pair->reference = ref; pair->integer = i;
	return pair;
}

void createAtomicStampedReference(AtomicStampedReference* current, void* initialRef, int initialStamp) {
	current->atomicRef = (struct ReferenceIntegerPair*)malloc(sizeof(struct ReferenceIntegerPair));
	current->atomicRef->reference = initialRef;
	current->atomicRef->integer = initialStamp;
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


					/* ||
						Bhagwan bharose
						   atomicRef.compareAndSet(current,
						   new ReferenceIntegerPair<V>(newReference,newStamp))));*/
}

void set(AtomicStampedReference* current, void* newReference, int newStamp) {
	if (newReference != current->atomicRef->reference || newStamp != current->atomicRef->integer)
	{
		current->atomicRef = createReferenceIntegerPair(newReference, newStamp); //memory leak??
		/*Bhagwan bharose*/
		//atomicRef.set(new ReferenceIntegerPair<V>(newReference, newStamp));
		current->atomicRef->reference = newReference;
		current->atomicRef->integer = newStamp;
	}
}