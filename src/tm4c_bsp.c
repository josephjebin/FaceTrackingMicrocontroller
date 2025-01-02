#include "tm4c_bsp.h" 

void set_PWM0_CMPA(unsigned short new_compare){
	PWM0->_0_CMPA = new_compare; 
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