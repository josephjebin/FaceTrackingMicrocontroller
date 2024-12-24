#include "unity.h"

void setUp(void) {
    // Optional: Initialize resources before each test
}

void tearDown(void) {
    // Optional: Clean up resources after each test
}

void test_add(void) {
    TEST_ASSERT_EQUAL_INT(5, 2 + 3);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add);
    return UNITY_END();
}
