#ifdef HIL_TEST
#ifndef unity_uart_config_h
#include "tm4c_bsp.h"  // Your UART driver header

#define UNITY_OUTPUT_CHAR(a)    UART_TransmitChar(a)
#define UNITY_OUTPUT_START()    // UART is already initialized 
#define UNITY_OUTPUT_FLUSH()    UART_Flush()
#define UNITY_OUTPUT_COMPLETE() // not implemented because we don't need to close the connection

#include "unity.h"

#endif // unity_uart_config_h
#endif // HIL_TEST