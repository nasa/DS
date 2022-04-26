/*************************************************************************
** File: ds_app_test.c
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
**   This file contains unit test cases for the functions contained in the file ds_app.c
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

#include "ds_app_tests.h"
#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_events.h"
#include "ds_version.h"
#include "ds_test_utils.h"
/*#include "ut_utils_lib.h"*/
#include "ds_cmds.h"
#include "ds_file.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <OCS_stdio.h>

#define CMD_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_CommandHeader_t)) % 4) == 0
#define TLM_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_TelemetryHeader_t)) % 4) == 0

uint8 call_count_CFE_EVS_SendEvent;
uint8 call_count_CFE_ES_WriteToSysLog;

#define snprintf OCS_snprintf

/*
 * Function Definitions
 */

void DS_AppMain_Test_Nominal(void)
{
    CFE_SB_MsgId_t forced_MsgID = DS_SEND_HK_MID;
    size_t         forced_Size  = sizeof(DS_NoopCmd_t);

    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Set to prevent segmentation fault */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Execute the function being tested */
    DS_AppMain();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    call_count_CFE_ES_WriteToSysLog = UT_GetStubCount(UT_KEY(CFE_ES_WriteToSysLog));
    UtAssert_INT32_EQ(call_count_CFE_ES_WriteToSysLog, 0);

} /* end DS_AppMain_Test_Nominal */

void DS_AppMain_Test_AppInitializeError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application terminating, err = 0x%%08X");

    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "DS application terminating, err = 0x%%08X\n");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;
    UT_SetHookFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Utils_stub_reporter_hook, &context_CFE_ES_WriteToSysLog);

    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Set to satisfy condition "if (Result != CFE_SUCCESS)" immediately after call to DS_AppInitialize (which calls
     * CFE_EVS_Register) */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    DS_AppMain();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_EXIT_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_ES_ExitApp)), 1);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_ES_WriteToSysLog = UT_GetStubCount(UT_KEY(CFE_ES_WriteToSysLog));
    UtAssert_INT32_EQ(call_count_CFE_ES_WriteToSysLog, 1);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);

} /* end DS_AppMain_Test_AppInitializeError */

void DS_AppMain_Test_SBError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application terminating, err = 0x%%08X");

    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "DS application terminating, err = 0x%%08X\n");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent[2];
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, context_CFE_EVS_SendEvent);

    CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;
    UT_SetHookFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Utils_stub_reporter_hook, &context_CFE_ES_WriteToSysLog);

    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to fail condition "if (Result != CFE_SUCCESS)" immediately after call to CFE_SB_RcvMsg */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_PIPE_RD_ERR);

    /* Execute the function being tested */
    DS_AppMain();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);

    /* Generates 1 event message we don't care about in this test */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_EXIT_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_ES_ExitApp)), 1);

    call_count_CFE_ES_WriteToSysLog = UT_GetStubCount(UT_KEY(CFE_ES_WriteToSysLog));
    UtAssert_INT32_EQ(call_count_CFE_ES_WriteToSysLog, 1);
    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);

} /* end DS_AppMain_Test_SBError */

void DS_AppMain_Test_SBTimeout(void)
{
    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to fail condition "if (Result != CFE_SUCCESS)" immediately after call to CFE_SB_RcvMsg */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_TIME_OUT);

    /* Execute the function being tested */
    DS_AppMain();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 2);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_FileTestAge)), 1);

} /* end DS_AppMain_Test_SBTimeout */

void DS_AppInitialize_Test_Nominal(void)
{
    int32 Result;
    char  Message[125];
    int32 strCmpResult;

    char ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Application initialized, version %%d.%%d.%%d.%%d, data at %%p");

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    CFE_PSP_MemSet(&DS_AppData, 1, sizeof(DS_AppData_t));

    /* Execute the function being tested */
    Result = DS_AppInitialize();

    /* Verify results */
    UtAssert_True(DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE, "DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE");

    UtAssert_True(DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileHandle == DS_CLOSED_FILE_HANDLE");

    /* Note: not verifying that CFE_PSP_MemSet set the rest of DS_AppData to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventID, DS_INIT_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent.EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent.Spec);

} /* end DS_AppInitialize_Test_Nominal */

void DS_AppInitialize_Test_EVSRegisterError(void)
{
    int32 Result;

    /* Set to generate error message DS_INIT_ERR_EID for EVS services */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    Result = DS_AppInitialize();

    /* Verify results */
    UtAssert_True(DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE, "DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE");

    UtAssert_True(DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileHandle == DS_CLOSED_FILE_HANDLE");

    /* Note: not verifying that CFE_PSP_MemSet set the rest of DS_AppData to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */
    /*
        UtAssert_True
            (Ut_CFE_EVS_EventSent(DS_INIT_ERR_EID, CFE_EVS_ERROR, "Unable to register for EVS services, err =
       0xFFFFFFFF"), "Unable to register for EVS services, err = 0xFFFFFFFF");
    */
    UtAssert_True(Result == -1, "Result == -1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppInitialize_Test_EVSRegisterError */

void DS_AppInitialize_Test_SBCreatePipeError(void)
{
    int32 Result;

    /* Set to generate error message DS_INIT_ERR_EID for input pipe */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    Result = DS_AppInitialize();

    /* Verify results */
    UtAssert_True(DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE, "DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE");

    UtAssert_True(DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileHandle == DS_CLOSED_FILE_HANDLE");

    /* Note: not verifying that CFE_PSP_MemSet set the rest of DS_AppData to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */

    /*UtAssert_True
        (Ut_CFE_EVS_EventSent(DS_INIT_ERR_EID, CFE_EVS_ERROR, "Unable to create input pipe, err = 0xFFFFFFFF"),
        "Unable to create input pipe, err = 0xFFFFFFFF");
*/

    UtAssert_True(Result == -1, "Result == -1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppInitialize_Test_SBCreatePipeError */

void DS_AppInitialize_Test_SBSubscribeHKError(void)
{
    int32 Result;

    /* Set to generate error message DS_INIT_ERR_EID for HK request */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    Result = DS_AppInitialize();

    /* Verify results */
    UtAssert_True(DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE, "DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE");

    UtAssert_True(DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileHandle == DS_CLOSED_FILE_HANDLE");

    /* Note: not verifying that CFE_PSP_MemSet set the rest of DS_AppData to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */
    /*
        UtAssert_True
            (Ut_CFE_EVS_EventSent(DS_INIT_ERR_EID, CFE_EVS_ERROR, "Unable to subscribe to HK request, err =
       0xFFFFFFFF"), "Unable to subscribe to HK request, err = 0xFFFFFFFF");
    */

    UtAssert_True(Result == -1, "Result == -1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppInitialize_Test_SBSubscribeHKError */

void DS_AppInitialize_Test_SBSubscribeDSError(void)
{
    int32 Result;

    /* Set to generate error message DS_INIT_ERR_EID for DS commands */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    Result = DS_AppInitialize();

    /* Verify results */
    UtAssert_True(DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE, "DS_AppData.AppEnableState == DS_DEF_ENABLE_STATE");

    UtAssert_True(DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[0].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileHandle == DS_CLOSED_FILE_HANDLE");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT-1].FileHandle == DS_CLOSED_FILE_HANDLE");

    /* Note: not verifying that CFE_PSP_MemSet set the rest of DS_AppData to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */

    /*   UtAssert_True
           (Ut_CFE_EVS_EventSent(DS_INIT_ERR_EID, CFE_EVS_ERROR, "Unable to subscribe to DS commands, err =
       0xFFFFFFFF"), "Unable to subscribe to DS commands, err = 0xFFFFFFFF");
   */
    UtAssert_True(Result == -1, "Result == -1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppInitialize_Test_SBSubscribeDSError */

void DS_AppProcessMsg_Test_CmdStore(void)
{
    DS_NoopCmd_t      CmdPacket;
    DS_HashLink_t     HashLink;
    DS_FilterTable_t  FilterTable;
    char              Message[125];
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = 6331;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    /* Verifying that the DisabledPktCounter == 1 confirms that an attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 1);

    /* event message that would normally be sent by noop in production code is
     * stubbed out for the purposes of this test */

} /* end DS_AppProcessMsg_Test_CmdStore */

void DS_AppProcessMsg_Test_CmdNoStore(void)
{
    DS_NoopCmd_t      CmdPacket;
    DS_HashLink_t     HashLink;
    DS_FilterTable_t  FilterTable;
    char              Message[125];
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = 6331;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    /* Verifying that the DisabledPktCounter == 0 confirms that no attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 0);

    /* event message that would normally be sent by noop in production code is
     * stubbed out for the purposes of this test */

} /* end DS_AppProcessMsg_Test_CmdNoStore */

void DS_AppProcessMsg_Test_HKStore(void)
{
    DS_HkPacket_t    CmdPacket;
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;
    size_t           forced_Size  = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t   forced_MsgID = DS_SEND_HK_MID;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[188]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = 6332;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)), 1);
    /* verifying a sent message indirectly verifies that DS_AppProcessHK was
     * called */

    /* Verifying that the DisabledPktCounter == 1 confirms that an attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppProcessMsg_Test_HKStore */

void DS_AppProcessMsg_Test_HKNoStore(void)
{
    DS_HkPacket_t    CmdPacket;
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;
    size_t           forced_Size  = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t   forced_MsgID = DS_SEND_HK_MID;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[188]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = 6332;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)), 1);
    /* verifying a sent message indirectly verifies that DS_AppProcessHK was
     * called */

    /* Verifying that the DisabledPktCounter == 0 confirms that no attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppProcessMsg_Test_HKNoStore */

void DS_AppProcessMsg_Test_HKInvalidRequest(void)
{
    DS_HkPacket_t CmdPacket;

    size_t         forced_Size  = 0;
    CFE_SB_MsgId_t forced_MsgID = DS_SEND_HK_MID;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    /*UtAssert_True
        (Ut_CFE_EVS_EventSent(DS_HK_REQUEST_ERR_EID, CFE_EVS_ERROR, "Invalid HK request length: expected = 8, actual =
       0"), "Invalid HK request length: expected = 8, actual = 0");
*/
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppProcessMsg_Test_HKInvalidRequest */

void DS_AppProcessMsg_Test_UnknownMID(void)
{
    DS_HkPacket_t CmdPacket;

    size_t         forced_Size  = 0;
    CFE_SB_MsgId_t forced_MsgID = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Execute the function being tested */
    DS_AppProcessMsg((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppProcessMsg_Test_UnknownMID */

void DS_AppProcessCmd_Test_Noop(void)
{
    DS_NoopCmd_t      CmdPacket;
    char              Message[125];
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdNoop)), 1);

} /* end DS_AppProcessCmd_Test_Noop */

void DS_AppProcessCmd_Test_Reset(void)
{
    DS_ResetCmd_t     CmdPacket;
    size_t            forced_Size    = sizeof(DS_ResetCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_RESET_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdReset)), 1);

} /* end DS_AppProcessCmd_Test_Reset */

void DS_AppProcessCmd_Test_SetAppState(void)
{
    DS_AppStateCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_AppStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_APP_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetAppState)), 1);
} /* end DS_AppProcessCmd_Test_SetAppState */

void DS_AppProcessCmd_Test_SetFilterFile(void)
{
    DS_FilterFileCmd_t CmdPacket;
    size_t             forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetFilterFile)), 1);
} /* end DS_AppProcessCmd_Test_SetFilterFile */

void DS_AppProcessCmd_Test_SetFilterType(void)
{
    DS_FilterTypeCmd_t CmdPacket;
    size_t             forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetFilterType)), 1);

} /* end DS_AppProcessCmd_Test_SetFilterType */

void DS_AppProcessCmd_Test_SetFilterParms(void)
{
    DS_FilterParmsCmd_t CmdPacket;
    size_t              forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t      forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t   forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetFilterParms)), 1);
} /* end DS_AppProcessCmd_Test_SetFilterParms */

void DS_AppProcessCmd_Test_SetDestType(void)
{
    DS_DestTypeCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestType)), 1);
} /* end DS_AppProcessCmd_Test_SetDestType */

void DS_AppProcessCmd_Test_SetDestState(void)
{
    DS_DestStateCmd_t CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestState)), 1);
} /* end DS_AppProcessCmd_Test_SetDestState */

void DS_AppProcessCmd_Test_SetDestPath(void)
{
    DS_DestPathCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestPathCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestPath)), 1);
} /* end DS_AppProcessCmd_Test_SetDestPath */

void DS_AppProcessCmd_Test_SetDestBase(void)
{
    DS_DestBaseCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestBase)), 1);
} /* end DS_AppProcessCmd_Test_SetDestBase */

void DS_AppProcessCmd_Test_SetDestExt(void)
{
    DS_DestExtCmd_t   CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestExtCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestExt)), 1);

} /* end DS_AppProcessCmd_Test_SetDestExt */

void DS_AppProcessCmd_Test_SetDestSize(void)
{
    DS_DestSizeCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestSize)), 1);
} /* end DS_AppProcessCmd_Test_SetDestSize */

void DS_AppProcessCmd_Test_SetDestAge(void)
{
    DS_DestAgeCmd_t   CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestAge)), 1);
} /* end DS_AppProcessCmd_Test_SetDestAge */

void DS_AppProcessCmd_Test_SetDestCount(void)
{
    DS_DestCountCmd_t CmdPacket;
    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdSetDestCount)), 1);
} /* end DS_AppProcessCmd_Test_SetDestCount */

void DS_AppProcessCmd_Test_CloseFile(void)
{
    DS_CloseFileCmd_t CmdPacket;
    uint32            i;
    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = DS_CLOSED_FILE_HANDLE;
    }

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdCloseFile)), 1);
} /* end DS_AppProcessCmd_Test_CloseFile */

void DS_AppProcessCmd_Test_GetFileInfo(void)
{
    DS_GetFileInfoCmd_t CmdPacket;
    size_t              forced_Size    = sizeof(DS_GetFileInfoCmd_t);
    CFE_SB_MsgId_t      forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t   forced_CmdCode = DS_GET_FILE_INFO_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdGetFileInfo)), 1);
} /* end DS_AppProcessCmd_Test_GetFileInfo */

void DS_AppProcessCmd_Test_AddMID(void)
{
    DS_AddMidCmd_t    CmdPacket;
    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdAddMID)), 1);
} /* end DS_AppProcessCmd_Test_AddMID */

void DS_AppProcessCmd_Test_CloseAll(void)
{
    DS_CloseAllCmd_t  CmdPacket;
    uint32            i;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_ALL_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = DS_CLOSED_FILE_HANDLE;
    }

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_CmdCloseAll)), 1);
} /* end DS_AppProcessCmd_Test_CloseAll */

void DS_AppProcessCmd_Test_InvalidCommandCode(void)
{
    DS_CloseAllCmd_t  CmdPacket;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd((CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_AppProcessCmd_Test_InvalidCommandCode */

void DS_AppProcessHK_Test(void)
{
    uint32 i;

    /* Most values in the HK packet can't be checked because they're stored in a local variable. */

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileGrowth = 99;
    }

    /* Execute the function being tested */
    DS_AppProcessHK();

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[0].FileGrowth == 0, "DS_AppData.FileStatus[0].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0");

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)), 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t), "DS_HkPacket_t is 32-bit aligned");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppProcessHK_Test */

/**
 * TODO: This test has a known issue in which the return value of snprintf is
 * not being correctly forced to fail.
 */
void DS_AppProcessHK_Test_SnprintfFail(void)
{
    uint32 i;

    /* Most values in the HK packet can't be checked because they're stored in a local variable. */

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileGrowth = 99;
    }

    UT_SetDeferredRetcode(UT_KEY(OCS_snprintf), 1, -1);

    /* Execute the function being tested */
    DS_AppProcessHK();

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[0].FileGrowth == 0, "DS_AppData.FileStatus[0].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0");

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)), 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t), "DS_HkPacket_t is 32-bit aligned");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
}

void DS_AppProcessHK_Test_TblFail(void)
{
    uint32 i;

    /* Most values in the HK packet can't be checked because they're stored in a local variable. */

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileGrowth = 99;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetInfo), -1);

    /* Execute the function being tested */
    DS_AppProcessHK();

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[0].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[0].FileGrowth == 0, "DS_AppData.FileStatus[0].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT/2].FileGrowth == 0");

    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate == 99 / DS_SECS_PER_HK_CYCLE");
    UtAssert_True(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0,
                  "DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth == 0");

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(CFE_SB_TransmitMsg)), 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t), "DS_HkPacket_t is 32-bit aligned");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
}

void DS_AppStorePacket_Test_Nominal(void)
{
    DS_CloseAllCmd_t   CmdPacket;
    CFE_SB_MsgId_t     MessageID = 1;
    DS_DestFileTable_t destTable;
    DS_FilterTable_t   filterTable;
    size_t             forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;
    DS_AppData.DestFileTblPtr = &destTable;
    DS_AppData.FilterTblPtr   = &filterTable;

    /* Execute the function being tested */
    DS_AppStorePacket(MessageID, (CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results -- IgnoredPktCounter increments in call to DS_FileStorePacket() */
    UtAssert_True(DS_AppData.IgnoredPktCounter == 0, "DS_AppData.IgnoredPktCounter == 0");

    UtAssert_True(DS_AppData.DisabledPktCounter == 0, "DS_AppData.DisabledPktCounter == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_FileStorePacket)), 1);
} /* end DS_AppStorePacket_Test_Nominal */

void DS_AppStorePacket_Test_DSDisabled(void)
{
    DS_CloseAllCmd_t  CmdPacket;
    CFE_SB_MsgId_t    MessageID      = 1;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_DISABLED;

    /* Execute the function being tested */
    DS_AppStorePacket(MessageID, (CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(DS_AppData.DisabledPktCounter == 1, "DS_AppData.DisabledPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppStorePacket_Test_DSDisabled */

void DS_AppStorePacket_Test_FilterTableNotLoaded(void)
{
    DS_CloseAllCmd_t   CmdPacket;
    CFE_SB_MsgId_t     MessageID = 1;
    DS_DestFileTable_t destTable;
    size_t             forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;
    DS_AppData.DestFileTblPtr = &destTable; /* force to non-zero so filter table is tested */
    DS_AppData.FilterTblPtr   = 0;

    /* Execute the function being tested */
    DS_AppStorePacket(MessageID, (CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(DS_AppData.IgnoredPktCounter == 1, "DS_AppData.IgnoredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppStorePacket_Test_FilterTableNotLoaded */

void DS_AppStorePacket_Test_DestFileTableNotLoaded(void)
{
    DS_CloseAllCmd_t  CmdPacket;
    CFE_SB_MsgId_t    MessageID = 1;
    DS_FilterTable_t  filterTable;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;
    DS_AppData.FilterTblPtr   = &filterTable; /* Force to non-zero so destination table is tested */
    DS_AppData.DestFileTblPtr = 0;

    /* Execute the function being tested */
    DS_AppStorePacket(MessageID, (CFE_SB_Buffer_t *)(&CmdPacket));

    /* Verify results */
    UtAssert_True(DS_AppData.IgnoredPktCounter == 1, "DS_AppData.IgnoredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_AppStorePacket_Test_DestFileTableNotLoaded */

void UtTest_Setup(void)
{
    UtTest_Add(DS_AppMain_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_AppMain_Test_Nominal");
    UtTest_Add(DS_AppMain_Test_AppInitializeError, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppMain_Test_AppInitializeError");
    UtTest_Add(DS_AppMain_Test_SBError, DS_Test_Setup, DS_Test_TearDown, "DS_AppMain_Test_SBError");
    UtTest_Add(DS_AppMain_Test_SBTimeout, DS_Test_Setup, DS_Test_TearDown, "DS_AppMain_Test_SBTimeout");

    UtTest_Add(DS_AppInitialize_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_AppInitialize_Test_Nominal");
    UtTest_Add(DS_AppInitialize_Test_EVSRegisterError, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppInitialize_Test_EVSRegisterError");
    UtTest_Add(DS_AppInitialize_Test_SBCreatePipeError, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppInitialize_Test_SBCreatePipeError");
    UtTest_Add(DS_AppInitialize_Test_SBSubscribeHKError, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppInitialize_Test_SBSubscribeHKError");
    UtTest_Add(DS_AppInitialize_Test_SBSubscribeDSError, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppInitialize_Test_SBSubscribeDSError");

    UtTest_Add(DS_AppProcessMsg_Test_CmdStore, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessMsg_Test_CmdStore");
    UtTest_Add(DS_AppProcessMsg_Test_CmdNoStore, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessMsg_Test_CmdNoStore");
    UtTest_Add(DS_AppProcessMsg_Test_HKStore, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessMsg_Test_HKStore");
    UtTest_Add(DS_AppProcessMsg_Test_HKNoStore, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessMsg_Test_HKNoStore");
    UtTest_Add(DS_AppProcessMsg_Test_HKInvalidRequest, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessMsg_Test_HKInvalidRequest");
    UtTest_Add(DS_AppProcessMsg_Test_UnknownMID, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessMsg_Test_UnknownMID");

    UtTest_Add(DS_AppProcessCmd_Test_Noop, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_Noop");
    UtTest_Add(DS_AppProcessCmd_Test_Reset, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_Reset");
    UtTest_Add(DS_AppProcessCmd_Test_SetAppState, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetAppState");
    UtTest_Add(DS_AppProcessCmd_Test_SetFilterFile, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_SetFilterFile");
    UtTest_Add(DS_AppProcessCmd_Test_SetFilterType, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_SetFilterType");
    UtTest_Add(DS_AppProcessCmd_Test_SetFilterParms, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_SetFilterParms");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestType, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestType");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestState, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_SetDestState");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestPath, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestPath");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestBase, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestBase");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestExt, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestExt");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestSize, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestSize");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestAge, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_SetDestAge");
    UtTest_Add(DS_AppProcessCmd_Test_SetDestCount, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_SetDestCount");
    UtTest_Add(DS_AppProcessCmd_Test_CloseFile, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_CloseFile");
    UtTest_Add(DS_AppProcessCmd_Test_GetFileInfo, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_GetFileInfo");
    UtTest_Add(DS_AppProcessCmd_Test_AddMID, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_AddMID");
    UtTest_Add(DS_AppProcessCmd_Test_CloseAll, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessCmd_Test_CloseAll");
    UtTest_Add(DS_AppProcessCmd_Test_InvalidCommandCode, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppProcessCmd_Test_InvalidCommandCode");

    UtTest_Add(DS_AppProcessHK_Test, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessHK_Test");
    /* UtTest_Add(DS_AppProcessHK_Test_SnprintfFail, DS_Test_Setup, DS_Test_TearDown,
     * "DS_AppProcessHK_Test_SnprintfFail"); */
    UtTest_Add(DS_AppProcessHK_Test_TblFail, DS_Test_Setup, DS_Test_TearDown, "DS_AppProcessHK_Test_TblFail");

    UtTest_Add(DS_AppStorePacket_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_AppStorePacket_Test_Nominal");
    UtTest_Add(DS_AppStorePacket_Test_DSDisabled, DS_Test_Setup, DS_Test_TearDown, "DS_AppStorePacket_Test_DSDisabled");
    UtTest_Add(DS_AppStorePacket_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppStorePacket_Test_FilterTableNotLoaded");
    UtTest_Add(DS_AppStorePacket_Test_DestFileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_AppStorePacket_Test_DestFileTableNotLoaded");
} /* end UtTest_Setup */

/************************/
/*  End of File Comment */
/************************/
