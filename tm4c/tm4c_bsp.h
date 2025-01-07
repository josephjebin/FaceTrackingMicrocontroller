#ifndef tm4c_bsp_h
#define tm4c_bsp_h

#include <stdint.h>

#define SYSCTL_RCC2_R           (*((volatile unsigned long *)0x400FE070))
#define SYSCTL_RCC_R            (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RIS_R            (*((volatile unsigned long *)0x400FE050))

#define NVIC_ST_CTRL_R      (*((volatile uint32_t *)0xE000E010)) // SysTick Control and Status Register
#define NVIC_ST_RELOAD_R    (*((volatile uint32_t *)0xE000E014)) // SysTick Reload Value Register
#define NVIC_ST_CURRENT_R   (*((volatile uint32_t *)0xE000E018)) // SysTick Current Value Register
	
void PLL_init(void); 
void SysTick_init(void); 
void delay_10ms(unsigned long); 
void UART0_and_servos_init(void);
uint8_t get_UART0_interrupt_data(void); 
void UART_TransmitChar(char); 
void UART_Flush(void);
void enable_interrupts(void); 
void wait_for_interrupt(void); //defined in startup_TM4C123.s
void exit_interrupt(); 
void set_PWM0_generator0_CMPA(unsigned short); 
void set_PWM0_generator1_CMPA(unsigned short); 

// must be mocked during HIL testing to allow flow of control to return to tests
void set_sp(uint32_t *); 


#endif // tm4c_bsp_h