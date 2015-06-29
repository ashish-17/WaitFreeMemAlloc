#include"utils.h"
#include <pthread.h>
#include <time.h>

extern int errno;

typedef struct {
	unsigned int isDirty : 1;
	unsigned int unusedBits:31;
} hp_malloc_header;

void * my_malloc(size_t nBytes) {
	LOG_PROLOG();

    size_t sizeOfBlock = nBytes + sizeof(hp_malloc_header);
	void* ptr = malloc(sizeOfBlock);
	if (ptr != NULL) {
        memset(ptr, 0, sizeof(hp_malloc_header));

        LOG_DEBUG("Pointer to allocated memory(%d bytes) %x", sizeOfBlock, ptr);

        char* tmp = ptr;
        tmp += sizeof(hp_malloc_header);
        ptr = tmp;
        LOG_DEBUG("Pointer(%d bytes) returned to caller %x", nBytes, ptr);
	} else {
        int err = errno;        // Preserve the errno from the failed malloc().
        LOG_WARN("malloc failed : %s", strerror(err)); // Log the error corresponding to the failed malloc()
	}

	LOG_EPILOG();
	return ptr;
}

void my_free(void *ptr) {
	LOG_PROLOG();
    LOG_DEBUG("Pointer to be freed %x", ptr);

	char* tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
    LOG_DEBUG("Pointer to actual memory being freed %x", tmp);

	free(tmp);
	ptr = NULL;

	LOG_EPILOG();
}

bool isDirty(void* ptr) {
	LOG_PROLOG();
	LOG_DEBUG("Memory block to be checked dirty %x", ptr);

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	LOG_DEBUG("Actual Memory block that will be checked dirty %x", tmp);

	hp_malloc_header *header = (hp_malloc_header*)tmp;
	bool flag = header->isDirty;
	LOG_DEBUG("Value of dirty bit %d", flag);

	LOG_EPILOG();
	return flag;
}

void setDirty(void *ptr, bool isDirty) {
	LOG_PROLOG();
	LOG_DEBUG("Memory block to be set(%d) dirty %x", isDirty, ptr);

	char *tmp = ptr;
	tmp -= sizeof(hp_malloc_header);
	LOG_DEBUG("Actual Memory block that will be set(%d) dirty %x", isDirty, tmp);

	hp_malloc_header *header = (hp_malloc_header*)tmp;
	LOG_DEBUG("Old value of dirty bit %d", header->isDirty);

	header->isDirty = isDirty;
	LOG_EPILOG();
}

/*
    Test cases for malloc and related utilities.
*/
int testMalloc() {
    LOG_INIT_CONSOLE();
	LOG_INIT_FILE();

    bool pass = false;

    //Test 1
    LOG_INFO("Test case 1 (Try to allocate mem for 10 chars and R/W data to that memory)");
    char *ptr_test1 = my_malloc(sizeof(char) * 10);
    if (ptr_test1 != NULL) {
        char test1_data[] = "123456789";
        strncpy(ptr_test1, test1_data, sizeof(test1_data) - 1);
        if (strcmp(ptr_test1, test1_data) == 0) {
            pass = true;
        }

        my_free(ptr_test1);
    }

    if (pass == true) {
        LOG_INFO("Test case 1 pass");
    } else {
        LOG_INFO("Test case 1 fail");
    }

    //Test 2
    pass = false;
    LOG_INFO("Test case 2 (Try to allocate mem for 10 int and R/W data to that memory)");
    int *ptr_test2 = my_malloc(sizeof(int) * 10);
    if (ptr_test2 != NULL) {
        int test2_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        memcpy(ptr_test2, test2_data, sizeof(test2_data));

        for (int i=0; i < 10; ++i) {
            if (*(ptr_test2 + i) != test2_data[i]) {
                pass= false;
                break;
            } else if (pass == false) {
                pass = true;
            }
        }

        my_free(ptr_test2);
    }

    if (pass == true) {
        LOG_INFO("Test case 2 pass");
    } else {
        LOG_INFO("Test case 2 fail");
    }

    //Test 3
    pass = false;
    LOG_INFO("Test case 3 (Try to allocate very large chunk of memory)");
    char *ptr_test3 = my_malloc(200000000000UL);
    if (ptr_test3 != NULL) {

        my_free(ptr_test3);
    }

    pass = true;

    LOG_INFO("Manually verify if error information is displayed above");

    // Test 4
    pass = false;
    LOG_INFO("Test case 4 (Try to set the dirty bit on memory block)");
    char *ptr_test4 = my_malloc(sizeof(char) * 10);
    if (ptr_test4 != NULL) {
        char test4_data[] = "123456789";
        strncpy(ptr_test4, test4_data, sizeof(test4_data) - 1);

        bool originalDirtyBit = isDirty(ptr_test4);
        if (originalDirtyBit == false) {
            bool testValueOfDirtyBit = true;
            setDirty(ptr_test4, testValueOfDirtyBit);
            if (testValueOfDirtyBit == isDirty(ptr_test4)) {
                setDirty(ptr_test4, originalDirtyBit);
                if (originalDirtyBit == isDirty(ptr_test4)) {
                    pass = true;
                }
            }
        }

        my_free(ptr_test4);
    }

    if (pass == true) {
        LOG_INFO("Test case 4 pass");
    } else {
        LOG_INFO("Test case 4 fail");
    }

	LOG_CLOSE();
    return 0;
}


