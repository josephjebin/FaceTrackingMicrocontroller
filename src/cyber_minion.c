#include "cyber_minion.h"
#include "tm4c_bsp.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MINIMUM_COMPARE (unsigned short) 1249
#define MIDDLE_COMPARE (unsigned short)1874 
#define MAXIMUM_COMPARE (unsigned short) 2499
// mg995 has a range of 120 degrees and COMPARE values range from 1250 to 2500. (2500 - 1250 = 1250)
// 120 degrees / 1250 = .096 degrees per COMPARE increment 
// (.096 degrees / COMPARE increment) * 10 = .96 degrees
#define SMALL_COMPARE_INCREMENT (unsigned short) 10
// (.096 degrees / COMPARE increment) * 100 = 9.6 degrees
#define MEDIUM_COMPARE_INCREMENT (unsigned short) 100
// (.096 degrees / COMPARE increment) * 200 = 19.2 degrees
#define LARGE_COMPARE_INCREMENT (unsigned short) 200
#define STACK_SIZE 40

typedef enum {
    WAITING,  // Minion is either sleeping or locked on a face and waiting for movement
    SCANNING  // Scanning for a face
} State;


State current_state = WAITING;  
// not worried about unsigned short overflowing. MAXIMUM_COMPARE is 2500. LARGE_COMPARE_INCREMENT is 250. 2500 + 250 = 2750
// unsigned shorts have a maximum value of 65,535
// not worried about negative values. MINIMUM_COMPARE is 1250. LARGE_COMPARE_INCREMENT is 250. 1250 - 250 = 1000
unsigned short compare_x = MIDDLE_COMPARE; 
unsigned short compare_y = MAXIMUM_COMPARE; 

uint32_t stack1[40];
uint32_t stack2[40];

/* 	intentionally out of bounds. to use the pointer, decrement it and then fill it.
*		this way, the stack points to the last filled element
*/
uint32_t * volatile sp1 = &stack1[STACK_SIZE]; 
uint32_t * volatile sp2 = &stack2[STACK_SIZE]; 
bool using_stack1 = false; 

void main_loop(void) {
	while (1) {
		switch(current_state) {
			case WAITING: 
				WaitForInterrupt();
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
	__asm volatile (
		"MOV R12, LR	\n"   // Save LR to r12
	);
	
	//don't need to check receiver FIFO since UART handler is only triggered when FIFO has data
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
		__asm volatile (
			"LDR r0, =sp2        	\n"   // Load the address of next_sp into r0
		); 
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
		__asm volatile (
			"LDR r0, =sp1        	\n"   // Load the address of next_sp into r0
		); 
	}
	
	__asm volatile (
		"LDR r0, [r0]             \n"   // Dereference sp1 or sp2 to get the next stack pointer
	); 
	
	__asm volatile (
    "MOV sp, r0               \n"   // Move next stack pointer into sp
	); 
	
	__asm volatile (
    "MOV LR, R12						\n"   // Restore LR from r12
    "BX LR                 	\n"   // Return from the interrupt
	);
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

