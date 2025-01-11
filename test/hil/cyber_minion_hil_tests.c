#include <TM4C123GH6PM.h>
#include "cyber_minion.h"
#include "unity_config.h" 
#include "unity.h"
#include "Mocktm4c_bsp_mocks.h"

/*..........................................................................*/
/* tests in this file can send a frame of data via UART that UART_echo_and_test_results.py will echo */
void setUp(void) {
	using_stack1 = false; 
}

void tearDown(void) {
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_UINT(6, 2 + 3);
}

void hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled(void) {
	current_state = WAITING; 
	char frame = 0x80; 
	set_sp_Expect(sp1); 
	exit_interrupt_Expect(); 
	TEST_ASSERT_EQUAL_INT_MESSAGE(WAITING, current_state, "setup: state"); 
	
	// send command to python program to echo frame back
	UART_SendChar(0x7E); 
	UART_SendChar(frame); 
	// small delay so UART data has time to be sent to python and back
	delay_10ms(2); 
	
	// check: clears interrupt flag
	// clearing the interrupt flag should clear the RXRIS bit in the UARTRIS register
	TEST_ASSERT_FALSE_MESSAGE(UART0->RIS & (1 << 4), "UARTRIS"); 
	// clearing the interrupt flag should clear the RXMIS bit in the UARTMIS register
	TEST_ASSERT_FALSE_MESSAGE(UART0->MIS & (1 << 4), "UARTMIS");		
	TEST_ASSERT_EQUAL_INT_MESSAGE(SCANNING, current_state, "current_state"); 
}

void hilTest_UART0_Handler_changesStateToWaiting_and_setsServoPositions_andAlternatesStackPointer_whenUARTFrameMSBDisabled(void) {
	current_state = SCANNING; 
	compare_x = MIDDLE_COMPARE; 
	compare_y = MAXIMUM_COMPARE; 
	char frame = 0x11;
	set_sp_Expect(sp1); 
	exit_interrupt_Expect(); 
	TEST_ASSERT_EQUAL_INT(SCANNING, current_state); 
	
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

int main(void) {
	PLL_init();
	UART0_and_servos_init();
	SysTick_init();
	enable_interrupts();
	UNITY_BEGIN();
	RUN_TEST(hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled);
	RUN_TEST(hilTest_set_compare_x_setsLocalCompareValueAndGenerator_when_validCompareValuePassed);
	RUN_TEST(hilTest_set_compare_x_setsLocalCompareValueAndGenerator_when_validCompareValuePassed); 
	RUN_TEST(hilTest_set_compare_x_staysWithinBounds_when_invalidCompareValuePassed); 
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
