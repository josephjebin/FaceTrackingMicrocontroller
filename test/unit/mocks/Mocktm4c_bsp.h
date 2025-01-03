/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKTM4C_BSP_H
#define _MOCKTM4C_BSP_H

#include "unity.h"
#include "tm4c_bsp.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic push
#endif
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void Mocktm4c_bsp_Init(void);
void Mocktm4c_bsp_Destroy(void);
void Mocktm4c_bsp_Verify(void);




#define PLL_init_ExpectAndReturn(cmock_retval) TEST_FAIL_MESSAGE("PLL_init requires _Expect (not AndReturn)");
#define PLL_init_Expect() PLL_init_CMockExpect(__LINE__)
void PLL_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define SysTick_init_ExpectAndReturn(cmock_retval) TEST_FAIL_MESSAGE("SysTick_init requires _Expect (not AndReturn)");
#define SysTick_init_Expect() SysTick_init_CMockExpect(__LINE__)
void SysTick_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define delay_10ms_ExpectAndReturn(cmock_arg1, cmock_retval) TEST_FAIL_MESSAGE("delay_10ms requires _Expect (not AndReturn)");
#define delay_10ms_Expect(cmock_arg1) delay_10ms_CMockExpect(__LINE__, cmock_arg1)
void delay_10ms_CMockExpect(UNITY_LINE_TYPE cmock_line, unsigned long cmock_arg1);
#define UART0_and_servos_init_ExpectAndReturn(cmock_retval) TEST_FAIL_MESSAGE("UART0_and_servos_init requires _Expect (not AndReturn)");
#define UART0_and_servos_init_Expect() UART0_and_servos_init_CMockExpect(__LINE__)
void UART0_and_servos_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define get_UART0_interrupt_data_Expect() TEST_FAIL_MESSAGE("get_UART0_interrupt_data requires _ExpectAndReturn");
#define get_UART0_interrupt_data_ExpectAndReturn(cmock_retval) get_UART0_interrupt_data_CMockExpectAndReturn(__LINE__, cmock_retval)
void get_UART0_interrupt_data_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint8_t cmock_to_return);
#define set_PWM0_generator0_CMPA_ExpectAndReturn(cmock_arg1, cmock_retval) TEST_FAIL_MESSAGE("set_PWM0_generator0_CMPA requires _Expect (not AndReturn)");
#define set_PWM0_generator0_CMPA_Expect(cmock_arg1) set_PWM0_generator0_CMPA_CMockExpect(__LINE__, cmock_arg1)
void set_PWM0_generator0_CMPA_CMockExpect(UNITY_LINE_TYPE cmock_line, unsigned short cmock_arg1);
#define set_PWM0_generator1_CMPA_ExpectAndReturn(cmock_arg1, cmock_retval) TEST_FAIL_MESSAGE("set_PWM0_generator1_CMPA requires _Expect (not AndReturn)");
#define set_PWM0_generator1_CMPA_Expect(cmock_arg1) set_PWM0_generator1_CMPA_CMockExpect(__LINE__, cmock_arg1)
void set_PWM0_generator1_CMPA_CMockExpect(UNITY_LINE_TYPE cmock_line, unsigned short cmock_arg1);

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
