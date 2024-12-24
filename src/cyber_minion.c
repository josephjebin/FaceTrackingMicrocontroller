#include "TM4C123GH6PM.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define SYSCTL_RCC2_R           (*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RCC_R            (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RIS_R            (*((volatile unsigned long *)0x400FE050))

#define NVIC_ST_CTRL_R      (*((volatile uint32_t *)0xE000E010)) // SysTick Control and Status Register
#define NVIC_ST_RELOAD_R    (*((volatile uint32_t *)0xE000E014)) // SysTick Reload Value Register
#define NVIC_ST_CURRENT_R   (*((volatile uint32_t *)0xE000E018)) // SysTick Current Value Register

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

void DisableInterrupts(void); 
void EnableInterrupts(void);  
void WaitForInterrupt(void);  // low power mode

void PLL_init(void); 
void SysTick_init(void); 
void delay_10ms(unsigned long); 
void UART0_and_servos_init(void);
void main_loop(void) __attribute__((aligned(4)));
void UART0_Handler(void);
void scan(void); 
void sleep_position(void); 
void set_compare_x(short); 
void set_compare_y(short); 

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

int main(void) {
	PLL_init(); 
	UART0_and_servos_init(); 
	SysTick_init(); 
	EnableInterrupts(); 
	sleep_position(); 
	main_loop(); 
}

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
	//don't need to check receiver FIFO since UART handler is only triggered when FIFO has data
	uint8_t data = UART0->DR & 0xFF;  // Read received byte
	UART0->ICR = 0x10;               // Clear interrupt flag

	if (data & 0x80) {
		current_state = SCANNING;
	} else {
		current_state = WAITING; 
		__asm volatile (
			"MOV R12, LR	\n"   // Save LR to r12
		);
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

void set_compare_x(short new_compare_x) {
	new_compare_x = (new_compare_x < MAXIMUM_COMPARE) ? new_compare_x : MAXIMUM_COMPARE;
	new_compare_x = (new_compare_x > MINIMUM_COMPARE) ? new_compare_x : MINIMUM_COMPARE;
	compare_x = new_compare_x; 
	PWM0->_0_CMPA = compare_x; 
}

void set_compare_y(short new_compare_y) {
	new_compare_y = (new_compare_y < MAXIMUM_COMPARE) ? new_compare_y : MAXIMUM_COMPARE;
	new_compare_y = (new_compare_y > MINIMUM_COMPARE) ? new_compare_y : MINIMUM_COMPARE;
	compare_y = new_compare_y; 
	PWM0->_1_CMPA = compare_y;
}

void scan() {
	compare_y = MIDDLE_COMPARE - MEDIUM_COMPARE_INCREMENT; 
	PWM0->_1_CMPA = compare_y;
	compare_x = MINIMUM_COMPARE;
	delay_10ms(100);
	while (compare_x < MAXIMUM_COMPARE) {
		compare_x += SMALL_COMPARE_INCREMENT; 
		PWM0->_0_CMPA = compare_x;
		delay_10ms(2);
	}
	
	compare_y -= LARGE_COMPARE_INCREMENT;
	PWM0->_1_CMPA = compare_y;
	delay_10ms(2);
	
	while (compare_x > MINIMUM_COMPARE) {
		compare_x -= SMALL_COMPARE_INCREMENT; 
		PWM0->_0_CMPA = compare_x;
		delay_10ms(2);
	}
	
	compare_y -= LARGE_COMPARE_INCREMENT;
	PWM0->_1_CMPA = compare_y;
	delay_10ms(2);
	
	while (compare_x < MAXIMUM_COMPARE) {
		compare_x += SMALL_COMPARE_INCREMENT; 
		PWM0->_0_CMPA = compare_x;
		delay_10ms(2);
	}
}

void sleep_position(void) {
	compare_x = MIDDLE_COMPARE; 
	PWM0->_0_CMPA = compare_x;

	compare_y = MAXIMUM_COMPARE; 
	PWM0->_1_CMPA = compare_y;
}	


void PLL_init(void){
  // 0) Use RCC2
  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}

void SysTick_init(void) {
	NVIC_ST_CTRL_R = 0;						// 1) disable SysTick during setup 
	//NVIC_ST_RELOAD_R = 159999;		// 2) 16 MHz: 159999 + 1 = 160,000; 160,000 * 62.5ns = 10ms
	NVIC_ST_RELOAD_R = 799999;			// 2) 80 MHz: 799999 + 1 = 800,000; 800,000 * 12.5ns = 10ms
	NVIC_ST_CURRENT_R = 0;				// 3) any write to current clears it 
	NVIC_ST_CTRL_R = 0x00000005;	// 4) enable SysTick with core clock
}

// reload is initialized to 800,000 (10ms)
void SysTick_wait(void) {
	NVIC_ST_CURRENT_R = 0;       						// any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){}	// wait for count flag
}

void delay_10ms(unsigned long delay) {
	unsigned long i;
  for(i=0; i < delay; i++){
    SysTick_wait();  // wait 10ms
  }
} 

// UART0 uses PA1-0
// Servos use PB6 (M0PWM0 - module 0, generator 0) and PB4 (M0PWM2 - module 0, generator 1)
void UART0_and_servos_init(void) {
	SYSCTL->RCGCGPIO |= 0x3; 	// Enable clock for GPIO A and B
	SYSCTL->RCGCUART |= (1); 	// Enable clock for UART0 
	SYSCTL->RCGCPWM |= (1);		// Enable clock for PWM0 (module 0 handles PB6 and PB4)
	
	while ((SYSCTL->PRGPIO & 0x3) != 0x3) {} 	// Wait until both GPIO ports are ready
		
	GPIOA->AFSEL |= 0x3;	// Enable alternate functions for PA1-0 		
	GPIOA->AMSEL &= ~0x3;		// Disable analog mode for PA1-0
	GPIOA->PCTL = (GPIOA->PCTL & ~0xFF) | 0x11;						// Enable UART for PA1-0
	GPIOA->DEN |= 0x3;	// Enable data for PA1-0
		
	GPIOB->AFSEL |= 0x50;	// PB6 and PB4 = 0b 0101 0000 = 0x50
	GPIOB->AMSEL &= ~0x50;	// Disable analog mode for PB6 and PB4 		
	GPIOB->PCTL = (GPIOB->PCTL & ~0xF0F0000) | 0x4040000;	// Enable PWM for PB6 and PB4
	GPIOB->DEN |= 0x50;	// Enable data for PB6 and PB4	
		
	// configure PWM 	
	while ((SYSCTL->PRPWM & 0x1) == 0) {}  // Wait until PWM module 0 is ready
		
	SYSCTL->RCC |= (1 << 20);			// Use PWM Divider USEPWMDIV (bit 20)
	SYSCTL->RCC |= (0x7 << 17);  	// Set divider to 64
	
	// PWM MODULE# -> _GEN#_CTL	
	PWM0->_0_CTL = 0;                 // Disable Generator 0 during configuration
	PWM0->_0_LOAD = 24999;            // Set LOAD for 50 Hz
	PWM0->_0_CMPA = 1874;             // Set COMPARE for 1.5 ms pulse width
	PWM0->_0_GENA = (0x2 << 2) | (0x3 << 6); // Set GENA: Low on LOAD, High on CMPA
	PWM0->_0_CTL |= 1;                // Enable Generator 0

	PWM0->_1_CTL = 0;                 // Disable Generator 1 during configuration
	PWM0->_1_LOAD = 24999;            // Set LOAD for 50 Hz
	PWM0->_1_CMPA = 1874;             // Set COMPARE for 1.5 ms pulse width
	PWM0->_1_GENA = (0x2 << 2) | (0x3 << 6); // Set GENA: Low on LOAD, High on CMPA
	PWM0->_1_CTL |= 1;                // Enable Generator 1
	
	PWM0->ENABLE |= (1 << 0) | (1 << 2); // Enable M0PWM0 (PB6) and M0PWM2 (PB4)

	// UART config - baud rate = 9600
	while ((SYSCTL->PRUART & (1)) == 0) {}	// Wait until UART is ready 
	UART0->CTL &= ~(1);	// Disable UART while setting up
	UART0->IBRD = 520;	// Integer part of the baud rate divisor
	UART0->FBRD = 53;	// Fractional part of the baud rate divisor
	UART0->LCRH = (0x60);	// 8-bit, no parity, 1-stop bit
	UART0->CC = 0x0;		// Use system clock as UART clock source
		
	// UART interrupt setup
	UART0->ICR |= 0x10;
	UART0->IM |= 0x10;        
	NVIC_SetPriority(UART0_IRQn, 5); 
	NVIC_EnableIRQ(UART0_IRQn);  
		
	UART0->CTL |= (1 << 0) | (1 << 8) | (1 << 9);	// Enable UART0, TXE, RXE
}
