#ifndef ATOMICSTAMPEDREF_H_INCLUDED
#define ATOMICSTAMPEDREF_H_INCLUDED

#include "commons.h"

typedef struct AtomicStampedRef AtomicStampedRef;

AtomicStampedRef* createAtomicStampedRef(void* ref, int stamp);

void destroyAtomicStampedRef(AtomicStampedRef* atomicStampedRef);

void* getRef(AtomicStampedRef* atomicStampedRef);

int getStamp(AtomicStampedRef* atomicStampedRef);

bool cmpAndSet(AtomicStampedRef** currentRef,
               void* expectedRef,
               void* newRef,
               int expectedStamp,
               int newStamp,
               int threadIndex);

#endif // ATOMICSTAMPEDREF_H_INCLUDED
