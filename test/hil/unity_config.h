#ifdef HIL_TEST
#ifndef unity_uart_config_h
#include "tm4c_bsp.h"  
 
// Undefine Unity's default macros to avoid redefinition warnings
#undef UNITY_OUTPUT_CHAR
#undef UNITY_OUTPUT_START
#undef UNITY_OUTPUT_FLUSH
#undef UNITY_OUTPUT_COMPLETE

#define UNITY_OUTPUT_CHAR(a)    UART_SendChar(a)
#define UNITY_OUTPUT_START()    // UART is already initialized
#define UNITY_OUTPUT_FLUSH()    UART_Flush()
#define UNITY_OUTPUT_COMPLETE() // not implemented because we don't need to close the connection

#endif // unity_uart_config_h
#endif // HIL_TEST