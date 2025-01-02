#ifndef cyber_minion_h
#define cyber_minion_h

void DisableInterrupts(void); 
void EnableInterrupts(void);  
void WaitForInterrupt(void);  // low power mode

void main_loop(void) __attribute__((aligned(4)));
void UART0_Handler(void);
void scan(void); 
void sleep_position(void); 
void set_compare_x(unsigned short); 
void set_compare_y(unsigned short); 

#endif 