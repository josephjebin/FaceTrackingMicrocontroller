#include "tm4c_bsp.h"
//#include "Mocktm4c_bsp_set_sp.h"
#include "cyber_minion.h"
#include "unity_uart_config.h"

void setUp(void) {
	// same set up as cyber_minion_runner.c 
	UART0_and_servos_init(); 
	SysTick_init(); 
	enable_interrupts();
}

void tearDown(void) {
    // Optional: Clean up resources after each test
}

void test_sanity(void) {
	TEST_ASSERT_EQUAL_INT(5, 2 + 3);
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_sanity); 
	return UNITY_END();
}