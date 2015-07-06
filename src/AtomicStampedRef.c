#include "AtomicStampedRef.h"
#include <stdatomic.h>
#include "HazardPointer.h"

#define STAMP_INVALID_VAL -1

struct AtomicStampedRef {
    void* ref;
    int stamp;
};

AtomicStampedRef* createAtomicStampedRef(void* ref, int stamp) {
    LOG_PROLOG();

    AtomicStampedRef* atomicStampedRef = (AtomicStampedRef*)my_malloc(sizeof(AtomicStampedRef));
    if (atomicStampedRef != NULL) {
        atomicStampedRef->ref = ref;
        atomicStampedRef->stamp = stamp;
    } else {
        LOG_ERROR("Error while allocating memory to atomic stamped reference");
    }

    LOG_EPILOG();
    return atomicStampedRef;
}

void destroyAtomicStampedRef(AtomicStampedRef* atomicStampedRef) {
    LOG_PROLOG();

    if (atomicStampedRef != NULL) {
        if (atomicStampedRef->ref != NULL) {
            LOG_WARN("Freeing this (%x) reference is the responsibility of the allocator of this reference", atomicStampedRef->ref);
        }

        my_free(atomicStampedRef);
    } else {
        LOG_ERROR("Error freeing NULL ptr");
    }

    LOG_EPILOG();
}

void* getRef(AtomicStampedRef* atomicStampedRef) {
    LOG_PROLOG();

    void* ref = NULL;
    if (atomicStampedRef == NULL) {
        LOG_ERROR("Null value for atomic stamped reference");
    } else {
        ref = atomicStampedRef->ref;
    }

    LOG_EPILOG();
    return ref;
}

int getStamp(AtomicStampedRef* atomicStampedRef){
    LOG_PROLOG();

    int stamp = STAMP_INVALID_VAL;
    if (atomicStampedRef == NULL) {
        LOG_ERROR("Null value for atomic stamped reference");
    } else {
        stamp = atomicStampedRef->stamp;
    }

    LOG_EPILOG();
    return stamp;
}

bool cmpAndSet(AtomicStampedRef** currentRef,
               void* expectedRef,
               void* newRef,
               int expectedStamp,
               int newStamp,
               int threadIndex) {
    LOG_PROLOG();

    bool success = false;
    if ((currentRef != NULL) && (*currentRef != NULL)) {

        AtomicStampedRef* copy = *currentRef;
        if ((expectedRef == getRef(*currentRef)) && (expectedStamp == getStamp(*currentRef))) {
            if ((newRef == getRef(*currentRef)) && (newStamp == getStamp(*currentRef))) {
                // Same value as current
                success = true;
            } else {
                AtomicStampedRef* tmp = createAtomicStampedRef(newRef, newStamp);
                if (tmp != NULL) {
                    // It uses memcmp to compare current and expected references and then uses
                    // memcpy to copy the value from desired reference to current reference.
                    LOG_DEBUG("Before CAS currentRef = %x, copy = %x", *currentRef, copy);
                    success = atomic_compare_exchange_strong(currentRef, &copy, tmp);
                    if (!success) {
                        LOG_INFO("CAS Unsuccessful");

                        //atomic CAS failed so free the temp ref-int pair
                        destroyAtomicStampedRef(tmp);
                        tmp = NULL;
                    } else {
                        LOG_INFO("CAS successful");
                        LOG_DEBUG("currentRef = %x, copy = %x", *currentRef, copy);

                        //atomic CAS successful so free the old ref-int pair
                        destroyAtomicStampedRef(copy);
                        copy = NULL;
                    }
                } else {
                    LOG_ERROR("Unable to create a ref int pair");
                }
            }
        }
    } else {
        LOG_ERROR("Invalid atomic reference for CAS");
    }

    LOG_EPILOG();
    return success;
}
