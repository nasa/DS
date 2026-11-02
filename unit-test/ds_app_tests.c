/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   This file contains unit test cases for the functions contained in the file ds_app.c
 */

/*
 * Includes
 */

#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_eventids.h"
#include "ds_version.h"
#include "ds_test_utils.h"
#include "ds_cmds.h"
#include "ds_file.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* Overrides */
#include "stub_stdio.h"

#include <unistd.h>
#include <stdlib.h>

#define CMD_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_CommandHeader_t)) % 4) == 0
#define TLM_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_TelemetryHeader_t)) % 4) == 0

uint8 call_count_CFE_EVS_SendEvent;
uint8 call_count_CFE_ES_WriteToSysLog;

/*
 * Function Definitions
 */

void DS_AppMain_Test_Nominal(void)
{
    CFE_SB_MsgId_t forced_MsgID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);
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
    UtAssert_VOIDCALL(DS_AppMain());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 0);
}

void DS_AppMain_Test_AppInitializeError(void)
{
    /* Set to exit loop after first run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to prevent call to CFE_SB_RcvMsg from returning an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Set to satisfy condition "if (Result != CFE_SUCCESS)" immediately after call to DS_AppInitialize (which calls
     * CFE_EVS_Register) */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppMain());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_CRITICAL);
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 2);
}

void DS_AppMain_Test_SBError(void)
{
    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to fail condition "if (Result != CFE_SUCCESS)" immediately after call to CFE_SB_RcvMsg */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_PIPE_RD_ERR);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppMain());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_EXIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);
    UtAssert_STUB_COUNT(CFE_ES_ExitApp, 1);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);
}

void DS_AppMain_Test_SBTimeout(void)
{
    /* Set to exit loop after first run */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RunLoop), true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 2, false);

    /* Set to fail condition "if (Result != CFE_SUCCESS)" immediately after call to CFE_SB_RcvMsg */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_TIME_OUT);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppMain());

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void DS_AppInitialize_Test_Nominal(void)
{
    memset(&DS_AppData, 1, sizeof(DS_AppData));

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_AppInitialize(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.AppEnableState, DS_DEF_ENABLE_STATE);

    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[0].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle));

    /* Note: not verifying the rest of DS_AppData is set to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_AppInitialize_Test_EVSRegisterError(void)
{
    /* Set to generate error message DS_INIT_ERR_EID for EVS services */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_AppInitialize(), -1);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.AppEnableState, DS_DEF_ENABLE_STATE);

    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[0].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle));

    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 1);
}

void DS_AppInitialize_Test_SBCreatePipeError(void)
{
    /* Set to generate error message DS_INIT_ERR_EID for input pipe */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_AppInitialize(), -1);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.AppEnableState, DS_DEF_ENABLE_STATE);

    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[0].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle));

    /* Note: not verifying that the rest of DS_AppData is set to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AppInitialize_Test_SBSubscribeHKError(void)
{
    /* Set to generate error message DS_INIT_ERR_EID for HK request */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_AppInitialize(), -1);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.AppEnableState, DS_DEF_ENABLE_STATE);

    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[0].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle));

    /* Note: not verifying that the rest of DS_AppData is set to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AppInitialize_Test_SBSubscribeDSError(void)
{
    /* Set to generate error message DS_INIT_ERR_EID for DS commands */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_AppInitialize(), -1);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.AppEnableState, DS_DEF_ENABLE_STATE);

    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[0].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileHandle));
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileHandle));

    /* Note: not verifying that the rest of DS_AppData is set to 0, because some elements of DS_AppData
     * are modified by subfunctions, which we're not testing here */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AppStorePacket_Test_Nominal(void)
{
    CFE_SB_MsgId_t    MessageID      = DS_UT_MID_1;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf));

    /* Verify results -- IgnoredPktCounter increments in call to DS_FileStorePacket() */
    UtAssert_UINT32_EQ(DS_AppData.IgnoredPktCounter, 0);
    UtAssert_UINT32_EQ(DS_AppData.DisabledPktCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(DS_FileStorePacket, 1);
}

void DS_AppStorePacket_Test_DSDisabled(void)
{
    CFE_SB_MsgId_t    MessageID      = DS_UT_MID_1;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_DISABLED;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.DisabledPktCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppStorePacket_Test_FilterTableNotLoaded(void)
{
    CFE_SB_MsgId_t    MessageID      = DS_UT_MID_1;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;
    DS_AppData.FilterTblPtr   = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.IgnoredPktCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppStorePacket_Test_DestFileTableNotLoaded(void)
{
    CFE_SB_MsgId_t    MessageID      = DS_UT_MID_1;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.AppEnableState = DS_ENABLED;
    DS_AppData.DestFileTblPtr = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.IgnoredPktCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void UtTest_Setup(void)
{
    UT_DS_TEST_ADD(DS_AppMain_Test_Nominal);
    UT_DS_TEST_ADD(DS_AppMain_Test_AppInitializeError);
    UT_DS_TEST_ADD(DS_AppMain_Test_SBError);
    UT_DS_TEST_ADD(DS_AppMain_Test_SBTimeout);

    UT_DS_TEST_ADD(DS_AppInitialize_Test_Nominal);
    UT_DS_TEST_ADD(DS_AppInitialize_Test_EVSRegisterError);
    UT_DS_TEST_ADD(DS_AppInitialize_Test_SBCreatePipeError);
    UT_DS_TEST_ADD(DS_AppInitialize_Test_SBSubscribeHKError);
    UT_DS_TEST_ADD(DS_AppInitialize_Test_SBSubscribeDSError);

    UT_DS_TEST_ADD(DS_AppStorePacket_Test_Nominal);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_DSDisabled);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_FilterTableNotLoaded);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_DestFileTableNotLoaded);
}
