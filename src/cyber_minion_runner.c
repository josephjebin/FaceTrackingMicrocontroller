#include "cyber_minion.h"
#include "tm4c_bsp.h"

int main(void) {
	PLL_init(); 
	UART0_and_servos_init(); 
	SysTick_init(); 
	EnableInterrupts(); 
	sleep_position(); 
	main_loop(); 
}