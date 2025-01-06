#include "unity.h"
#include "cyber_minion.h"
#include "mocks/Mocktm4c_bsp.h"

void setUp(void) {
    // Optional: Initialize resources before each test
}

void tearDown(void) {
    // Optional: Clean up resources after each test
}

void testHelper_UART0_Handler_set_up_and_call_and_assert(
	uint8_t UART0_frame, 
	uint32_t *sp, 
	unsigned short expected_compare_x, 
	unsigned short expected_compare_y, 
	unsigned short case_number
) {
	get_UART0_interrupt_data_ExpectAndReturn(UART0_frame); 
	set_sp_Expect(sp); 
	// case 0 doesn't set compare values
	if(case_number != 0) {
		set_PWM0_generator0_CMPA_Expect(expected_compare_x); 
		set_PWM0_generator1_CMPA_Expect(expected_compare_y); 
	}
	exit_interrupt_Expect(); 
	UART0_Handler(); 
	char test_message[] = {"Case x"}; 
	test_message[5] = (char)(case_number + '0'); 
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected_compare_x, compare_x, test_message); 
	TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected_compare_y, compare_y, test_message);
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_INT(5, 2 + 3);
}

void test_sleep_position_centersHorizontalServo_and_maximizesVerticalServo(void) {
	compare_x = MINIMUM_COMPARE; 
	compare_y = MINIMUM_COMPARE; 
	set_PWM0_generator0_CMPA_Expect(MIDDLE_COMPARE); 
	set_PWM0_generator1_CMPA_Expect(MAXIMUM_COMPARE); 
	TEST_ASSERT_EQUAL_UINT16(MINIMUM_COMPARE, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(MINIMUM_COMPARE, compare_y); 
	
	sleep_position(); 
	
	TEST_ASSERT_EQUAL_UINT16(MIDDLE_COMPARE, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(MAXIMUM_COMPARE, compare_y); 
}

void test_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled(void) {
	current_state = WAITING; 
	uint8_t UART0_frame = 0x80; 
	get_UART0_interrupt_data_ExpectAndReturn(UART0_frame); 
	set_sp_Expect(sp1); 
	exit_interrupt_Expect(); 
	TEST_ASSERT_EQUAL_INT(WAITING, current_state); 
	
	UART0_Handler(); 
	
	TEST_ASSERT_EQUAL_INT(SCANNING, current_state); 
}

// testing UART0_Handler indirectly tests the set_compare_x/y methods
void test_UART0_Handler_changesStateToWaiting_and_setsServoPositions_andAlternatesStackPointer_whenUARTFrameMSBDisabled(void) {
	current_state = SCANNING; 
	compare_x = MIDDLE_COMPARE; 
	compare_y = MAXIMUM_COMPARE; 
	unsigned short expected_compare_x = MIDDLE_COMPARE; 
	unsigned short expected_compare_y = MAXIMUM_COMPARE; 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_x, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_y, compare_y);
	
	unsigned short case_number = 0; 
	uint8_t UART0_frame = 0; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp1, expected_compare_x, expected_compare_y, case_number); 
	
	case_number = 1; 
	UART0_frame = 0x11;
	expected_compare_x -= SMALL_COMPARE_INCREMENT; 
	expected_compare_y -= SMALL_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp2, expected_compare_x, expected_compare_y, case_number); 
	
	case_number = 2; 
	UART0_frame = 0x22;
	expected_compare_x -= MEDIUM_COMPARE_INCREMENT; 
	expected_compare_y -= MEDIUM_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp1, expected_compare_x, expected_compare_y, case_number);
		
	case_number = 3; 
	UART0_frame = 0x33;
	expected_compare_x -= LARGE_COMPARE_INCREMENT; 
	expected_compare_y -= LARGE_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp2, expected_compare_x, expected_compare_y, case_number);	
		
	case_number = 4; 
	UART0_frame = 0x44;
	expected_compare_x += SMALL_COMPARE_INCREMENT; 
	expected_compare_y += SMALL_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp1, expected_compare_x, expected_compare_y, case_number);
		
	case_number = 5; 
	UART0_frame = 0x55;
	expected_compare_x += MEDIUM_COMPARE_INCREMENT; 
	expected_compare_y += MEDIUM_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp2, expected_compare_x, expected_compare_y, case_number);
		
	case_number = 6; 
	UART0_frame = 0x66;
	expected_compare_x += LARGE_COMPARE_INCREMENT; 
	expected_compare_y += LARGE_COMPARE_INCREMENT; 
	testHelper_UART0_Handler_set_up_and_call_and_assert(
		UART0_frame, sp1, expected_compare_x, expected_compare_y, case_number);
}

void set_compare_x_doesNotCompareValuesOutsideValidRange(void) {
	
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_sanity);
	RUN_TEST(test_sleep_position_centersHorizontalServo_and_maximizesVerticalServo); 
	RUN_TEST(test_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled); 
	RUN_TEST(test_UART0_Handler_changesStateToWaiting_and_setsServoPositions_andAlternatesStackPointer_whenUARTFrameMSBDisabled);
	return UNITY_END();
}
