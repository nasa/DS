#ifndef _DS_APP_TEST_UTILS_H_
#define _DS_APP_TEST_UTILS_H_

#include "ds_app.h"
#include "utstubs.h"

/*
 * Allow UT access to the global "DS_AppData" object.
 */
extern DS_AppData_t DS_AppData;

/*
 * Global context structures
 */
typedef struct
{
    uint16      EventID;
    uint16      EventType;
    const char *Spec;
} __attribute__((packed)) CFE_EVS_SendEvent_context_t;

typedef struct
{
    const char *Spec;
} __attribute__((packed)) CFE_ES_WriteToSysLog_context_t;

/*
 * Macro to call a function and check its int32 return code
 */
#define UT_TEST_FUNCTION_RC(func, exp)                                                                \
    {                                                                                                 \
        int32 rcexp = exp;                                                                            \
        int32 rcact = func;                                                                           \
        UtAssert_True(rcact == rcexp, "%s (%ld) == %s (%ld)", #func, (long)rcact, #exp, (long)rcexp); \
    }

/*
 * Macro to add a test case to the list of tests to execute
 */
#define ADD_TEST(test) UtTest_Add((Test_##test), DS_UT_Setup, DS_UT_TearDown, #test)

int32 UT_Utils_stub_reporter_hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context);

/*
 * Setup function prior to every test
 */
void DS_Test_Setup(void);

/*
 * Teardown function after every test
 */
void DS_Test_TearDown(void);

#endif
