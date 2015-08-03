/*
 * TestUtils.c
 *
 *  Created on: 13-Jul-2015
 *      Author: architaagarwal
 */

/*
    Test cases for malloc and related utilities.
 */

#include "../utils.h"
#include "../mini-logger/logger.h"
#include <string.h>

int utilsmain() {
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
	char *ptr_test3 = my_malloc(200000000000000UL);
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

	// Test 5
/*	pass = false;
	LOG_INFO("Test case 5 (Try to set the threadIndex on memory block)");
	char *ptr_test5 = my_malloc(sizeof(char) * 10);
	if (ptr_test5!= NULL) {
		char test5_data[] = "123456789";
		strncpy(ptr_test5, test5_data, sizeof(test5_data) - 1);

		int originalThreadIndex = getIndex(ptr_test5);
		if (originalThreadIndex == -1) {
			int testValueOfThreadIndex = 5;
			setIndex(ptr_test5, testValueOfThreadIndex);
			if (testValueOfThreadIndex == getIndex(ptr_test5)) {
				setIndex(ptr_test5, originalThreadIndex);
				if (originalThreadIndex == getIndex(ptr_test5)) {
					pass = true;
				}
			}
		}

		my_free(ptr_test5);
	}

	if (pass == true) {
		LOG_INFO("Test case 5 pass");
	} else {
		LOG_INFO("Test case 5 fail");
	}*/

	LOG_CLOSE();
	return 0;
}






