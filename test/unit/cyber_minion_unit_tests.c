#include "unity.h"
#include "cyber_minion.h"
#include "mocks/Mocktm4c_bsp.h"

void setUp(void) {
    // Optional: Initialize resources before each test
}

void tearDown(void) {
    // Optional: Clean up resources after each test
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_INT(5, 2 + 3);
}

void test_sleep_position_centersHorizontalServo_and_maximizesVerticalServo(void) {
	compare_x = MINIMUM_COMPARE; 
	compare_y = MINIMUM_COMPARE; 
	
	TEST_ASSERT_EQUAL_UINT16(compare_x, MINIMUM_COMPARE); 
	TEST_ASSERT_EQUAL_UINT16(compare_y, MINIMUM_COMPARE); 
	
	sleep_position(); 
	TEST_ASSERT_EQUAL_UINT16(compare_x, MIDDLE_COMPARE); 
	TEST_ASSERT_EQUAL_UINT16(compare_y, MAXIMUM_COMPARE); 
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_sanity);
	RUN_TEST(test_sleep_position_centersHorizontalServo_and_maximizesVerticalServo); 
	return UNITY_END();
}
