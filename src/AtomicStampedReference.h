#include "stdbool.h"
#include "stdatomic.h"
#include "stdlib.h"

typedef struct {
	void* reference;
	int integer;
} ReferenceIntegerPair;

typedef struct {
	ReferenceIntegerPair* atomicRef;
} AtomicStampedReference;

void createReferenceIntegerPair(ReferenceIntegerPair *pair, void* ref, int i);

void createAtomicStampedReference(AtomicStampedReference* current, void* initialRef, int initialStamp);

bool compareAndSet(AtomicStampedReference* current,
				   void* expectedReference,
				   void* newReference,
				   int expectedStamp,
				   int newStamp);

void set(AtomicStampedReference* current, void* newReference, int newStamp);
