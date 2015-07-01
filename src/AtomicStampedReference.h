#include "stdbool.h"
#include "commons.h"
#include "stdlib.h"

typedef struct {
	void* reference;
	int integer;
} ReferenceIntegerPair;

typedef struct {
	ReferenceIntegerPair* atomicRef;
} AtomicStampedReference;

ReferenceIntegerPair* createReferenceIntegerPair(void* ref, int i);

void freeAtomicStampedReference(AtomicStampedReference* ptr);

void createAtomicStampedReference(AtomicStampedReference* current, void* initialRef, int initialStamp);

bool compareAndSet(AtomicStampedReference* current,
				   void* expectedReference,
				   void* newReference,
				   int expectedStamp,
				   int newStamp,
				   int threadId);

void set(AtomicStampedReference* current, void* newReference, int newStamp);
