#include <TM4C123GH6PM.h>
#include "cyber_minion.h"
#include "unity_config.h" 
#include "unity.h"



#include "Mocktm4c_bsp_mocks.h"
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
	set_sp_Expect(sp1); 
	exit_interrupt_Expect(); 
	TEST_ASSERT_EQUAL_INT(WAITING, current_state); 
	
	UART_SendChar(frame); 
	// check: clears interrupt flag
	// clearing the interrupt flag should clear the RXRIS bit in the UARTRIS register
	TEST_ASSERT_FALSE(UART0->RIS & (1 << 4)); 
	// clearing the interrupt flag should clear the RXMIS bit in the UARTMIS register
	TEST_ASSERT_FALSE(UART0->MIS & (1 << 4));
	UART_SendChar('x'); 
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
    RUN_TEST(hilTest_UART0_Handler_changesStateToScanning_whenUARTFrameMSBIsEnabled);
    return UNITY_END();
}


/*..........................................................................*/
/* fault handler called from the exception handlers in the startup code */
void assert_failed(char const * const module, int const loc) {
	(void)module;
	(void)loc;
	UART_SendString("exception! oopsies!"); 
	for (;;) {}
}
