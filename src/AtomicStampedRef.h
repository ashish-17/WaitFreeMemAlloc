#ifndef ATOMICSTAMPEDREF_H_INCLUDED
#define ATOMICSTAMPEDREF_H_INCLUDED

#include "commons.h"

typedef struct AtomicStampedRef AtomicStampedRef;

typedef struct CASResult {
    bool success; // CAS success or fail?
    void* ref; // If CAS is successful, then it has the old reference else has the new reference
} CASResult;

AtomicStampedRef* createAtomicStampedRef(void* ref, int stamp);

void* destroyAtomicStampedRef(AtomicStampedRef* atomicStampedRef);

void* getRef(AtomicStampedRef* atomicStampedRef);

int getStamp(AtomicStampedRef* atomicStampedRef);

CASResult cmpAndSet(AtomicStampedRef* currentRef,
                       void* expectedRef,
                       void* newRef,
                       int expectedStamp,
                       int newStamp,
                       int threadIndex);

#endif // ATOMICSTAMPEDREF_H_INCLUDED
