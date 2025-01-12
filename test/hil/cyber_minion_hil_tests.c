#include <TM4C123GH6PM.h>
#include "cyber_minion.h"
#include "unity_config.h" 
#include "unity.h"
#include "Mocktm4c_bsp_mocks.h"

/*..........................................................................*/
/* 	tests in this file can send a frame of data via UART that UART_echo_and_test_results.py will echo back */

/* 	
		// send a frame indicating we want the python program to echo the next frame using 0x7E 
		// 0x7E is the ~ char, which isn't frequently used 
		UART_SendChar(0x7E); 
		
		// then send the frame to echo
		UART_SendChar(frame); 
		
		// baud rate of 9600 --> .104ms / bit. 1 frame is 10 bits --> 1ms per frame. 
		// small delay so UART data has time to be sent to python and back
		// (i found thru trial that 5ms is the minimum needed to guarantee 
		//  the UART data can go back and forth and trigger the interrupt
		//  before the test assertions)
		delay_10ms(5);
*/
void hilTestHelper_trigger_UART_interrupt_with_frame(char frame) { 
	UART_SendChar(0x7E); 
	UART_SendChar(frame); 
	delay_1ms(5); 
}

void hilTestHelper_UART0_Handler_set_up_and_call_and_assert(
	char frame, 
	unsigned short expected_compare_x, 
	unsigned short expected_compare_y
) {
	current_state = SCANNING; 
	TEST_ASSERT_EQUAL_INT(SCANNING, current_state); 
	set_sp_Expect(&stack[40] - 8); 
	exit_interrupt_Expect(); 
	
	hilTestHelper_trigger_UART_interrupt_with_frame(frame); 
	
	TEST_ASSERT_EQUAL_INT(WAITING, current_state); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_x, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_x, PWM0->_0_CMPA); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_y, compare_y);
	TEST_ASSERT_EQUAL_UINT16(expected_compare_y, PWM0->_1_CMPA); 
}

void setUp(void) {}

void tearDown(void) {}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_UINT(5, 2 + 3);
}

void hilTest_set_compare_x_setsLocalCompareValueAndGenerator_when_validCompareValuePassed(void) {
	unsigned short expected_compare = 1500; 
	set_compare_x(expected_compare); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, PWM0->_0_CMPA); 
	
	expected_compare = 2000;
	set_compare_x(expected_compare); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, PWM0->_0_CMPA);
}

void hilTest_set_compare_x_staysWithinBounds_when_invalidCompareValuePassed(void) {
	unsigned short expected_compare = 1500; 
	set_compare_x(expected_compare); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, PWM0->_0_CMPA); 
	
	expected_compare = MAXIMUM_COMPARE; 
	set_compare_x(MAXIMUM_COMPARE + 1);
	TEST_ASSERT_EQUAL_UINT16(expected_compare, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, PWM0->_0_CMPA); 
	
	expected_compare = MINIMUM_COMPARE; 
	set_compare_x(MINIMUM_COMPARE - 1);
	TEST_ASSERT_EQUAL_UINT16(expected_compare, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare, PWM0->_0_CMPA); 
}

void hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled(void) {
	current_state = WAITING; 
	char frame = 0x80; 
	set_sp_Expect(&stack[40] - 8); 
	exit_interrupt_Expect(); 
	TEST_ASSERT_EQUAL_INT_MESSAGE(WAITING, current_state, "setup: state"); 
	
	hilTestHelper_trigger_UART_interrupt_with_frame(frame); 
	
	// check: clears interrupt flag
	// clearing the interrupt flag should clear the RXRIS bit in the UARTRIS register
	TEST_ASSERT_FALSE_MESSAGE(UART0->RIS & (1 << 4), "UARTRIS"); 
	// clearing the interrupt flag should clear the RXMIS bit in the UARTMIS register
	TEST_ASSERT_FALSE_MESSAGE(UART0->MIS & (1 << 4), "UARTMIS");		
	TEST_ASSERT_EQUAL_INT_MESSAGE(SCANNING, current_state, "current_state"); 
}

void hilTest_UART0_Handler_changesStateToWaiting_and_setsServoPositions_andAlternatesStackPointer_whenUARTFrameMSBDisabled(void) {
	set_compare_x(MIDDLE_COMPARE);
	set_compare_y(MAXIMUM_COMPARE);
	unsigned short expected_compare_x = MIDDLE_COMPARE; 
	unsigned short expected_compare_y = MAXIMUM_COMPARE;
	TEST_ASSERT_EQUAL_UINT16(expected_compare_x, compare_x); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_x, PWM0->_0_CMPA); 
	TEST_ASSERT_EQUAL_UINT16(expected_compare_y, compare_y);
	TEST_ASSERT_EQUAL_UINT16(expected_compare_y, PWM0->_1_CMPA); 

	char frame = 0x00;
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x11;
	expected_compare_x -= SMALL_COMPARE_INCREMENT;
	expected_compare_y -= SMALL_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x22;
	expected_compare_x -= MEDIUM_COMPARE_INCREMENT;
	expected_compare_y -= MEDIUM_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x33;
	expected_compare_x -= LARGE_COMPARE_INCREMENT;
	expected_compare_y -= LARGE_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x44;
	expected_compare_x += SMALL_COMPARE_INCREMENT;
	expected_compare_y += SMALL_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x55;
	expected_compare_x += MEDIUM_COMPARE_INCREMENT;
	expected_compare_y += MEDIUM_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y); 
	
	frame = 0x66;
	expected_compare_x += LARGE_COMPARE_INCREMENT;
	expected_compare_y += LARGE_COMPARE_INCREMENT; 
	hilTestHelper_UART0_Handler_set_up_and_call_and_assert(frame, expected_compare_x, expected_compare_y);
}

int main(void) {
	PLL_init();
	UART0_and_servos_init();
	SysTick_init();
	enable_interrupts();
	UNITY_BEGIN();
	RUN_TEST(test_sanity); 
	RUN_TEST(hilTest_set_compare_x_setsLocalCompareValueAndGenerator_when_validCompareValuePassed);
	RUN_TEST(hilTest_set_compare_x_staysWithinBounds_when_invalidCompareValuePassed); 
	RUN_TEST(hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled);
	RUN_TEST(hilTest_UART0_Handler_changesStateToWaiting_and_setsServoPositions_andAlternatesStackPointer_whenUARTFrameMSBDisabled);
	(void) UNITY_END();
	for(;;);
}


/*..........................................................................*/
/* fault handler called from the exception handlers in the startup code */
void assert_failed(char const * const module, int const loc) {
	(void)module;
	(void)loc;
	UART_SendString("exception! oopsies!"); 
	for (;;) {}
}
