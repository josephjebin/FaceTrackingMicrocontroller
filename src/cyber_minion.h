#ifndef cyber_minion_h
#define cyber_minion_h

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

#endif 