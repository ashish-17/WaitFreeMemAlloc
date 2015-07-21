#include "AtomicStampedRef.h"
#include "HazardPointer.h"

#define STAMP_INVALID_VAL -1

typedef struct RefIntPair {
    void* ref;
    int stamp;
} RefIntPair;

struct AtomicStampedRef {
    RefIntPair* pair;
};

RefIntPair* createRefIntPair(void* ref, int stamp);
void* destroyRefIntPair(RefIntPair* pair);

RefIntPair* createRefIntPair(void* ref, int stamp) {
    LOG_PROLOG();

    RefIntPair* pair = (RefIntPair*)my_malloc(sizeof(RefIntPair));
    if (pair != NULL) {
        pair->ref = ref;
        pair->stamp = stamp;
    } else {
        LOG_ERROR("Error while allocating memory to ref int pair");
    }

    LOG_EPILOG();
    return pair;
}

void* destroyRefIntPair(RefIntPair* pair) {
    LOG_PROLOG();

    void* ref = NULL;

    if (pair != NULL) {
        if (pair->ref != NULL) {
            LOG_WARN("Freeing this (%x) reference is the responsibility of the allocator of this reference", pair->ref);
            ref = pair->ref;
        }

        my_free(pair);
        pair = NULL;
    } else {
        LOG_ERROR("Error freeing NULL ptr");
    }

    LOG_EPILOG();

    return ref;
}

AtomicStampedRef* createAtomicStampedRef(void* ref, int stamp) {
    LOG_PROLOG();

    AtomicStampedRef* atomicStampedRef = (AtomicStampedRef*)my_malloc(sizeof(AtomicStampedRef));
    if (atomicStampedRef != NULL) {
        atomicStampedRef->pair = createRefIntPair(ref, stamp);
    } else {
        LOG_ERROR("Error while allocating memory to atomic stamped reference");
    }

    LOG_EPILOG();
    return atomicStampedRef;
}

void* destroyAtomicStampedRef(AtomicStampedRef* atomicStampedRef) {
    LOG_PROLOG();

    void* ref = NULL;
    if (atomicStampedRef != NULL) {
        ref = destroyRefIntPair(atomicStampedRef->pair);

        my_free(atomicStampedRef);
        atomicStampedRef = NULL;
    } else {
        LOG_ERROR("Error freeing NULL ptr");
    }

    LOG_EPILOG();
    return ref;
}

void* getRef(AtomicStampedRef* atomicStampedRef) {
    LOG_PROLOG();

    void* ref = NULL;
    if ((atomicStampedRef == NULL) || (atomicStampedRef->pair == NULL)) {
        LOG_ERROR("Null value for atomic stamped reference");
    } else {
        ref = atomicStampedRef->pair->ref;
    }

    LOG_EPILOG();
    return ref;
}

int getStamp(AtomicStampedRef* atomicStampedRef){
    LOG_PROLOG();

    int stamp = STAMP_INVALID_VAL;
    if ((atomicStampedRef == NULL) || (atomicStampedRef->pair == NULL)) {
        LOG_ERROR("Null value for atomic stamped reference");
    } else {
        stamp = atomicStampedRef->pair->stamp;
    }

    LOG_EPILOG();
    return stamp;
}

CASResult cmpAndSet(AtomicStampedRef* currentRef,
                   void* expectedRef,
                   void* newRef,
                   int expectedStamp,
                   int newStamp) {
    LOG_PROLOG();

    CASResult result = {false, newRef};
    if ((currentRef != NULL) && (currentRef->pair != NULL)) {

        RefIntPair* copyPair = currentRef->pair;
        if ((expectedRef == getRef(currentRef)) && (expectedStamp == getStamp(currentRef))) {
            if ((newRef == getRef(currentRef)) && (newStamp == getStamp(currentRef))) {
                // Same value as current
                result.success = true;
            } else {
                RefIntPair* tmpPair = createRefIntPair(newRef, newStamp);
                if (tmpPair != NULL) {
                    // It uses memcmp to compare current and expected references and then uses
                    // memcpy to copy the value from desired reference to current reference.
                    LOG_DEBUG("Before CAS currentRef = %x, copy = %x", currentRef->pair, copyPair);
                    result.success = atomic_compare_exchange_strong(&(currentRef->pair), &copyPair, tmpPair);
                    if (!result.success) {
                        LOG_INFO("CAS Unsuccessful");

                        //atomic CAS failed so free the temp ref-int pair
                        result.ref = destroyRefIntPair(tmpPair);
                        tmpPair = NULL;
                    } else {
                        LOG_INFO("CAS successful");
                        LOG_DEBUG("currentRef = %x, copy = %x", currentRef->pair, copyPair);

                        //atomic CAS successful so free the old ref-int pair
                        result.ref = destroyRefIntPair(copyPair);
                        copyPair = NULL;
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
    return result;
}
