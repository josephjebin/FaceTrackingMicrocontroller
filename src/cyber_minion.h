#ifndef cyber_minion_h
#define cyber_minion_h
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

void DisableInterrupts(void); 
void EnableInterrupts(void);  

void main_loop(void) __attribute__((aligned(4)));
void UART0_Handler(void);
void scan(void); 
void sleep_position(void); 
void set_compare_x(unsigned short); 
void set_compare_y(unsigned short); 

typedef enum {
    WAITING,  // Minion is either sleeping or locked on a face and waiting for movement
    SCANNING  // Scanning for a face
} State;

extern State current_state; 
extern unsigned short compare_x; 
extern unsigned short compare_y; 

#endif 