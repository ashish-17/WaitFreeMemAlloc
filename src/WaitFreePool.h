#ifndef WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_
#define WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_

#include "stdio.h"
#include "Stack.h"
#include "Chunk.h"
#include "LocalPool.h"
#include "FreePool.h"
#include "FullPool.h"
#include "pool.h"
#include "Block.h"

typedef struct {
	bool needHelp;
	int timestamp;
} Helper;

typedef struct{
	Helper *helpers;
	int numberOfHelpers;
} Announce;

typedef struct {
	int noOfOps;
	int lastDonated;
} Donor;

typedef struct{
	Donor *donors;
	int noOfDonors;
} Info;

#endif /* WAITFREEMEMALLOC_SRC_WAITFREEPOOL_H_ */
