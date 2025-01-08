#include "tm4c_bsp.h"
#include "cyber_minion.h"
#include "unity_config.h" 
#include "unity.h"

void setUp(void) {
	
}

void tearDown(void) {
	// Optional: Clean up resources after each test
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_INT(6, 2 + 3);
}

int main(void) {
    PLL_init();
    UART0_and_servos_init();
    SysTick_init();
    enable_interrupts();
		UART_SendChar('X'); // Before UNITY_BEGIN()

    UNITY_BEGIN();
		UNITY_OUTPUT_CHAR('Y'); // Should directly map to UART_SendChar

    UART_SendString(" After UNITY_BEGIN");
    // RUN_TEST(test_sanity);
    UART_SendString(" world");

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
