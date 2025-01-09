#include "tm4c_bsp.h"
#include "cyber_minion.h"
#include "unity_config.h" 
#include "unity.h"

/*..........................................................................*/
/* tests in this file can send a frame of data via UART that UART_echo_and_test_results.py will echo */

void setUp(void) {
	
}

void tearDown(void) {
	// Optional: Clean up resources after each test
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_UINT(6, 2 + 3);
}

void hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled(void) {
	current_state = WAITING; 
	char frame = 0x80; 
	
	TEST_ASSERT_EQUAL_INT(WAITING, current_state); 
	
	UART_SendChar(frame); 
	main_loop(); 
	
	// check: clears interrupt flag
	
	// check: sets servos
	
	// check: sets new stack pointer
	
	// check: calls exit_interrupt()
}

int main(void) {
    PLL_init();
    UART0_and_servos_init();
    SysTick_init();
    enable_interrupts();
    UNITY_BEGIN();
    RUN_TEST(test_sanity);
    return UNITY_END();
}


/*..........................................................................*/
/* fault handler called from the exception handlers in the startup code */
void assert_failed(char const * const module, int const loc) {
	(void)module;
	(void)loc;
	UART_SendString("init fail"); 
	for (;;) {}
}
