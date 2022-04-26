/*************************************************************************
** File: ds_table_test.c
**
** NASA Docket No. GSC-16,126-1, and identified as "Core Flight Software System
** (CFS) Data Storage Application Version 2”
**
** Copyright © 2007-2014 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration. All Rights
** Reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** Purpose:
**   This file contains unit test cases for the functions contained in the file ds_table.c
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
** Notes:
**
*************************************************************************/

/*
 * Includes
 */

#include "ds_table_tests.h"
#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_table.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_events.h"
#include "ds_version.h"
#include "ds_test_utils.h"
/*#include "ut_utils_lib.h"*/
#include "cfs_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>

DS_DestFileTable_t UT_DS_TABLE_TEST_DestFileTable;
int32              UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                            const UT_StubContext_t *Context)
{
    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    return CFE_TBL_INFO_UPDATED;
} /* end UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1 */

int32 UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook2(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                               const UT_StubContext_t *Context)
{
    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    return CFE_TBL_ERR_NEVER_LOADED;
} /* end UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook2 */

DS_FilterTable_t UT_DS_TABLE_TEST_FilterTable;
int32            UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook3(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                          const UT_StubContext_t *Context)
{
    DS_AppData.FilterTblPtr = &UT_DS_TABLE_TEST_FilterTable;

    return CFE_TBL_INFO_UPDATED;
} /* end UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook3 */

int32 UT_DS_TABLE_TEST_CFE_TBL_GetStatusHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                             const UT_StubContext_t *Context)
{
    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    return CFE_TBL_INFO_UPDATE_PENDING;
} /* end UT_DS_TABLE_TEST_CFE_TBL_GetStatusHook */

typedef struct
{
    void *             RestoreToMemory;
    CFE_ES_CDSHandle_t Handle;
} CFE_ES_RestoreFromCDS_context_t;

int32 UT_DS_TABLE_TEST_CFE_ES_RestoreFromCDSHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                 const UT_StubContext_t *Context)
{
    CFE_ES_RestoreFromCDS_context_t *HookContext = (CFE_ES_RestoreFromCDS_context_t *)Context;
    memset(HookContext->RestoreToMemory, 0, (DS_DEST_FILE_CNT + 1) * 4);
    return CFE_SUCCESS;
} /* end UT_DS_TABLE_TEST_CFE_ES_RestoreFromCDSHook */

uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void DS_TableInit_Test_Nominal(void)
{
    int32 Result;

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_TableInit_Test_Nominal */

void DS_TableInit_Test_TableInfoRecovered(void)
{
    int32 Result;

    int32 strCmpResult;
    char  ExpectedEventString1[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char  ExpectedEventString2[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString1, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Destination File Table data restored from CDS");
    snprintf(ExpectedEventString2, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Filter Table data restored from CDS");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    /* Set to generate both of the two error messages DS_INIT_TBL_CDS_EID  */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Register), CFE_TBL_INFO_RECOVERED_TBL);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_CDS_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString1, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_INIT_TBL_CDS_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString2, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

} /* end DS_TableInit_Test_TableInfoRecovered */

void DS_TableInit_Test_RegisterDestTableError(void)
{
    int32 Result;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Unable to register Destination File Table: Error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Set to generate first instance of error message DS_INIT_TBL_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Register), 0x99);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == 0x99, "Result == 0x99");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableInit_Test_RegisterDestTableError */

void DS_TableInit_Test_RegisterFilterTableError(void)
{
    int32 Result;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Unable to register Filter Table: Error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Set to generate second instance of error message DS_INIT_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 2, 0x99);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == 0x99, "Result == 0x99");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableInit_Test_RegisterFilterTableError */

void DS_TableInit_Test_LoadDestTableError(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Unable to load default Destination File Table: Filename = '%%s', Error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Fail on the first load (loading the dest table */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableInit_Test_LoadDestTableError */

void DS_TableInit_Test_LoadFilterTableError(void)
{
    int32 Result;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Unable to load default Filter Table: Filename = '%%s', Error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Set to generate error message DS_INIT_TBL_ERR_EID on 2nd call (but not 1st) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 2, -1);

    /* Execute the function being tested */
    Result = DS_TableInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableInit_Test_LoadFilterTableError */

void DS_TableManageDestFile_Test_TableInfoUpdated(void)
{
    uint32 i;

    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    DS_AppData.DestFileTblPtr = (DS_DestFileTable_t *)NULL;

    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATED)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1, NULL);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_True(DS_AppData.DestTblLoadCounter == 1, "DS_AppData.DestTblLoadCounter == 1");

    UtAssert_True(DS_AppData.FileStatus[0].FileState == 0, "DS_AppData.FileStatus[0].FileState == 0");
    UtAssert_True(DS_AppData.FileStatus[0].FileCount == 0, "DS_AppData.FileStatus[0].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState == DS_DEST_FILE_CNT / 2,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileState == DS_DEST_FILE_CNT/2");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount == DS_DEST_FILE_CNT / 2,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileCount == DS_DEST_FILE_CNT/2");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState == DS_DEST_FILE_CNT - 1,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileState == DS_DEST_FILE_CNT-1");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount == DS_DEST_FILE_CNT - 1,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileCount == DS_DEST_FILE_CNT-1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageDestFile_Test_TableInfoUpdated */

void DS_TableManageDestFile_Test_TableNeverLoaded(void)
{
    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_ERR_NEVER_LOADED)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook2, NULL);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_True(DS_AppData.DestTblErrCounter == 1, "DS_AppData.DestTblErrCounter == 1");
    UtAssert_True(DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL,
                  "DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *) NULL");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_TableManageDestFile_Test_TableNeverLoaded */

void DS_TableManageDestFile_Test_TableInfoDumpPending(void)
{
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_DUMP_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_DUMP_PENDING);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_TableManageDestFile_Test_TableInfoDumpPending */

void DS_TableManageDestFile_Test_TableInfoValidationPending(void)
{
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_VALIDATION_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_VALIDATION_PENDING);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_TableManageDestFile_Test_TableInfoValidationPending */

void DS_TableManageDestFile_Test_TableInfoUpdatePending(void)
{
    uint32 i;

    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATE_PENDING)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetStatus), &UT_DS_TABLE_TEST_CFE_TBL_GetStatusHook, NULL);

    /* Prevents segmentation fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1, NULL);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_True(DS_AppData.DestTblLoadCounter == 1, "DS_AppData.DestTblLoadCounter == 1");

    UtAssert_True(DS_AppData.FileStatus[0].FileState == 0, "DS_AppData.FileStatus[0].FileState == 0");
    UtAssert_True(DS_AppData.FileStatus[0].FileCount == 0, "DS_AppData.FileStatus[0].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState == DS_DEST_FILE_CNT / 2,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileState == DS_DEST_FILE_CNT/2");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount == DS_DEST_FILE_CNT / 2,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileCount == DS_DEST_FILE_CNT/2");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState == DS_DEST_FILE_CNT - 1,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileState == DS_DEST_FILE_CNT-1");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount == DS_DEST_FILE_CNT - 1,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileCount == DS_DEST_FILE_CNT-1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageDestFile_Test_TableInfoUpdatePending */

void DS_TableManageDestFile_Test_TableSuccess(void)
{
    uint32 i;

    DS_AppData.DestFileTblPtr = &UT_DS_TABLE_TEST_DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATE_PENDING)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    // UT_SetHookFunction(UT_KEY(CFE_TBL_GetStatus), &UT_DS_TABLE_TEST_CFE_TBL_GetStatusHook, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_SUCCESS);
    /* Prevents segmentation fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1, NULL);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_True(DS_AppData.DestTblLoadCounter == 0, "DS_AppData.DestTblLoadCounter == 0");

    UtAssert_True(DS_AppData.FileStatus[0].FileState == 0, "DS_AppData.FileStatus[0].FileState == 0");
    UtAssert_True(DS_AppData.FileStatus[0].FileCount == 0, "DS_AppData.FileStatus[0].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileState == 0");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileState == 0");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileCount == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageDestFile_Test_TableInfoUpdatePending */

void DS_TableManageFilter_Test_TableInfoUpdated(void)
{
    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATED)", and sets
     * DS_AppData.FilterTblPtr to the address of a table defined globally in this file, to prevent segmentation fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook3, NULL);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_True(DS_AppData.FilterTblLoadCounter == 1, "DS_AppData.FilterTblLoadCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableInfoUpdated */

void DS_TableManageFilter_Test_TableNeverLoaded(void)
{
    /* Set to satisfy condition "if (Result == CFE_TBL_ERR_NEVER_LOADED)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_ERR_NEVER_LOADED);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_True(DS_AppData.FilterTblErrCounter == 1, "DS_AppData.FilterTblErrCounter == 1");
    UtAssert_True(DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL,
                  "DS_AppData.FilterTblPtr == (DS_FilterTable_t *) NULL");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableNeverLoaded */

void DS_TableManageFilter_Test_TableInfoDumpPending(void)
{
    DS_FilterTable_t FilterTable;

    DS_AppData.FilterTblPtr = &FilterTable;

    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_DUMP_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_DUMP_PENDING);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableInfoDumpPending */

void DS_TableManageFilter_Test_TableInfoValidationPending(void)
{
    DS_FilterTable_t FilterTable;

    DS_AppData.FilterTblPtr = &FilterTable;

    /* Set to satisfy condition "CFE_TBL_INFO_VALIDATION_PENDING" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_VALIDATION_PENDING);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableInfoValidationPending */

void DS_TableManageFilter_Test_TableInfoUpdatePending(void)
{
    DS_AppData.FilterTblPtr = &UT_DS_TABLE_TEST_FilterTable;

    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATE_PENDING)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetStatus), &UT_DS_TABLE_TEST_CFE_TBL_GetStatusHook, NULL);

    /* Prevents segmentation fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1, NULL);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_True(DS_AppData.FilterTblLoadCounter == 1, "DS_AppData.FilterTblLoadCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableInfoUpdatePending */

void DS_TableManageFilter_Test_TableSuccess(void)
{
    DS_AppData.FilterTblPtr = &UT_DS_TABLE_TEST_FilterTable;

    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATE_PENDING)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_SUCCESS);
    /* Prevents segmentation fault */
    UT_SetHookFunction(UT_KEY(CFE_TBL_GetAddress), &UT_DS_TABLE_TEST_CFE_TBL_GetAddressHook1, NULL);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_True(DS_AppData.FilterTblLoadCounter == 0, "DS_AppData.FilterTblLoadCounter == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableManageFilter_Test_TableInfoUpdatePending */

void DS_TableVerifyDestFile_Test_Nominal(void)
{
    int32              Result;
    DS_DestFileTable_t DestFileTable;
    uint32             FileIndex = 0;
    uint32             i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Destination file table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileTable.File[FileIndex].FileNameType  = DS_BY_TIME;
    DestFileTable.File[FileIndex].EnableState   = DS_ENABLED;
    DestFileTable.File[FileIndex].MaxFileSize   = 2048;
    DestFileTable.File[FileIndex].MaxFileAge    = 100;
    DestFileTable.File[FileIndex].SequenceCount = 1;

    strncpy(DestFileTable.File[FileIndex].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Extension, "ext", DS_EXTENSION_BUFSIZE);

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        memset(&DestFileTable.File[i], DS_UNUSED, sizeof(DS_DestFileEntry_t));
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFile(&DestFileTable);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFile_Test_Nominal */

void DS_TableVerifyDestFile_Test_DestFileTableVerificationError(void)
{
    int32              Result;
    DS_DestFileTable_t DestFileTable;
    uint32             FileIndex = 0;
    uint32             i;
    int32              strCmpResult;
    char               ExpectedEventString1[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char               ExpectedEventString2[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString1, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Destination file table verify err: invalid descriptor text");

    snprintf(ExpectedEventString2, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Destination file table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    DestFileTable.File[FileIndex].FileNameType  = DS_BY_TIME;
    DestFileTable.File[FileIndex].EnableState   = DS_ENABLED;
    DestFileTable.File[FileIndex].MaxFileSize   = 2048;
    DestFileTable.File[FileIndex].MaxFileAge    = 100;
    DestFileTable.File[FileIndex].SequenceCount = 1;

    strncpy(DestFileTable.File[FileIndex].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Extension, "ext", DS_EXTENSION_BUFSIZE);

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        memset(&DestFileTable.File[i], DS_UNUSED, sizeof(DS_DestFileEntry_t));
    }

    for (i = 0; i < DS_DESCRIPTOR_BUFSIZE; i++)
    {
        DestFileTable.Descriptor[i] = '*';
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 1, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFile(&DestFileTable);

    /* Verify results */
    UtAssert_True(Result == DS_TABLE_VERIFY_ERR, "Result == DS_TABLE_VERIFY_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString1, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FIL_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString2, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

} /* end DS_TableVerifyDestFile_Test_DestFileTableVerificationError */

void DS_TableVerifyDestFile_Test_CountBad(void)
{
    int32              Result;
    DS_DestFileTable_t DestFileTable;
    uint32             FileIndex = 0;
    uint32             i;
    int32              strCmpResult;
    char               ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Destination file table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    strncpy(DestFileTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    DestFileTable.File[FileIndex].FileNameType  = DS_BY_TIME;
    DestFileTable.File[FileIndex].EnableState   = DS_ENABLED;
    DestFileTable.File[FileIndex].MaxFileSize   = 2048;
    DestFileTable.File[FileIndex].MaxFileAge    = 100;
    DestFileTable.File[FileIndex].SequenceCount = 1;

    strncpy(DestFileTable.File[FileIndex].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[FileIndex].Extension, "123456789", DS_EXTENSION_BUFSIZE);

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        memset(&DestFileTable.File[i], DS_UNUSED, sizeof(DS_DestFileEntry_t));
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 3, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFile(&DestFileTable);

    /* Verify results */
    UtAssert_True(Result == DS_TABLE_VERIFY_ERR, "Result == DS_TABLE_VERIFY_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);
    /* this generates 1 event message we don't care about for this test */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FIL_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

} /* end DS_TableVerifyDestFile_Test_CountBad */

void DS_TableVerifyDestFileEntry_Test_NominalErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_NominalErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;
    int32              strCmpResult;
    char               ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, invalid pathname text");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "***", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, invalid basename text");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "***", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 2, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, invalid extension text");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "123456789", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 3, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, filename type = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = 99;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, file enable state = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = 99;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, max file size = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = DS_FILE_MIN_SIZE_LIMIT - 1;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, max file age = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = DS_FILE_MIN_AGE_LIMIT - 1;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "%%s index = %%d, sequence count = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FIL_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "***", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrNonZero */

void DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "***", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 2, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrNonZero */

void DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "123456789", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 3, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrNonZero */

void DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = 99;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = 99;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidSizeErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = DS_FILE_MIN_SIZE_LIMIT - 1;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidAgeErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = DS_FILE_MIN_AGE_LIMIT - 1;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero */

void DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrNonZero(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero */

void DS_TableVerifyFilter_Test_Nominal(void)
{
    int32            Result;
    DS_FilterTable_t FilterTable;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Filter table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    FilterTable.Packet[0].MessageID                = 0x18BB;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    FilterTable.Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    strncpy(FilterTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&FilterTable.Packet[0].Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);

    /* Execute the function being tested */
    Result = DS_TableVerifyFilter(&FilterTable);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FLT_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyFilter_Test_Nominal */

void DS_TableVerifyFilter_Test_FilterTableVerificationError(void)
{
    int32            Result;
    DS_FilterTable_t FilterTable;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString1[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char  ExpectedEventString2[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString1, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Filter table verify err: invalid descriptor text");

    snprintf(ExpectedEventString2, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Filter table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    FilterTable.Packet[0].MessageID                = 0x18BB;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    FilterTable.Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    for (i = 0; i < DS_DESCRIPTOR_BUFSIZE; i++)
    {
        FilterTable.Descriptor[i] = '*';
    }

    for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&FilterTable.Packet[0].Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), false);

    /* Execute the function being tested */
    Result = DS_TableVerifyFilter(&FilterTable);

    /* Verify results */
    UtAssert_True(Result == DS_TABLE_VERIFY_ERR, "Result == DS_TABLE_VERIFY_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString1, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FLT_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString2, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

} /* end DS_TableVerifyFilter_Test_FilterTableVerificationError */

void DS_TableVerifyFilter_Test_CountBad(void)
{
    int32            Result;
    DS_FilterTable_t FilterTable;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Filter table verify results: desc text = %%s, good entries = %%d, bad = %%d, unused = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    FilterTable.Packet[0].MessageID                = 0x18BB;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    FilterTable.Packet[0].Filter[0].FilterType     = DS_BY_TIME * 2;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    strncpy(FilterTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&FilterTable.Packet[0].Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFS_VerifyString), true);
    UT_SetDeferredRetcode(UT_KEY(CFS_VerifyString), 3, false);

    /* Execute the function being tested */
    Result = DS_TableVerifyFilter(&FilterTable);

    /* Verify results */
    UtAssert_True(Result == DS_TABLE_VERIFY_ERR, "Result == DS_TABLE_VERIFY_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);
    /* this generates 1 event message we don't care about for this test */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FLT_TBL_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

} /* end DS_TableVerifyFilter_Test_CountBad */

void DS_TableVerifyFilterEntry_Test_Unused(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;
    uint32           i;

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 0;
    PacketEntry.Filter[0].Algorithm_X    = 0;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFilterEntry_Test_Unused */

void DS_TableVerifyFilterEntry_Test_Nominal(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;
    uint32           i;

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 0;
    PacketEntry.Filter[0].Algorithm_X    = 0;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFilterEntry_Test_Nominal */

void DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "%%s MID = 0x%%08X, index = %%d, filter = %%d, file table index = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = DS_DEST_FILE_CNT + 1;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FLT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero */

void DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "%%s MID = 0x%%08X, index = %%d, filter = %%d, filter type = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 99;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FLT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero */

void DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;
    uint32           i;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "%%s MID = 0x%%08X, index = %%d, filter = %%d, filter parms N = %%d, X = %%d, O = %%d");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 99;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_FLT_TBL_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero */

void DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;
    uint32           i;

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = DS_DEST_FILE_CNT + 1;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero */

void DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;
    uint32           i;

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 99;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero */

void DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero(void)
{
    int32            Result;
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;
    uint32           i;

    PacketEntry.MessageID                = 0x18BB;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 99;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    for (i = 1; i < DS_FILTERS_PER_PACKET; i++)
    {
        memset(&PacketEntry.Filter[i], DS_UNUSED, sizeof(DS_FilterParms_t));
    }

    /* Execute the function being tested */
    Result = DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero */

void DS_TableEntryUnused_Test_Nominal(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;

    memset(&DestFileEntry, DS_UNUSED, sizeof(DS_DestFileEntry_t));

    /* Execute the function being tested */
    Result = DS_TableEntryUnused(&DestFileEntry, sizeof(DS_DestFileEntry_t));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableEntryUnused_Test_Nominal */

void DS_TableEntryUnused_Test_Fail(void)
{
    int32              Result;
    DS_DestFileEntry_t DestFileEntry;

    memset(&DestFileEntry, 99, sizeof(DS_DestFileEntry_t));

    /* Execute the function being tested */
    Result = DS_TableEntryUnused(&DestFileEntry, sizeof(DS_DestFileEntry_t));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableEntryUnused_Test_Fail */

void DS_TableVerifyFileIndex_Test_Nominal(void)
{
    int32  Result;
    uint16 FileTableIndex = 0;

    /* Execute the function being tested */
    Result = DS_TableVerifyFileIndex(FileTableIndex);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFileIndex_Test_Nominal */

void DS_TableVerifyFileIndex_Test_Fail(void)
{
    int32  Result;
    uint16 FileTableIndex = DS_DEST_FILE_CNT;

    /* Execute the function being tested */
    Result = DS_TableVerifyFileIndex(FileTableIndex);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyFileIndex_Test_Fail */

void DS_TableVerifyParms_Test_NominalOnlyXNonZero(void)
{
    int32  Result;
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 1;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    Result = DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyParms_Test_NominalOnlyXNonZero */

void DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero(void)
{
    int32  Result;
    uint16 Algorithm_N = 1;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    Result = DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero */

void DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero(void)
{
    int32  Result;
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 1;

    /* Execute the function being tested */
    Result = DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero */

void DS_TableVerifyParms_Test_AllZero(void)
{
    int32  Result;
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    Result = DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyParms_Test_AllZero */

void DS_TableVerifyType_Test_Nominal(void)
{
    int32  Result;
    uint16 TimeVsCount = DS_BY_TIME;

    /* Execute the function being tested */
    Result = DS_TableVerifyType(TimeVsCount);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyType_Test_Nominal */

void DS_TableVerifyType_Test_Fail(void)
{
    int32  Result;
    uint16 TimeVsCount = 99;

    /* Execute the function being tested */
    Result = DS_TableVerifyType(TimeVsCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyType_Test_Fail */

void DS_TableVerifyState_Test_NominalEnabled(void)
{
    int32  Result;
    uint16 EnableState = DS_ENABLED;

    /* Execute the function being tested */
    Result = DS_TableVerifyState(EnableState);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyState_Test_NominalEnabled */

void DS_TableVerifyState_Test_NominalDisabled(void)
{
    int32  Result;
    uint16 EnableState = DS_DISABLED;

    /* Execute the function being tested */
    Result = DS_TableVerifyState(EnableState);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyState_Test_NominalDisabled */

void DS_TableVerifyState_Test_Fail(void)
{
    int32  Result;
    uint16 EnableState = 99;

    /* Execute the function being tested */
    Result = DS_TableVerifyState(EnableState);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyState_Test_Fail */

void DS_TableVerifySize_Test_Nominal(void)
{
    int32  Result;
    uint32 MaxFileSize = DS_FILE_MIN_SIZE_LIMIT;

    /* Execute the function being tested */
    Result = DS_TableVerifySize(MaxFileSize);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifySize_Test_Nominal */

void DS_TableVerifySize_Test_Fail(void)
{
    int32  Result;
    uint32 MaxFileSize = DS_FILE_MIN_SIZE_LIMIT - 1;

    /* Execute the function being tested */
    Result = DS_TableVerifySize(MaxFileSize);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifySize_Test_Fail */

void DS_TableVerifyAge_Test_Nominal(void)
{
    int32  Result;
    uint32 MaxFileAge = DS_FILE_MIN_AGE_LIMIT;

    /* Execute the function being tested */
    Result = DS_TableVerifyAge(MaxFileAge);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyAge_Test_Nominal */

void DS_TableVerifyAge_Test_Fail(void)
{
    int32  Result;
    uint32 MaxFileAge = DS_FILE_MIN_AGE_LIMIT - 1;

    /* Execute the function being tested */
    Result = DS_TableVerifyAge(MaxFileAge);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyAge_Test_Fail */

void DS_TableVerifyCount_Test_Nominal(void)
{
    int32  Result;
    uint32 SequenceCount = DS_MAX_SEQUENCE_COUNT;

    /* Execute the function being tested */
    Result = DS_TableVerifyCount(SequenceCount);

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyCount_Test_Nominal */

void DS_TableVerifyCount_Test_Fail(void)
{
    int32  Result;
    uint32 SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    /* Execute the function being tested */
    Result = DS_TableVerifyCount(SequenceCount);

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableVerifyCount_Test_Fail */

void DS_TableSubscribe_Test_Unused(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_SubscribeEx = 0;
    DS_AppData.FilterTblPtr                        = &FilterTable;

    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_SubscribeEx = UT_GetStubCount(UT_KEY(CFE_SB_SubscribeEx));
    UtAssert_INT32_EQ(call_count_CFE_SB_SubscribeEx, 0);

} /* end DS_TableSubscribe_Test_Unused */

void DS_TableSubscribe_Test_Cmd(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_SubscribeEx = 0;
    DS_AppData.FilterTblPtr                        = &FilterTable;

    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }
    FilterTable.Packet[0].MessageID = DS_CMD_MID;

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_SubscribeEx = UT_GetStubCount(UT_KEY(CFE_SB_SubscribeEx));
    UtAssert_INT32_EQ(call_count_CFE_SB_SubscribeEx, 0);

} /* end DS_TableSubscribe_Test_Cmd */

void DS_TableSubscribe_Test_SendHk(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_SubscribeEx = 0;
    DS_AppData.FilterTblPtr                        = &FilterTable;

    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }
    FilterTable.Packet[0].MessageID = DS_SEND_HK_MID;

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_SubscribeEx = UT_GetStubCount(UT_KEY(CFE_SB_SubscribeEx));
    UtAssert_INT32_EQ(call_count_CFE_SB_SubscribeEx, 0);

} /* end DS_TableSubscribe_Test_SendHk*/

void DS_TableSubscribe_Test_Data(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_SubscribeEx = 0;
    DS_AppData.FilterTblPtr                        = &FilterTable;

    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }
    FilterTable.Packet[0].MessageID = 0x18DD; /* NOT the CMD or SEND_HK MIDs */

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_SubscribeEx = UT_GetStubCount(UT_KEY(CFE_SB_SubscribeEx));
    UtAssert_INT32_EQ(call_count_CFE_SB_SubscribeEx, 1);

} /* end DS_TableSubscribe_Test_Data */

void DS_TableUnsubscribe_Test_Unused(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_Unsubscribe = 0;

    DS_AppData.FilterTblPtr = &FilterTable;

    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_Unsubscribe = UT_GetStubCount(UT_KEY(CFE_SB_Unsubscribe));
    UtAssert_INT32_EQ(call_count_CFE_SB_Unsubscribe, 0);

} /* end DS_TableUnsubscribe_Test_Unused */

void DS_TableUnsubscribe_Test_Cmd(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_Unsubscribe = 0;

    DS_AppData.FilterTblPtr = &FilterTable;
    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    FilterTable.Packet[0].MessageID = DS_CMD_MID;

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_Unsubscribe = UT_GetStubCount(UT_KEY(CFE_SB_Unsubscribe));
    UtAssert_INT32_EQ(call_count_CFE_SB_Unsubscribe, 0);

} /* end DS_TableUnsubscribe_Test_Cmd */

void DS_TableUnsubscribe_Test_SendHk(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_Unsubscribe = 0;

    DS_AppData.FilterTblPtr = &FilterTable;
    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    FilterTable.Packet[0].MessageID = DS_SEND_HK_MID;

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_Unsubscribe = UT_GetStubCount(UT_KEY(CFE_SB_Unsubscribe));
    UtAssert_INT32_EQ(call_count_CFE_SB_Unsubscribe, 0);

} /* end DS_TableUnsubscribe_Test_SendHk */

void DS_TableUnsubscribe_Test_Data(void)
{
    DS_FilterTable_t FilterTable;
    uint8            call_count_CFE_SB_Unsubscribe = 0;

    DS_AppData.FilterTblPtr = &FilterTable;
    for (int i = 0; i < DS_PACKETS_IN_FILTER_TABLE; i++)
    {
        FilterTable.Packet[i].MessageID = DS_UNUSED;
    }

    FilterTable.Packet[0].MessageID = 0x18DD; /* NOT the CMD or SEND_HK MIDs */

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    call_count_CFE_SB_Unsubscribe = UT_GetStubCount(UT_KEY(CFE_SB_Unsubscribe));
    UtAssert_INT32_EQ(call_count_CFE_SB_Unsubscribe, 1);

} /* end DS_TableUnsubscribe_Test_Data */

void DS_TableCreateCDS_Test_NewCDSArea(void)
{
    int32 Result;

    /* Execute the function being tested */
    Result = DS_TableCreateCDS();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableCreateCDS_Test_NewCDSArea */

void DS_TableCreateCDS_Test_PreExistingCDSArea(void)
{
    int32                           Result;
    CFE_ES_RestoreFromCDS_context_t CFE_ES_RestoreFromCDS_context;
    uint32                          DataStoreBuffer[DS_DEST_FILE_CNT + 1] = {0};

    CFE_ES_RestoreFromCDS_context.RestoreToMemory = DataStoreBuffer;

    /* Set to satisfy condition "if (Result == CFE_ES_CDS_ALREADY_EXISTS)", which is the main thing we're testing here
     */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), CFE_ES_CDS_ALREADY_EXISTS);

    /* Execute the function being tested */
    Result = DS_TableCreateCDS();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(DS_AppData.FileStatus[0].FileCount == 0, "DS_AppData.FileStatus[0].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileCount == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileCount == 0");

#if (DS_CDS_ENABLE_STATE == 1)
    /* only test if configured */
    UtAssert_True(DS_AppData.AppEnableState == 0, "DS_AppData.AppEnableState == 0");
#endif

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableCreateCDS_Test_PreExistingCDSArea */

void DS_TableCreateCDS_Test_RestoreFail(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Critical Data Store access error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CFE_ES_RestoreFromCDS_context_t CFE_ES_RestoreFromCDS_context;
    uint32                          DataStoreBuffer[DS_DEST_FILE_CNT + 1] = {0};

    CFE_ES_RestoreFromCDS_context.RestoreToMemory = DataStoreBuffer;

    /* Set to satisfy condition "if (Result == CFE_ES_CDS_ALREADY_EXISTS)", which is the main thing we're testing here
     */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), CFE_ES_CDS_ALREADY_EXISTS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RestoreFromCDS), -1);

    /* Execute the function being tested */
    Result = DS_TableCreateCDS();

    /* Verify results */
    UtAssert_True(CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE),
                  "CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE");

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS"); /* CFE_SUCCESS because error is not fatal */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_CDS_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableCreateCDS_Test_RestoreFail */

void DS_TableCreateCDS_Test_Error(void)
{
    int32 Result;

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Critical Data Store access error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Set to generate error message DS_INIT_CDS_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), -1);

    /* Execute the function being tested */
    Result = DS_TableCreateCDS();

    /* Verify results */
    UtAssert_True(CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE),
                  "CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE");

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS"); /* CFE_SUCCESS because error is not fatal */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_CDS_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableCreateCDS_Test_Error */

void DS_TableUpdateCDS_Test_Nominal(void)
{
    DS_AppData.DataStoreHandle = (CFE_ES_CDSHandle_t)CFE_RESOURCEID_WRAP(1);

    /* Execute the function being tested */
    DS_TableUpdateCDS();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableUpdateCDS_Test_Nominal */

void DS_TableUpdateCDS_Test_Error(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Critical Data Store access error = 0x%%08X");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    DS_AppData.DataStoreHandle = (CFE_ES_CDSHandle_t)CFE_RESOURCEID_WRAP(1);

    /* Set to generate error message DS_INIT_CDS_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CopyToCDS), -1);

    /* Execute the function being tested */
    DS_TableUpdateCDS();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_CDS_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_TableUpdateCDS_Test_Error */

void DS_TableHashFunction_Test(void)
{
    uint32         Result;
    CFE_SB_MsgId_t MessageID = 0x18BB;

    /* Execute the function being tested */
    Result = DS_TableHashFunction(MessageID);

    /* Verify results */
    UtAssert_True(Result == 187, "Result == 187");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableHashFunction_Test */

void DS_TableCreateHash_Test_Nominal(void)
{
    DS_HashLink_t    HashLink1;
    DS_HashLink_t    HashLink2;
    DS_FilterTable_t FilterTable;

    DS_AppData.FilterTblPtr = &FilterTable;

    FilterTable.Packet[0].MessageID = 0x18BB;

    DS_AppData.HashTable[187] = &HashLink1;

    DS_AppData.HashLinks[0].Index     = 0;
    DS_AppData.HashLinks[0].MessageID = 0x18BB;
    DS_AppData.HashTable[187]->Next   = &HashLink2;

    /* Execute the function being tested */
    DS_TableCreateHash();

    /* Verify results */
    UtAssert_True(DS_AppData.HashLinks[0].Index == 0, "DS_AppData.HashLinks[0].Index == 0");
    UtAssert_True(DS_AppData.HashLinks[0].MessageID == 0x18BB, "DS_AppData.HashLinks[0].MessageID == 0x18BB");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableCreateHash_Test_Nominal */

void DS_TableCreateHash_Test_NullTable(void)
{
    DS_FilterTable_t FilterTable;

    DS_AppData.FilterTblPtr = &FilterTable;

    FilterTable.Packet[0].MessageID = 0x18BB;

    /* Execute the function being tested */
    DS_TableCreateHash();

    /* Verify results */
    UtAssert_True(DS_AppData.HashLinks[0].Index == 0, "DS_AppData.HashLinks[0].Index == 0");
    UtAssert_True(DS_AppData.HashLinks[0].MessageID == 0x18BB, "DS_AppData.HashLinks[0].MessageID == 0x18BB");
    UtAssert_True(DS_AppData.HashTable[187] == &DS_AppData.HashLinks[0],
                  "DS_AppData.HashTable[187] == &DS_AppData.HashLinks[0]");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableCreateHash_Test_NullTable */

void DS_TableFindMsgID_Test(void)
{
    int32            Result;
    CFE_SB_MsgId_t   MessageID = 0x18BB;
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    uint32 HashTableIndex = ((uint32)(MessageID & DS_HASH_TABLE_MASK));

    DS_AppData.FilterTblPtr = &FilterTable;

    DS_AppData.HashTable[HashTableIndex] = &HashLink;

    HashLink.Index = 1;

    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = MessageID;

    /* Execute the function being tested */
    Result = DS_TableFindMsgID(MessageID);

    /* Verify results */
    UtAssert_True(Result == 1, "Result == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableFindMsgID_Test */

void DS_TableFindMsgID_Test_NullTable(void)
{
    int32            Result;
    CFE_SB_MsgId_t   MessageID = 0x18BB;
    DS_FilterTable_t FilterTable;

    DS_AppData.FilterTblPtr = &FilterTable;

    for (int i = 0; i < DS_HASH_TABLE_ENTRIES; i++)
    {
        DS_AppData.HashTable[i] = NULL;
    }

    /* Execute the function being tested */
    Result = DS_TableFindMsgID(MessageID);

    /* Verify results */
    UtAssert_INT32_EQ(Result, DS_INDEX_NONE);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableFindMsgID_Test_NullTable */

void DS_TableFindMsgID_Test_Mismatch(void)
{
    int32            Result;
    CFE_SB_MsgId_t   MessageID = 0x18BB;
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    uint32 HashTableIndex = ((uint32)(MessageID & DS_HASH_TABLE_MASK));

    DS_AppData.FilterTblPtr = &FilterTable;

    DS_AppData.HashTable[HashTableIndex] = &HashLink;

    HashLink.Index = 1;
    HashLink.Next  = NULL;

    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = MessageID + 1;

    /* Execute the function being tested */
    Result = DS_TableFindMsgID(MessageID);

    /* Verify results */
    UtAssert_INT32_EQ(Result, DS_INDEX_NONE);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_TableFindMsgID_Test_Mismatch */

void UtTest_Setup(void)
{
    UtTest_Add(DS_TableInit_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableInit_Test_Nominal");
    UtTest_Add(DS_TableInit_Test_TableInfoRecovered, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableInit_Test_TableInfoRecovered");
    UtTest_Add(DS_TableInit_Test_RegisterDestTableError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableInit_Test_RegisterDestTableError");
    UtTest_Add(DS_TableInit_Test_RegisterFilterTableError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableInit_Test_RegisterFilterTableError");
    UtTest_Add(DS_TableInit_Test_LoadDestTableError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableInit_Test_LoadDestTableError");
    UtTest_Add(DS_TableInit_Test_LoadFilterTableError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableInit_Test_LoadFilterTableError");

    UtTest_Add(DS_TableManageDestFile_Test_TableInfoUpdated, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableInfoUpdated");
    UtTest_Add(DS_TableManageDestFile_Test_TableNeverLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableNeverLoaded");
    UtTest_Add(DS_TableManageDestFile_Test_TableInfoDumpPending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableInfoDumpPending");
    UtTest_Add(DS_TableManageDestFile_Test_TableInfoValidationPending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableInfoValidationPending");
    UtTest_Add(DS_TableManageDestFile_Test_TableInfoUpdatePending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableInfoUpdatePending");
    UtTest_Add(DS_TableManageDestFile_Test_TableSuccess, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageDestFile_Test_TableSuccess");

    UtTest_Add(DS_TableManageFilter_Test_TableInfoUpdated, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableInfoUpdated");
    UtTest_Add(DS_TableManageFilter_Test_TableNeverLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableNeverLoaded");
    UtTest_Add(DS_TableManageFilter_Test_TableInfoDumpPending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableInfoDumpPending");
    UtTest_Add(DS_TableManageFilter_Test_TableInfoValidationPending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableInfoValidationPending");
    UtTest_Add(DS_TableManageFilter_Test_TableInfoUpdatePending, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableInfoUpdatePending");
    UtTest_Add(DS_TableManageFilter_Test_TableSuccess, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableManageFilter_Test_TableSuccess");

    UtTest_Add(DS_TableVerifyDestFile_Test_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFile_Test_Nominal");
    UtTest_Add(DS_TableVerifyDestFile_Test_DestFileTableVerificationError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFile_Test_DestFileTableVerificationError");
    UtTest_Add(DS_TableVerifyDestFile_Test_CountBad, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFile_Test_CountBad");

    UtTest_Add(DS_TableVerifyDestFileEntry_Test_NominalErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_NominalErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero");

    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidExtensionErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidSizeErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidSizeErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidAgeErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidAgeErrNonZero");
    UtTest_Add(DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrNonZero");

    UtTest_Add(DS_TableVerifyFilter_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyFilter_Test_Nominal");
    UtTest_Add(DS_TableVerifyFilter_Test_FilterTableVerificationError, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilter_Test_FilterTableVerificationError");
    UtTest_Add(DS_TableVerifyFilter_Test_CountBad, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilter_Test_CountBad");

    UtTest_Add(DS_TableVerifyFilterEntry_Test_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_Nominal");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_Unused, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_Unused");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero");
    UtTest_Add(DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero");

    UtTest_Add(DS_TableEntryUnused_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableEntryUnused_Test_Nominal");
    UtTest_Add(DS_TableEntryUnused_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableEntryUnused_Test_Fail");

    UtTest_Add(DS_TableVerifyFileIndex_Test_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyFileIndex_Test_Nominal");
    UtTest_Add(DS_TableVerifyFileIndex_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyFileIndex_Test_Fail");

    UtTest_Add(DS_TableVerifyParms_Test_NominalOnlyXNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyParms_Test_NominalOnlyXNonZero");
    UtTest_Add(DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero");
    UtTest_Add(DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero");
    UtTest_Add(DS_TableVerifyParms_Test_AllZero, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyParms_Test_AllZero");

    UtTest_Add(DS_TableVerifyType_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyType_Test_Nominal");
    UtTest_Add(DS_TableVerifyType_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyType_Test_Fail");

    UtTest_Add(DS_TableVerifyState_Test_NominalEnabled, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyState_Test_NominalEnabled");
    UtTest_Add(DS_TableVerifyState_Test_NominalDisabled, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableVerifyState_Test_NominalDisabled");
    UtTest_Add(DS_TableVerifyState_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyState_Test_Fail");

    UtTest_Add(DS_TableVerifySize_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifySize_Test_Nominal");
    UtTest_Add(DS_TableVerifySize_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifySize_Test_Fail");

    UtTest_Add(DS_TableVerifyAge_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyAge_Test_Nominal");
    UtTest_Add(DS_TableVerifyAge_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyAge_Test_Fail");

    UtTest_Add(DS_TableVerifyCount_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyCount_Test_Nominal");
    UtTest_Add(DS_TableVerifyCount_Test_Fail, DS_Test_Setup, DS_Test_TearDown, "DS_TableVerifyCount_Test_Fail");

    UtTest_Add(DS_TableSubscribe_Test_Unused, DS_Test_Setup, DS_Test_TearDown, "DS_TableSubscribe_Test_Unused");
    UtTest_Add(DS_TableSubscribe_Test_Cmd, DS_Test_Setup, DS_Test_TearDown, "DS_TableSubscribe_Test_Cmd");
    UtTest_Add(DS_TableSubscribe_Test_SendHk, DS_Test_Setup, DS_Test_TearDown, "DS_TableSubscribe_Test_SendHk");
    UtTest_Add(DS_TableSubscribe_Test_Data, DS_Test_Setup, DS_Test_TearDown, "DS_TableSubscribe_Test_Data");

    UtTest_Add(DS_TableUnsubscribe_Test_Unused, DS_Test_Setup, DS_Test_TearDown, "DS_TableUnsubscribe_Test_Unused");
    UtTest_Add(DS_TableUnsubscribe_Test_Cmd, DS_Test_Setup, DS_Test_TearDown, "DS_TableUnsubscribe_Test_Cmd");
    UtTest_Add(DS_TableUnsubscribe_Test_SendHk, DS_Test_Setup, DS_Test_TearDown, "DS_TableUnsubscribe_Test_SendHk");
    UtTest_Add(DS_TableUnsubscribe_Test_Data, DS_Test_Setup, DS_Test_TearDown, "DS_TableUnsubscribe_Test_Data");

    UtTest_Add(DS_TableCreateCDS_Test_NewCDSArea, DS_Test_Setup, DS_Test_TearDown, "DS_TableCreateCDS_Test_NewCDSArea");
    UtTest_Add(DS_TableCreateCDS_Test_PreExistingCDSArea, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableCreateCDS_Test_PreExistingCDSArea");
    UtTest_Add(DS_TableCreateCDS_Test_RestoreFail, DS_Test_Setup, DS_Test_TearDown,
               "DS_TableCreateCDS_Test_RestoreFail");
    UtTest_Add(DS_TableCreateCDS_Test_Error, DS_Test_Setup, DS_Test_TearDown, "DS_TableCreateCDS_Test_Error");

    UtTest_Add(DS_TableUpdateCDS_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableUpdateCDS_Test_Nominal");
    UtTest_Add(DS_TableUpdateCDS_Test_Error, DS_Test_Setup, DS_Test_TearDown, "DS_TableUpdateCDS_Test_Error");

    UtTest_Add(DS_TableHashFunction_Test, DS_Test_Setup, DS_Test_TearDown, "DS_TableHashFunction_Test");

    UtTest_Add(DS_TableCreateHash_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_TableCreateHash_Test_Nominal");
    UtTest_Add(DS_TableCreateHash_Test_NullTable, DS_Test_Setup, DS_Test_TearDown, "DS_TableCreateHash_Test_NullTable");

    UtTest_Add(DS_TableFindMsgID_Test, DS_Test_Setup, DS_Test_TearDown, "DS_TableFindMsgID_Test");
    UtTest_Add(DS_TableFindMsgID_Test_NullTable, DS_Test_Setup, DS_Test_TearDown, "DS_TableFindMsgID_Test_NullTable");

    UtTest_Add(DS_TableFindMsgID_Test_Mismatch, DS_Test_Setup, DS_Test_TearDown, "DS_TableFindMsgID_Test_Mismatch");
} /* end DS_Table_Test_AddTestCases */

/************************/
/*  End of File Comment */
/************************/
