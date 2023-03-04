/************************************************************************
 * NASA Docket No. GSC-18,917-1, and identified as “CFS Data Storage
 * (DS) application version 2.6.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
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
#include "ds_events.h"
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
    DS_AppMain();

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
    DS_AppMain();

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
    DS_AppMain();

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
    DS_AppMain();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_STUB_COUNT(DS_FileTestAge, 1);
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
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_EID);
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

void DS_AppProcessMsg_Test_CmdStore(void)
{
    DS_HashLink_t     HashLink;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* Verifying that the DisabledPktCounter == 1 confirms that an attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 1);

    /* event message that would normally be sent by noop in production code is
     * stubbed out for the purposes of this test */
}

void DS_AppProcessMsg_Test_CmdNoStore(void)
{
    DS_HashLink_t     HashLink;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* Verifying that the DisabledPktCounter == 0 confirms that no attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 0);

    /* event message that would normally be sent by noop in production code is
     * stubbed out for the purposes of this test */
}

void DS_AppProcessMsg_Test_HKStore(void)
{
    DS_HashLink_t  HashLink;
    size_t         forced_Size  = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t forced_MsgID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[188]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    /* verifying a sent message indirectly verifies that DS_AppProcessHK was
     * called */

    /* Verifying that the DisabledPktCounter == 1 confirms that an attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppProcessMsg_Test_HKNoStore(void)
{
    DS_HashLink_t  HashLink;
    size_t         forced_Size  = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t forced_MsgID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);

    DS_AppData.AppEnableState = DS_DISABLED;

    DS_AppData.HashTable[188]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    /* verifying a sent message indirectly verifies that DS_AppProcessHK was
     * called */

    /* Verifying that the DisabledPktCounter == 0 confirms that no attempt was
     * made to store this packet (setting AppEnableState to DS_DISABLED forces
     * this counter to increment when the attempt is made). */
    UtAssert_INT32_EQ(DS_AppData.DisabledPktCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppProcessMsg_Test_HKInvalidRequest(void)
{
    size_t         forced_Size  = 0;
    CFE_SB_MsgId_t forced_MsgID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_HK_REQUEST_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AppProcessMsg_Test_UnknownMID(void)
{
    size_t         forced_Size  = 0;
    CFE_SB_MsgId_t forced_MsgID = DS_UT_MID_1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Execute the function being tested */
    DS_AppProcessMsg(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppProcessCmd_Test_Noop(void)
{
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdNoop, 1);
}

void DS_AppProcessCmd_Test_Reset(void)
{
    size_t            forced_Size    = sizeof(DS_ResetCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_RESET_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdReset, 1);
}

void DS_AppProcessCmd_Test_SetAppState(void)
{
    size_t            forced_Size    = sizeof(DS_AppStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_APP_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetAppState, 1);
}

void DS_AppProcessCmd_Test_SetFilterFile(void)
{
    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetFilterFile, 1);
}

void DS_AppProcessCmd_Test_SetFilterType(void)
{
    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetFilterType, 1);
}

void DS_AppProcessCmd_Test_SetFilterParms(void)
{
    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetFilterParms, 1);
}

void DS_AppProcessCmd_Test_SetDestType(void)
{
    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestType, 1);
}

void DS_AppProcessCmd_Test_SetDestState(void)
{
    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestState, 1);
}

void DS_AppProcessCmd_Test_SetDestPath(void)
{
    size_t            forced_Size    = sizeof(DS_DestPathCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestPath, 1);
}

void DS_AppProcessCmd_Test_SetDestBase(void)
{
    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestBase, 1);
}

void DS_AppProcessCmd_Test_SetDestExt(void)
{
    size_t            forced_Size    = sizeof(DS_DestExtCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestExt, 1);
}

void DS_AppProcessCmd_Test_SetDestSize(void)
{
    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestSize, 1);
}

void DS_AppProcessCmd_Test_SetDestAge(void)
{
    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestAge, 1);
}

void DS_AppProcessCmd_Test_SetDestCount(void)
{
    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdSetDestCount, 1);
}

void DS_AppProcessCmd_Test_CloseFile(void)
{
    uint32            i;
    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdCloseFile, 1);
}

void DS_AppProcessCmd_Test_GetFileInfo(void)
{
    size_t            forced_Size    = sizeof(DS_GetFileInfoCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_GET_FILE_INFO_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdGetFileInfo, 1);
}

void DS_AppProcessCmd_Test_AddMID(void)
{
    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdAddMID, 1);
}

void DS_AppProcessCmd_Test_RemoveMID(void)
{
    size_t            forced_Size    = sizeof(DS_RemoveMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_REMOVE_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdRemoveMID, 1);
}

void DS_AppProcessCmd_Test_CloseAll(void)
{
    uint32            i;
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_ALL_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CmdCloseAll, 1);
}

void DS_AppProcessCmd_Test_InvalidCommandCode(void)
{
    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = 99;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_AppProcessCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

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
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth, 0);

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_BOOL_TRUE(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_AppProcessHK_Test_SnprintfFail(void)
{
    uint32 i;

    /* Most values in the HK packet can't be checked because they're stored in a local variable. */

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileGrowth = 99;
    }

    UT_SetDeferredRetcode(UT_KEY(stub_snprintf), 1, -1);

    /* Execute the function being tested */
    DS_AppProcessHK();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth, 0);

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_BOOL_TRUE(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_APPHK_FILTER_TBL_PRINT_ERR_EID);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileGrowth, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileRate, 99 / DS_SECS_PER_HK_CYCLE);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileGrowth, 0);

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_BOOL_TRUE(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_HkPacket_t));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf);

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
    DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf);

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
    DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf);

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
    DS_AppStorePacket(MessageID, &UT_CmdBuf.Buf);

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

    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_CmdStore);
    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_CmdNoStore);
    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_HKStore);
    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_HKNoStore);
    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_HKInvalidRequest);
    UT_DS_TEST_ADD(DS_AppProcessMsg_Test_UnknownMID);

    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_Noop);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_Reset);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetAppState);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetFilterFile);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetFilterType);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetFilterParms);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestType);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestState);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestPath);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestBase);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestExt);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestSize);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestAge);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_SetDestCount);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_CloseFile);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_GetFileInfo);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_AddMID);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_RemoveMID);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_CloseAll);
    UT_DS_TEST_ADD(DS_AppProcessCmd_Test_InvalidCommandCode);

    UT_DS_TEST_ADD(DS_AppProcessHK_Test);
    UT_DS_TEST_ADD(DS_AppProcessHK_Test_SnprintfFail);
    UT_DS_TEST_ADD(DS_AppProcessHK_Test_TblFail);

    UT_DS_TEST_ADD(DS_AppStorePacket_Test_Nominal);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_DSDisabled);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_FilterTableNotLoaded);
    UT_DS_TEST_ADD(DS_AppStorePacket_Test_DestFileTableNotLoaded);
}
