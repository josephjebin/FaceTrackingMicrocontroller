#include "cyber_minion.h"
#include "tm4c_bsp.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define STACK_SIZE 40
static uint32_t stack1[STACK_SIZE];
static uint32_t stack2[STACK_SIZE];

#if defined(UNIT_TEST) || defined (HIL_TEST)
State current_state = WAITING;  
unsigned short compare_x = MIDDLE_COMPARE; 
unsigned short compare_y = MAXIMUM_COMPARE; 
uint32_t * volatile sp1 = &stack1[STACK_SIZE]; 
uint32_t * volatile sp2 = &stack2[STACK_SIZE]; 

#else 
static State current_state = WAITING;  
/* not worried about unsigned short overflowing. MAXIMUM_COMPARE is 2500. LARGE_COMPARE_INCREMENT is 250. 2500 + 250 = 2750
	 unsigned shorts have a maximum value of 65,535
	 not worried about negative values. MINIMUM_COMPARE is 1250. LARGE_COMPARE_INCREMENT is 250. 1250 - 250 = 1000 
*/
static unsigned short compare_x = MIDDLE_COMPARE; 
static unsigned short compare_y = MAXIMUM_COMPARE; 

/* 	intentionally out of bounds. to use the pointer, decrement it and then fill it.
		this way, the stack points to the last filled element
*/
static uint32_t * volatile sp1 = &stack1[STACK_SIZE]; 
static uint32_t * volatile sp2 = &stack2[STACK_SIZE]; 

#endif

static bool using_stack1 = false; 

void main_loop(void) {
	while (1) {
		switch(current_state) {
			case WAITING: 
				UART_SendChar('a'); 
				wait_for_interrupt(); 
				break; 
			case SCANNING: 
				scan(); 
				sleep_position(); 
				current_state = WAITING; 
				break;
			default:
				sleep_position(); 
				current_state = WAITING; 
        break;
		}
	}
}

void UART0_Handler(void) {	
	// don't need to check receiver FIFO since UART handler is only triggered when FIFO has data
	uint8_t data = get_UART0_interrupt_data(); 

	if (data & 0x80) {
		current_state = SCANNING;
	} else {
		current_state = WAITING; 
		
		// least significant nibble controls vertical servo 
		// nibble will be in the input format:
		// 0: no change, 1/2/3: small/medium/large negative jump, 4/5/6: small/medium/large positive jump
		switch(data & 0x7){
			case 6: 
				set_compare_y(compare_y + LARGE_COMPARE_INCREMENT); 
				break; 
			case 5: 
				set_compare_y(compare_y + MEDIUM_COMPARE_INCREMENT); 
				break; 
			case 4: 
				set_compare_y(compare_y + SMALL_COMPARE_INCREMENT); 
				break; 
			case 3: 
				set_compare_y(compare_y - LARGE_COMPARE_INCREMENT); 
				break; 
			case 2: 
				set_compare_y(compare_y - MEDIUM_COMPARE_INCREMENT); 
				break; 
			case 1: 
				set_compare_y(compare_y - SMALL_COMPARE_INCREMENT); 
				break; 
			case 0: 
			default: 
				break; 
		}
		
		// bits 6-4 control horiztonal servo
		switch((data & 0x70) >> 4) {
			case 6: 
				set_compare_x(compare_x + LARGE_COMPARE_INCREMENT); 
				break; 
			case 5: 
				set_compare_x(compare_x + MEDIUM_COMPARE_INCREMENT); 
				break; 
			case 4: 
				set_compare_x(compare_x + SMALL_COMPARE_INCREMENT); 
				break; 
			case 3: 
				set_compare_x(compare_x - LARGE_COMPARE_INCREMENT); 
				break; 
			case 2: 
				set_compare_x(compare_x - MEDIUM_COMPARE_INCREMENT); 
				break; 
			case 1: 
				set_compare_x(compare_x - SMALL_COMPARE_INCREMENT); 
				break; 
			case 0: 
			default: 
				break; 
		}
	}
	
	// context switch logic: fabricate a stack that the processor will use while jumping to &main_loop
	if(using_stack1) {
		sp2 = &stack2[STACK_SIZE]; 
		*(--sp2) = (1U << 24);  /* xPSR */
		*(--sp2) = (uint32_t)&main_loop; /* PC */
		*(--sp2) = 0x0000000EU; /* LR  */
		*(--sp2) = 0x0000000CU; /* R12 */
		*(--sp2) = 0x00000003U; /* R3  */
		*(--sp2) = 0x00000002U; /* R2  */
		*(--sp2) = 0x00000001U; /* R1  */
		*(--sp2) = 0x00000000U; /* R0  */
		using_stack1 = false; 
		set_sp(sp2); 
	} 
	else {
		sp1 = &stack1[STACK_SIZE]; 
		*(--sp1) = (1U << 24);  /* xPSR */
		*(--sp1) = (uint32_t)&main_loop; /* PC */
		*(--sp1) = 0x0000000EU; /* LR  */
		*(--sp1) = 0x0000000CU; /* R12 */
		*(--sp1) = 0x00000003U; /* R3  */
		*(--sp1) = 0x00000002U; /* R2  */
		*(--sp1) = 0x00000001U; /* R1  */
		*(--sp1) = 0x00000000U; /* R0  */
		using_stack1 = true;
		set_sp(sp1); 
	}
	
	exit_interrupt(); 
}

void scan() {
	set_compare_y(MIDDLE_COMPARE - MEDIUM_COMPARE_INCREMENT); 
	set_compare_x(MINIMUM_COMPARE);
	delay_10ms(100);
	while (compare_x < MAXIMUM_COMPARE) {
		set_compare_x(compare_x + SMALL_COMPARE_INCREMENT); 
		delay_10ms(2);
	}
	
	set_compare_y(compare_y - LARGE_COMPARE_INCREMENT);
	delay_10ms(2);
	
	while (compare_x > MINIMUM_COMPARE) {
		set_compare_x(compare_x - SMALL_COMPARE_INCREMENT); 
		delay_10ms(2);
	}
	
	set_compare_y(compare_y - LARGE_COMPARE_INCREMENT);
	delay_10ms(2);
	
	while (compare_x < MAXIMUM_COMPARE) {
		set_compare_x(compare_x + SMALL_COMPARE_INCREMENT); 
		delay_10ms(2);
	}
}

void sleep_position(void) {
	set_compare_x(MIDDLE_COMPARE);
	set_compare_y(MAXIMUM_COMPARE);
}	

void set_compare_x(unsigned short new_compare_x) {
	new_compare_x = (new_compare_x < MAXIMUM_COMPARE) ? new_compare_x : MAXIMUM_COMPARE;
	new_compare_x = (new_compare_x > MINIMUM_COMPARE) ? new_compare_x : MINIMUM_COMPARE;
	compare_x = new_compare_x; 
	set_PWM0_generator0_CMPA(compare_x); 
}

void set_compare_y(unsigned short new_compare_y) {
	new_compare_y = (new_compare_y < MAXIMUM_COMPARE) ? new_compare_y : MAXIMUM_COMPARE;
	new_compare_y = (new_compare_y > MINIMUM_COMPARE) ? new_compare_y : MINIMUM_COMPARE;
	compare_y = new_compare_y; 
	set_PWM0_generator1_CMPA(compare_y);
}
