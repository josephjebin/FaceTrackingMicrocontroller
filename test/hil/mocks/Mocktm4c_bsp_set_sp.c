/* AUTOGENERATED FILE. DO NOT EDIT. */
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "Mocktm4c_bsp_set_sp.h"

static const char* CMockString_cmock_arg1 = "cmock_arg1";
static const char* CMockString_set_sp = "set_sp";

typedef struct _CMOCK_set_sp_CALL_INSTANCE
{
  UNITY_LINE_TYPE LineNumber;
  uint32_t* Expected_cmock_arg1;

} CMOCK_set_sp_CALL_INSTANCE;

static struct Mocktm4c_bsp_set_spInstance
{
  CMOCK_MEM_INDEX_TYPE set_sp_CallInstance;
} Mock;


void Mocktm4c_bsp_set_sp_Verify(void)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_MEM_INDEX_TYPE call_instance;
  call_instance = Mock.set_sp_CallInstance;
  if (CMOCK_GUTS_NONE != call_instance)
  {
    UNITY_SET_DETAIL(CMockString_set_sp);
    UNITY_TEST_FAIL(cmock_line, CMockStringCalledLess);
  }
}

void Mocktm4c_bsp_set_sp_Init(void)
{
  Mocktm4c_bsp_set_sp_Destroy();
}

void Mocktm4c_bsp_set_sp_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
}

void set_sp(uint32_t* cmock_arg1)
{
  UNITY_LINE_TYPE cmock_line = TEST_LINE_NUM;
  CMOCK_set_sp_CALL_INSTANCE* cmock_call_instance;
  UNITY_SET_DETAIL(CMockString_set_sp);
  cmock_call_instance = (CMOCK_set_sp_CALL_INSTANCE*)CMock_Guts_GetAddressFor(Mock.set_sp_CallInstance);
  Mock.set_sp_CallInstance = CMock_Guts_MemNext(Mock.set_sp_CallInstance);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringCalledMore);
  cmock_line = cmock_call_instance->LineNumber;
  {
    UNITY_SET_DETAILS(CMockString_set_sp,CMockString_cmock_arg1);
    if (cmock_call_instance->Expected_cmock_arg1 == NULL)
      { UNITY_TEST_ASSERT_NULL(cmock_arg1, cmock_line, CMockStringExpNULL); }
    else
      { UNITY_TEST_ASSERT_EQUAL_HEX32_ARRAY(cmock_call_instance->Expected_cmock_arg1, cmock_arg1, 1, cmock_line, CMockStringMismatch); }
  }
  UNITY_CLR_DETAILS();
}

void CMockExpectParameters_set_sp(CMOCK_set_sp_CALL_INSTANCE* cmock_call_instance, uint32_t* cmock_arg1);
void CMockExpectParameters_set_sp(CMOCK_set_sp_CALL_INSTANCE* cmock_call_instance, uint32_t* cmock_arg1)
{
  cmock_call_instance->Expected_cmock_arg1 = cmock_arg1;
}

void set_sp_CMockExpect(UNITY_LINE_TYPE cmock_line, uint32_t* cmock_arg1)
{
  CMOCK_MEM_INDEX_TYPE cmock_guts_index = CMock_Guts_MemNew(sizeof(CMOCK_set_sp_CALL_INSTANCE));
  CMOCK_set_sp_CALL_INSTANCE* cmock_call_instance = (CMOCK_set_sp_CALL_INSTANCE*)CMock_Guts_GetAddressFor(cmock_guts_index);
  UNITY_TEST_ASSERT_NOT_NULL(cmock_call_instance, cmock_line, CMockStringOutOfMemory);
  memset(cmock_call_instance, 0, sizeof(*cmock_call_instance));
  Mock.set_sp_CallInstance = CMock_Guts_MemChain(Mock.set_sp_CallInstance, cmock_guts_index);
  cmock_call_instance->LineNumber = cmock_line;
  CMockExpectParameters_set_sp(cmock_call_instance, cmock_arg1);
}
