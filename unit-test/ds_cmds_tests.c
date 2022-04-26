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
 *   This file contains unit test cases for the functions contained in the file ds_cmds.c
 */

/*
 * Includes
 */

#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_cmds.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_events.h"
#include "ds_version.h"
#include "ds_file.h"
#include "ds_test_utils.h"
/*#include "ut_utils_lib.h"*/

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

#define CMD_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_CommandHeader_t)) % 4) == 0
#define TLM_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_TelemetryHeader_t)) % 4) == 0

uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void DS_CmdNoop_Test_Nominal(void)
{

    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "NOOP command, Version %%d.%%d.%%d.%%d");

    /* Execute the function being tested */
    DS_CmdNoop(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NOOP_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_NoopCmd_t), "DS_NoopCmd_t is 32-bit aligned");

} /* end DS_CmdNoop_Test_Nominal */

void DS_CmdNoop_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_NoopCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid NOOP command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdNoop(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NOOP_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_CmdNoop_Test_InvalidCommandLength */

void DS_CmdReset_Test_Nominal(void)
{

    size_t            forced_Size    = sizeof(DS_ResetCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_RESET_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Reset counters command");

    /* Execute the function being tested */
    DS_CmdReset(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 0, "DS_AppData.CmdAcceptedCounter == 0");
    UtAssert_True(DS_AppData.CmdRejectedCounter == 0, "DS_AppData.CmdRejectedCounter == 0");
    UtAssert_True(DS_AppData.DisabledPktCounter == 0, "DS_AppData.DisabledPktCounter == 0");
    UtAssert_True(DS_AppData.IgnoredPktCounter == 0, "DS_AppData.IgnoredPktCounter == 0");
    UtAssert_True(DS_AppData.FilteredPktCounter == 0, "DS_AppData.FilteredPktCounter == 0");
    UtAssert_True(DS_AppData.PassedPktCounter == 0, "DS_AppData.PassedPktCounter == 0");
    UtAssert_True(DS_AppData.FileWriteCounter == 0, "DS_AppData.FileWriteCounter == 0");
    UtAssert_True(DS_AppData.FileWriteErrCounter == 0, "DS_AppData.FileWriteErrCounter == 0");
    UtAssert_True(DS_AppData.FileUpdateCounter == 0, "DS_AppData.FileUpdateCounter == 0");
    UtAssert_True(DS_AppData.FileUpdateErrCounter == 0, "DS_AppData.FileUpdateErrCounter == 0");
    UtAssert_True(DS_AppData.DestTblLoadCounter == 0, "DS_AppData.DestTblLoadCounter == 0");
    UtAssert_True(DS_AppData.DestTblErrCounter == 0, "DS_AppData.DestTblErrCounter == 0");
    UtAssert_True(DS_AppData.FilterTblLoadCounter == 0, "DS_AppData.FilterTblLoadCounter == 0");
    UtAssert_True(DS_AppData.FilterTblErrCounter == 0, "DS_AppData.FilterTblErrCounter == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_RESET_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_ResetCmd_t), "DS_ResetCmd_t is 32-bit aligned");

} /* end DS_CmdReset_Test_Nominal */

void DS_CmdReset_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_ResetCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_RESET_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid RESET command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdReset(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_RESET_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_CmdReset_Test_InvalidCommandLength */

void DS_CmdSetAppState_Test_Nominal(void)
{

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "APP STATE command: state = %%d");

    size_t            forced_Size    = sizeof(DS_AppStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_APP_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_CmdBuf.AppStateCmd.EnableState = true;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_CmdSetAppState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.CmdRejectedCounter, 0);
    UtAssert_INT32_EQ(DS_AppData.CmdAcceptedCounter, 1);
    UtAssert_True(DS_AppData.AppEnableState == true, "DS_AppData.AppEnableState == true");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ENADIS_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_AppStateCmd_t), "DS_AppStateCmd_t is 32-bit aligned");

} /* end DS_CmdSetAppState_Test_Nominal */

void DS_CmdSetAppState_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_AppStateCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_APP_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid APP STATE command length: expected = %%d, actual = %%d");

    UT_CmdBuf.AppStateCmd.EnableState = true;

    /* Execute the function being tested */
    DS_CmdSetAppState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ENADIS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_CmdSetAppState_Test_InvalidCommandLength */

void DS_CmdSetAppState_Test_InvalidAppState(void)
{

    size_t            forced_Size    = sizeof(DS_AppStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_APP_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid APP STATE command arg: app state = %%d");

    UT_CmdBuf.AppStateCmd.EnableState = 99;

    /* Execute the function being tested */
    DS_CmdSetAppState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ENADIS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_CmdSetAppState_Test_InvalidAppState */

void DS_CmdSetFilterFile_Test_Nominal(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size             = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID            = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode          = DS_SET_FILTER_FILE_CC;
    int32             forced_FilterTableIndex = 1;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "FILTER FILE command: MID = 0x%%08lX, index = %%d, filter = %%d, file = %%d");

    UT_CmdBuf.FilterFileCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterFileCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterFileCmd.FileTableIndex   = 4;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex]
        .Filter[UT_CmdBuf.FilterFileCmd.FilterParmsIndex]
        .FileTableIndex = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), forced_FilterTableIndex);

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex]
                          .Filter[UT_CmdBuf.FilterFileCmd.FilterParmsIndex]
                          .FileTableIndex == UT_CmdBuf.FilterFileCmd.FileTableIndex,
                  "DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex].Filter[UT_CmdBuf.FilterFileCmd."
                  "FilterParmsIndex].FileTableIndex == UT_CmdBuf.FilterFileCmd.FileTableIndex");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterFileCmd_t), "DS_FilterFileCmd_t is 32-bit aligned");
} /* end DS_CmdSetFilterFile_Test_Nominal */

void DS_CmdSetFilterFile_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_InvalidCommandLength */

void DS_CmdSetFilterFile_Test_InvalidMessageID(void)
{

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command arg: invalid messageID = 0x%%08lX");

    UT_CmdBuf.FilterFileCmd.MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_InvalidMessageID */

void DS_CmdSetFilterFile_Test_InvalidFilterParametersIndex(void)
{

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command arg: filter parameters index = %%d");

    UT_CmdBuf.FilterFileCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterFileCmd.FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_InvalidFilterParametersIndex */

void DS_CmdSetFilterFile_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command arg: file table index = %%d");

    UT_CmdBuf.FilterFileCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterFileCmd.FilterParmsIndex = 1;
    UT_CmdBuf.FilterFileCmd.FileTableIndex   = 99;

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_InvalidFileTableIndex */

void DS_CmdSetFilterFile_Test_FilterTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command: packet filter table is not loaded");

    UT_CmdBuf.FilterFileCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterFileCmd.FilterParmsIndex = 1;
    UT_CmdBuf.FilterFileCmd.FileTableIndex   = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_FilterTableNotLoaded */

void DS_CmdSetFilterFile_Test_MessageIDNotInFilterTable(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER FILE command: Message ID 0x%%08lX is not in filter table");

    UT_CmdBuf.FilterFileCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterFileCmd.MessageID        = DS_UT_MID_2;
    UT_CmdBuf.FilterFileCmd.FileTableIndex   = 4;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_CmdSetFilterFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterFile_Test_MessageIDNotInFilterTable */

void DS_CmdSetFilterType_Test_Nominal(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "FILTER TYPE command: MID = 0x%%08lX, index = %%d, filter = %%d, type = %%d");

    UT_CmdBuf.FilterTypeCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterTypeCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterTypeCmd.FilterType       = 1;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterTypeCmd.FilterParmsIndex].FilterType == 1,
        "DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterTypeCmd.FilterParmsIndex].FilterType == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterTypeCmd_t), "DS_FilterTypeCmd_t is 32-bit aligned");
} /* end DS_CmdSetFilterType_Test_Nominal */

void DS_CmdSetFilterType_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_InvalidCommandLength */

void DS_CmdSetFilterType_Test_InvalidMessageID(void)
{

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command arg: invalid messageID = 0x%%08lX");

    UT_CmdBuf.FilterTypeCmd.MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_InvalidMessageID */

void DS_CmdSetFilterType_Test_InvalidFilterParametersIndex(void)
{

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command arg: filter parameters index = %%d");

    UT_CmdBuf.FilterTypeCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterTypeCmd.FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_InvalidFilterParametersIndex */

void DS_CmdSetFilterType_Test_InvalidFilterType(void)
{

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command arg: filter type = %%d");

    UT_CmdBuf.FilterTypeCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterTypeCmd.FilterParmsIndex = 1;
    UT_CmdBuf.FilterTypeCmd.FilterType       = false;

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_InvalidFilterType */

void DS_CmdSetFilterType_Test_FilterTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command: packet filter table is not loaded");

    UT_CmdBuf.FilterTypeCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterTypeCmd.FilterParmsIndex = 1;
    UT_CmdBuf.FilterTypeCmd.FilterType       = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_FilterTableNotLoaded */

void DS_CmdSetFilterType_Test_MessageIDNotInFilterTable(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER TYPE command: Message ID 0x%%08lX is not in filter table");

    UT_CmdBuf.FilterTypeCmd.MessageID        = DS_UT_MID_2;
    UT_CmdBuf.FilterTypeCmd.FilterParmsIndex = 1;
    UT_CmdBuf.FilterTypeCmd.FilterType       = 1;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_CmdSetFilterType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterType_Test_MessageIDNotInFilterTable */

void DS_CmdSetFilterParms_Test_Nominal(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "FILTER PARMS command: MID = 0x%%08lX, index = %%d, filter = %%d, N = %%d, X = %%d, O = %%d");

    UT_CmdBuf.FilterParmsCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterParmsCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterParmsCmd.Algorithm_N      = 0;
    UT_CmdBuf.FilterParmsCmd.Algorithm_X      = 0;
    UT_CmdBuf.FilterParmsCmd.Algorithm_O      = 0;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_N == 0,
        "DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_N == 0");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_X == 0,
        "DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_X == 0");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_O == 0,
        "DS_AppData.FilterTblPtr->Packet[0].Filter[UT_CmdBuf.FilterParmsCmd.FilterParmsIndex].Algorithm_O == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterParmsCmd_t), "DS_FilterParmsCmd_t is 32-bit aligned");

} /* end DS_CmdSetFilterParms_Test_Nominal */

void DS_CmdSetFilterParms_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterParms_Test_InvalidCommandLength */

void DS_CmdSetFilterParms_Test_InvalidMessageID(void)
{

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command arg: invalid messageID = 0x%%08lX");

    UT_CmdBuf.FilterParmsCmd.MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterParms_Test_InvalidMessageID */

void DS_CmdSetFilterParms_Test_InvalidFilterParametersIndex(void)
{

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command arg: filter parameters index = %%d");

    UT_CmdBuf.FilterParmsCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterParmsCmd.FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterParms_Test_InvalidFilterParametersIndex */

void DS_CmdSetFilterParms_Test_InvalidFilterAlgorithm(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command arg: N = %%d, X = %%d, O = %%d");

    UT_CmdBuf.FilterParmsCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterParmsCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterParmsCmd.Algorithm_N      = 1;
    UT_CmdBuf.FilterParmsCmd.Algorithm_X      = 1;
    UT_CmdBuf.FilterParmsCmd.Algorithm_O      = 1;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_CmdSetFilterParms_Test_InvalidFilterAlgorithm */

void DS_CmdSetFilterParms_Test_FilterTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command: packet filter table is not loaded");

    UT_CmdBuf.FilterParmsCmd.MessageID        = DS_UT_MID_1;
    UT_CmdBuf.FilterParmsCmd.FilterParmsIndex = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterParms_Test_FilterTableNotLoaded */

void DS_CmdSetFilterParms_Test_MessageIDNotInFilterTable(void)
{
    DS_HashLink_t    HashLink;
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_FilterParmsCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_FILTER_PARMS_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid FILTER PARMS command: Message ID 0x%%08lX is not in filter table");

    UT_CmdBuf.FilterParmsCmd.FilterParmsIndex = 2;
    UT_CmdBuf.FilterParmsCmd.MessageID        = DS_UT_MID_2;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_CmdSetFilterParms(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetFilterParms_Test_MessageIDNotInFilterTable */

void DS_CmdSetDestType_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST TYPE command: file table index = %%d, filename type = %%d");

    UT_CmdBuf.DestTypeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestTypeCmd.FileNameType   = 2;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_CmdSetDestType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestTypeCmd.FileTableIndex].FileNameType == 2,
                  "DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestTypeCmd.FileTableIndex].FileNameType == 2");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestTypeCmd_t), "DS_DestTypeCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestType_Test_Nominal */

void DS_CmdSetDestType_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST TYPE command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetDestType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestType_Test_InvalidCommandLength */

void DS_CmdSetDestType_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST TYPE command arg: file table index = %%d");

    UT_CmdBuf.DestTypeCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdSetDestType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestType_Test_InvalidFileTableIndex */

void DS_CmdSetDestType_Test_InvalidFilenameType(void)
{

    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST TYPE command arg: filename type = %%d");

    UT_CmdBuf.DestTypeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestTypeCmd.FileNameType   = 99;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestType_Test_InvalidFilenameType */

void DS_CmdSetDestType_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestTypeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_TYPE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST TYPE command: destination file table is not loaded");

    UT_CmdBuf.DestTypeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestTypeCmd.FileNameType   = 2;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_CmdSetDestType(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestType_Test_FileTableNotLoaded */

void DS_CmdSetDestState_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST STATE command: file table index = %%d, file state = %%d");

    UT_CmdBuf.DestStateCmd.FileTableIndex = 1;
    UT_CmdBuf.DestStateCmd.EnableState    = 1;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_CmdSetDestState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestStateCmd.FileTableIndex].EnableState ==
                      UT_CmdBuf.DestStateCmd.EnableState,
                  "DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestStateCmd.FileTableIndex].EnableState == "
                  "UT_CmdBuf.DestStateCmd.EnableState");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestStateCmd_t), "DS_DestStateCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestState_Test_Nominal */

void DS_CmdSetDestState_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestStateCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST STATE command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetDestState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestState_Test_InvalidCommandLength */

void DS_CmdSetDestState_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST STATE command arg: file table index = %%d");

    UT_CmdBuf.DestStateCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdSetDestState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestState_Test_InvalidFileTableIndex */

void DS_CmdSetDestState_Test_InvalidFileState(void)
{

    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST STATE command arg: file state = %%d");

    UT_CmdBuf.DestStateCmd.FileTableIndex = 1;
    UT_CmdBuf.DestStateCmd.EnableState    = 99;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestState_Test_InvalidFileState */

void DS_CmdSetDestState_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestStateCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_STATE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST STATE command: destination file table is not loaded");

    UT_CmdBuf.DestStateCmd.FileTableIndex = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_CmdSetDestState(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestState_Test_FileTableNotLoaded */

void DS_CmdSetDestPath_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestPathCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST PATH command: file table index = %%d, pathname = '%%s'");

    UT_CmdBuf.DestPathCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestPathCmd.Pathname, "pathname", OS_MAX_PATH_LEN);

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestPath(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestPathCmd.FileTableIndex].Pathname, "pathname",
                          OS_MAX_PATH_LEN) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestPathCmd.FileTableIndex].Pathname, "
                  "'pathname', OS_MAX_PATH_LEN) == "
                  "0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestPathCmd_t), "DS_DestPathCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestPath_Test_Nominal */

void DS_CmdSetDestPath_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestPathCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST PATH command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetDestPath(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestPath_Test_InvalidCommandLength */

void DS_CmdSetDestPath_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestPathCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST PATH command arg: file table index = %%d");

    UT_CmdBuf.DestPathCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdSetDestPath(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestPath_Test_InvalidFileTableIndex */

void DS_CmdSetDestPath_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestPathCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_PATH_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST PATH command: destination file table is not loaded");

    UT_CmdBuf.DestPathCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestPathCmd.Pathname, "pathname", OS_MAX_PATH_LEN);

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestPath(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestPath_Test_FileTableNotLoaded */

void DS_CmdSetDestBase_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST BASE command: file table index = %%d, base filename = '%%s'");

    UT_CmdBuf.DestBaseCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestBaseCmd.Basename, "base", OS_MAX_PATH_LEN);

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestBase(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestBaseCmd.FileTableIndex].Basename, "base",
                          OS_MAX_PATH_LEN) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestBaseCmd.FileTableIndex].Basename, 'base', "
                  "OS_MAX_PATH_LEN) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestBaseCmd_t), "DS_DestBaseCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestBase_Test_Nominal */

void DS_CmdSetDestBase_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST BASE command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetDestBase(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestBase_Test_InvalidCommandLength */

void DS_CmdSetDestBase_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST BASE command arg: file table index = %%d");

    UT_CmdBuf.DestBaseCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdSetDestBase(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestBase_Test_InvalidFileTableIndex */

void DS_CmdSetDestBase_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestBaseCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_BASE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST BASE command: destination file table is not loaded");

    UT_CmdBuf.DestBaseCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestBaseCmd.Basename, "base", sizeof(UT_CmdBuf.DestBaseCmd.Basename));

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestBase(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestBase_Test_FileTableNotLoaded */

void DS_CmdSetDestExt_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestExtCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST EXT command: file table index = %%d, extension = '%%s'");

    UT_CmdBuf.DestExtCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestExtCmd.Extension, "txt", DS_EXTENSION_BUFSIZE);

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestExt(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestExtCmd.FileTableIndex].Extension, "txt",
                          DS_EXTENSION_BUFSIZE) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestExtCmd.FileTableIndex].Extension, 'txt', "
                  "DS_EXTENSION_BUFSIZE) == "
                  "0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestExtCmd_t), "DS_DestExtCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestExt_Test_Nominal */

void DS_CmdSetDestExt_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestExtCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST EXT command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdSetDestExt(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestExt_Test_InvalidCommandLength */

void DS_CmdSetDestExt_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestExtCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST EXT command arg: file table index = %%d");

    UT_CmdBuf.DestExtCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdSetDestExt(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestExt_Test_InvalidFileTableIndex */

void DS_CmdSetDestExt_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestExtCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_EXT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST EXT command: destination file table is not loaded");

    UT_CmdBuf.DestExtCmd.FileTableIndex = 1;
    strncpy(UT_CmdBuf.DestExtCmd.Extension, "txt", DS_EXTENSION_BUFSIZE);

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestExt(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestExt_Test_FileTableNotLoaded */

void DS_CmdSetDestSize_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST SIZE command: file table index = %%d, size limit = %%d");

    UT_CmdBuf.DestSizeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestSizeCmd.MaxFileSize    = 100000000;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifySize), true);

    /* Execute the function being tested */
    DS_CmdSetDestSize(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestSizeCmd.FileTableIndex].MaxFileSize == 100000000,
                  "DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestSizeCmd.FileTableIndex].MaxFileSize == 100000000");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestSizeCmd_t), "DS_DestSizeCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestSize_Test_Nominal */

void DS_CmdSetDestSize_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST SIZE command length: expected = %%d, actual = %%d");

    UT_CmdBuf.DestSizeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestSizeCmd.MaxFileSize    = 100000000;

    /* Execute the function being tested */
    DS_CmdSetDestSize(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestSize_Test_InvalidCommandLength */

void DS_CmdSetDestSize_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST SIZE command arg: file table index = %%d");

    UT_CmdBuf.DestSizeCmd.FileTableIndex = 99;
    UT_CmdBuf.DestSizeCmd.MaxFileSize    = 100000000;

    /* Execute the function being tested */
    DS_CmdSetDestSize(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestSize_Test_InvalidFileTableIndex */

void DS_CmdSetDestSize_Test_InvalidFileSizeLimit(void)
{

    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST SIZE command arg: size limit = %%d");

    UT_CmdBuf.DestSizeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestSizeCmd.MaxFileSize    = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestSize(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestSize_Test_InvalidFileSizeLimit */

void DS_CmdSetDestSize_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestSizeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_SIZE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST SIZE command: destination file table is not loaded");

    UT_CmdBuf.DestSizeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestSizeCmd.MaxFileSize    = 100000000;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifySize), true);

    /* Execute the function being tested */
    DS_CmdSetDestSize(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestSize_Test_FileTableNotLoaded */

void DS_CmdSetDestAge_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST AGE command: file table index = %%d, age limit = %%d");

    UT_CmdBuf.DestAgeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestAgeCmd.MaxFileAge     = 1000;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyAge), true);

    /* Execute the function being tested */
    DS_CmdSetDestAge(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestAgeCmd.FileTableIndex].MaxFileAge == 1000,
                  "DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestAgeCmd.FileTableIndex].MaxFileAge == 1000");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestAgeCmd_t), "DS_DestAgeCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestAge_Test_Nominal */

void DS_CmdSetDestAge_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST AGE command length: expected = %%d, actual = %%d");

    UT_CmdBuf.DestAgeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestAgeCmd.MaxFileAge     = 1000;

    /* Execute the function being tested */
    DS_CmdSetDestAge(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestAge_Test_InvalidCommandLength */

void DS_CmdSetDestAge_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST AGE command arg: file table index = %%d");

    UT_CmdBuf.DestAgeCmd.FileTableIndex = 99;
    UT_CmdBuf.DestAgeCmd.MaxFileAge     = 1000;

    /* Execute the function being tested */
    DS_CmdSetDestAge(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestAge_Test_InvalidFileTableIndex */

void DS_CmdSetDestAge_Test_InvalidFileAgeLimit(void)
{

    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid DEST AGE command arg: age limit = %%d");

    UT_CmdBuf.DestAgeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestAgeCmd.MaxFileAge     = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestAge(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestAge_Test_InvalidFileAgeLimit */

void DS_CmdSetDestAge_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestAgeCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_AGE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST AGE command: destination file table is not loaded");

    UT_CmdBuf.DestAgeCmd.FileTableIndex = 1;
    UT_CmdBuf.DestAgeCmd.MaxFileAge     = 1000;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyAge), true);

    /* Execute the function being tested */
    DS_CmdSetDestAge(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestAge_Test_FileTableNotLoaded */

void DS_CmdSetDestCount_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "DEST COUNT command: file table index = %%d, sequence count = %%d");

    UT_CmdBuf.DestCountCmd.FileTableIndex = 1;
    UT_CmdBuf.DestCountCmd.SequenceCount  = 1;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyCount), true);

    /* Execute the function being tested */
    DS_CmdSetDestCount(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_True(DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestCountCmd.FileTableIndex].SequenceCount == 1,
                  "DS_AppData.DestFileTblPtr->File[UT_CmdBuf.DestCountCmd.FileTableIndex].SequenceCount == 1");

    UtAssert_True(DS_AppData.FileStatus[UT_CmdBuf.DestCountCmd.FileTableIndex].FileCount == 1,
                  "DS_AppData.FileStatus[UT_CmdBuf.DestCountCmd.FileTableIndex].FileCount == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestCountCmd_t), "DS_DestCountCmd_t is 32-bit aligned");
} /* end DS_CmdSetDestCount_Test_Nominal */

void DS_CmdSetDestCount_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_DestCountCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST COUNT command length: expected = %%d, actual = %%d");

    UT_CmdBuf.DestCountCmd.FileTableIndex = 1;
    UT_CmdBuf.DestCountCmd.SequenceCount  = 1;

    /* Execute the function being tested */
    DS_CmdSetDestCount(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestCount_Test_InvalidCommandLength */

void DS_CmdSetDestCount_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST COUNT command arg: file table index = %%d");

    UT_CmdBuf.DestCountCmd.FileTableIndex = 99;
    UT_CmdBuf.DestCountCmd.SequenceCount  = 1;

    /* Execute the function being tested */
    DS_CmdSetDestCount(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestCount_Test_InvalidFileTableIndex */

void DS_CmdSetDestCount_Test_InvalidFileSequenceCount(void)
{

    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST COUNT command arg: sequence count = %%d");

    UT_CmdBuf.DestCountCmd.FileTableIndex = 1;
    UT_CmdBuf.DestCountCmd.SequenceCount  = -1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdSetDestCount(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestCount_Test_InvalidFileSequenceCount */

void DS_CmdSetDestCount_Test_FileTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_DestCountCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_SET_DEST_COUNT_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST COUNT command: destination file table is not loaded");

    UT_CmdBuf.DestCountCmd.FileTableIndex = 1;
    UT_CmdBuf.DestCountCmd.SequenceCount  = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyCount), true);

    /* Execute the function being tested */
    DS_CmdSetDestCount(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdSetDestCount_Test_FileTableNotLoaded */

void DS_CmdCloseFile_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;
    uint32             i;
    uint8              call_count_DS_FileUpdateHeader = 0;
    uint8              call_count_DS_FileCloseDest    = 0;

    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "DEST CLOSE command: file table index = %%d");

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    UT_CmdBuf.CloseFileCmd.FileTableIndex = 0;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.FileStatus[UT_CmdBuf.CloseFileCmd.FileTableIndex].FileHandle = DS_UT_OBJID_1;

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdCloseFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_DS_FileUpdateHeader = UT_GetStubCount(UT_KEY(DS_FileUpdateHeader));
    UtAssert_INT32_EQ(call_count_DS_FileUpdateHeader, 1);

    call_count_DS_FileCloseDest = UT_GetStubCount(UT_KEY(DS_FileCloseDest));
    UtAssert_INT32_EQ(call_count_DS_FileCloseDest, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseFileCmd_t), "DS_CloseFileCmd_t is 32-bit aligned");
} /* end DS_CmdCloseFile_Test_Nominal */

void DS_CmdCloseFile_Test_NominalAlreadyClosed(void)
{
    DS_DestFileTable_t DestFileTable;
    uint32             i;
    uint8              call_count_DS_FileUpdateHeader = 0;
    uint8              call_count_DS_FileCloseDest    = 0;

    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "DEST CLOSE command: file table index = %%d");

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    UT_CmdBuf.CloseFileCmd.FileTableIndex = 0;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CmdCloseFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_DS_FileUpdateHeader = UT_GetStubCount(UT_KEY(DS_FileUpdateHeader));
    UtAssert_INT32_EQ(call_count_DS_FileUpdateHeader, 0);

    call_count_DS_FileCloseDest = UT_GetStubCount(UT_KEY(DS_FileCloseDest));
    UtAssert_INT32_EQ(call_count_DS_FileCloseDest, 0);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseFileCmd_t), "DS_CloseFileCmd_t is 32-bit aligned");
} /* end DS_CmdCloseFile_Test_NominalAlreadyClosed */

void DS_CmdCloseFile_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST CLOSE command length: expected = %%d, actual = %%d");

    UT_CmdBuf.CloseFileCmd.FileTableIndex = 0;

    /* Execute the function being tested */
    DS_CmdCloseFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdCloseFile_Test_InvalidCommandLength */

void DS_CmdCloseFile_Test_InvalidFileTableIndex(void)
{

    size_t            forced_Size    = sizeof(DS_CloseFileCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_FILE_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST CLOSE command arg: file table index = %%d");

    UT_CmdBuf.CloseFileCmd.FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CmdCloseFile(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdCloseFile_Test_InvalidFileTableIndex */

void DS_CmdCloseAll_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;
    uint32             i;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_ALL_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "DEST CLOSE ALL command");

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

#if (DS_MOVE_FILES == true)
    strncpy(DS_AppData.DestFileTblPtr->File[0].Movename, "", DS_PATHNAME_BUFSIZE);
#endif

    /* Execute the function being tested */
    DS_CmdCloseAll(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_ALL_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseAllCmd_t), "DS_CloseAllCmd_t is 32-bit aligned");
} /* end DS_CmdCloseAll_Test_Nominal */

void DS_CmdCloseAll_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_CloseAllCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_CLOSE_ALL_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid DEST CLOSE ALL command length: expected = %%d, actual = %%d");

    /* Execute the function being tested */
    DS_CmdCloseAll(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_ALL_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdCloseAll_Test_InvalidCommandLength */

void DS_CmdGetFileInfo_Test_EnabledOpen(void)
{
    DS_DestFileTable_t DestFileTable;
    uint32             i;

    size_t            forced_Size    = sizeof(DS_GetFileInfoCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_GET_FILE_INFO_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "GET FILE INFO command");

    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileAge    = 1;
        DS_AppData.FileStatus[i].FileSize   = 2;
        DS_AppData.FileStatus[i].FileRate   = 3;
        DS_AppData.FileStatus[i].FileCount  = 4;
        DS_AppData.FileStatus[i].FileState  = 5;
        DS_AppData.FileStatus[i].FileHandle = DS_UT_OBJID_1;
        strncpy(DS_AppData.FileStatus[i].FileName, "filename", OS_MAX_PATH_LEN);
    }

    /* Execute the function being tested */
    DS_CmdGetFileInfo(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");
    /* Changes to DS_FileInfoPkt cannot easily be verified because DS_FileInfoPkt is a local variable */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_GET_FILE_INFO_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_FileInfoPkt_t), "DS_FileInfoPkt_t is 32-bit aligned");

} /* end DS_CmdGetFileInfo_Test_EnabledOpen */

void DS_CmdGetFileInfo_Test_DisabledClosed(void)
{
    uint32 i;

    size_t            forced_Size    = sizeof(DS_GetFileInfoCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_GET_FILE_INFO_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "GET FILE INFO command");

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileAge    = 1;
        DS_AppData.FileStatus[i].FileSize   = 2;
        DS_AppData.FileStatus[i].FileRate   = 3;
        DS_AppData.FileStatus[i].FileCount  = 4;
        DS_AppData.FileStatus[i].FileState  = 5;
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
        strncpy(DS_AppData.FileStatus[i].FileName, "filename", OS_MAX_PATH_LEN);
    }

    /* Execute the function being tested */
    DS_CmdGetFileInfo(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_GET_FILE_INFO_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */

} /* end DS_CmdGetFileInfo_Test_DisabledClosed */

void DS_CmdGetFileInfo_Test_InvalidCommandLength(void)
{
    DS_DestFileTable_t DestFileTable;

    size_t            forced_Size    = sizeof(DS_GetFileInfoCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_GET_FILE_INFO_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid GET FILE INFO command length: expected = %%d, actual = %%d");

    DS_AppData.DestFileTblPtr = &DestFileTable;

    /* Execute the function being tested */
    DS_CmdGetFileInfo(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_GET_FILE_INFO_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdGetFileInfo_Test_InvalidCommandLength */

void DS_CmdAddMID_Test_Nominal(void)
{
    DS_FilterTable_t FilterTable;
    int32            FilterTableIndex;
    DS_HashLink_t    HashLink;

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ADD MID command: MID = 0x%%08lX, filter index = %%d, hash index = %%d");

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_AddMidCmd_t), "DS_AddMidCmd_t is 32-bit aligned");

    UT_CmdBuf.AddMidCmd.MessageID = DS_UT_MID_1;

    /* This is the hash done in DS_TableHashFunction */
    DS_AppData.FilterTblPtr = &FilterTable;

    DS_AppData.HashTable[0]                                   = &HashLink;
    HashLink.Index                                            = 0;
    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = CFE_SB_INVALID_MSG_ID;
    HashLink.Next                                             = (struct DS_HashTag *)&DS_AppData.HashTable[1];

    DS_AppData.HashTable[1] = &HashLink;
    HashLink.Index          = 1;
    HashLink.Next           = (struct DS_HashTag *)&DS_AppData.HashTable[2];

    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = DS_UT_MID_2;

    FilterTableIndex = 0;

    /* for nominal case, first call to DS_TableFindMsgID must return
     * DS_INDEX_NONE and the second call must return something other than
     * DS_INDEX_NONE */
    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, DS_INDEX_NONE);
    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, 0);

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdAcceptedCounter == 1, "DS_AppData.CmdAcceptedCounter == 1");

    UtAssert_BOOL_TRUE(CFE_SB_MsgId_Equal(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].MessageID, DS_UT_MID_1));

    /* Check first elements */
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FileTableIndex == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FileTableIndex == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FilterType == DS_BY_COUNT,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FilterType == DS_BY_COUNT");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_N == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_N == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_X == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_X == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_O == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_O == 0");

    /* Check middle elements */
    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FileTableIndex == 0,
        "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET/2].FileTableIndex == 0");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FilterType == DS_BY_COUNT,
        "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET/2].FilterType == DS_BY_COUNT");

    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_N == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET/2].Algorithm_N == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_X == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET/2].Algorithm_X == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_O == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET/2].Algorithm_O == 0");

    /* Check last elements */
    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FileTableIndex == 0,
        "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET-1].FileTableIndex == 0");

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FilterType == DS_BY_COUNT,
        "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET-1].FilterType == DS_BY_COUNT");

    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_N == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET-1].Algorithm_N == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_X == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET-1].Algorithm_X == 0");
    UtAssert_True(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_O == 0,
                  "DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET-1].Algorithm_O == 0");

    // UtAssert_True (DS_AppData.HashLinks[0].Index == 0, "DS_AppData.HashLinks[0].Index == 0");
    // UtAssert_True (DS_AppData.HashLinks[0].MessageID == DS_UT_MID_1, "DS_AppData.HashLinks[0].MessageID ==
    // DS_UT_MID_1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_Nominal */

void DS_CmdAddMID_Test_InvalidCommandLength(void)
{

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t) + 1;
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid ADD MID command length: expected = %%d, actual = %%d");

    UT_CmdBuf.AddMidCmd.MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_InvalidCommandLength */

void DS_CmdAddMID_Test_InvalidMessageID(void)
{

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid ADD MID command arg: invalid MID = 0x%%08lX");

    UT_CmdBuf.AddMidCmd.MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    /*   UtAssert_True
           (Ut_CFE_EVS_EventSent(DS_ADD_MID_CMD_ERR_EID, CFE_EVS_ERROR, "Invalid ADD MID command arg: invalid MID = 0x
       0"), "Invalid ADD MID command arg: invalid MID = 0x   0");
   */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_InvalidMessageID */

void DS_CmdAddMID_Test_FilterTableNotLoaded(void)
{

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid ADD MID command: filter table is not loaded");

    UT_CmdBuf.AddMidCmd.MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_FilterTableNotLoaded */

void DS_CmdAddMID_Test_MIDAlreadyInFilterTable(void)
{
    DS_FilterTable_t FilterTable;
    DS_HashLink_t    HashLink;

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid ADD MID command: MID = 0x%%08lX is already in filter table at index = %%d");

    UT_CmdBuf.AddMidCmd.MessageID = DS_UT_MID_1;

    DS_AppData.HashTable[187]                  = &HashLink;
    HashLink.Index                             = 0;
    DS_AppData.FilterTblPtr                    = &FilterTable;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, 1);

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_MIDAlreadyInFilterTable */

void DS_CmdAddMID_Test_FilterTableFull(void)
{
    DS_FilterTable_t FilterTable;

    size_t            forced_Size    = sizeof(DS_AddMidCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_ADD_MID_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Invalid ADD MID command: filter table is full");

    UT_CmdBuf.AddMidCmd.MessageID = DS_UT_MID_1;

    DS_AppData.FilterTblPtr = &FilterTable;

    /* both calls to DS_TableFindMsgID must return DS_INDEX_NONE */
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_CmdAddMID(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(DS_AppData.CmdRejectedCounter == 1, "DS_AppData.CmdRejectedCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

} /* end DS_CmdAddMID_Test_FilterTableFull */

void UtTest_Setup(void)
{
    UtTest_Add(DS_CmdNoop_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdNoop_Test_Nominal");
    UtTest_Add(DS_CmdNoop_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdNoop_Test_InvalidCommandLength");

    UtTest_Add(DS_CmdReset_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdReset_Test_Nominal");
    UtTest_Add(DS_CmdReset_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdReset_Test_InvalidCommandLength");

    UtTest_Add(DS_CmdSetAppState_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetAppState_Test_Nominal");
    UtTest_Add(DS_CmdSetAppState_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetAppState_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetAppState_Test_InvalidAppState, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetAppState_Test_InvalidAppState");

    UtTest_Add(DS_CmdSetFilterFile_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetFilterFile_Test_Nominal");
    UtTest_Add(DS_CmdSetFilterFile_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetFilterFile_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_InvalidMessageID");
    UtTest_Add(DS_CmdSetFilterFile_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_CmdSetFilterFile_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetFilterFile_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_FilterTableNotLoaded");
    UtTest_Add(DS_CmdSetFilterFile_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterFile_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_CmdSetFilterType_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetFilterType_Test_Nominal");
    UtTest_Add(DS_CmdSetFilterType_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetFilterType_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_InvalidMessageID");
    UtTest_Add(DS_CmdSetFilterType_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_CmdSetFilterType_Test_InvalidFilterType, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_InvalidFilterType");
    UtTest_Add(DS_CmdSetFilterType_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_FilterTableNotLoaded");
    UtTest_Add(DS_CmdSetFilterType_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterType_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_CmdSetFilterParms_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetFilterParms_Test_Nominal");
    UtTest_Add(DS_CmdSetFilterParms_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetFilterParms_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_InvalidMessageID");
    UtTest_Add(DS_CmdSetFilterParms_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_CmdSetFilterParms_Test_InvalidFilterAlgorithm, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_InvalidFilterAlgorithm");
    UtTest_Add(DS_CmdSetFilterParms_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_FilterTableNotLoaded");
    UtTest_Add(DS_CmdSetFilterParms_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetFilterParms_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_CmdSetDestType_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestType_Test_Nominal");
    UtTest_Add(DS_CmdSetDestType_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestType_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestType_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestType_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestType_Test_InvalidFilenameType, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestType_Test_InvalidFilenameType");
    UtTest_Add(DS_CmdSetDestType_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestType_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestState_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestState_Test_Nominal");
    UtTest_Add(DS_CmdSetDestState_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestState_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestState_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestState_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestState_Test_InvalidFileState, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestState_Test_InvalidFileState");
    UtTest_Add(DS_CmdSetDestState_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestState_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestPath_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestPath_Test_Nominal");
    UtTest_Add(DS_CmdSetDestPath_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestPath_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestPath_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestPath_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestPath_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestPath_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestBase_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestBase_Test_Nominal");
    UtTest_Add(DS_CmdSetDestBase_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestBase_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestBase_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestBase_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestBase_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestBase_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestExt_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestExt_Test_Nominal");
    UtTest_Add(DS_CmdSetDestExt_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestExt_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestExt_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestExt_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestExt_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestExt_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestSize_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestSize_Test_Nominal");
    UtTest_Add(DS_CmdSetDestSize_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestSize_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestSize_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestSize_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestSize_Test_InvalidFileSizeLimit, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestSize_Test_InvalidFileSizeLimit");
    UtTest_Add(DS_CmdSetDestSize_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestSize_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestAge_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestAge_Test_Nominal");
    UtTest_Add(DS_CmdSetDestAge_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestAge_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestAge_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestAge_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestAge_Test_InvalidFileAgeLimit, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestAge_Test_InvalidFileAgeLimit");
    UtTest_Add(DS_CmdSetDestAge_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestAge_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdSetDestCount_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdSetDestCount_Test_Nominal");
    UtTest_Add(DS_CmdSetDestCount_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestCount_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdSetDestCount_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestCount_Test_InvalidFileTableIndex");
    UtTest_Add(DS_CmdSetDestCount_Test_InvalidFileSequenceCount, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestCount_Test_InvalidFileSequenceCount");
    UtTest_Add(DS_CmdSetDestCount_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdSetDestCount_Test_FileTableNotLoaded");

    UtTest_Add(DS_CmdCloseFile_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdCloseFile_Test_Nominal");
    UtTest_Add(DS_CmdCloseFile_Test_NominalAlreadyClosed, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdCloseFile_Test_NominalAlreadyClosed");
    UtTest_Add(DS_CmdCloseFile_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdCloseFile_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdCloseFile_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdCloseFile_Test_InvalidFileTableIndex");

    UtTest_Add(DS_CmdCloseAll_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdCloseAll_Test_Nominal");
    UtTest_Add(DS_CmdCloseAll_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdCloseAll_Test_InvalidCommandLength");

    UtTest_Add(DS_CmdGetFileInfo_Test_EnabledOpen, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdGetFileInfo_Test_EnabledOpen");
    UtTest_Add(DS_CmdGetFileInfo_Test_DisabledClosed, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdGetFileInfo_Test_DisabledClosed");
    UtTest_Add(DS_CmdGetFileInfo_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdGetFileInfo_Test_InvalidCommandLength");

    UtTest_Add(DS_CmdAddMID_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CmdAddMID_Test_Nominal");
    UtTest_Add(DS_CmdAddMID_Test_InvalidCommandLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdAddMID_Test_InvalidCommandLength");
    UtTest_Add(DS_CmdAddMID_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdAddMID_Test_InvalidMessageID");
    UtTest_Add(DS_CmdAddMID_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdAddMID_Test_FilterTableNotLoaded");
    UtTest_Add(DS_CmdAddMID_Test_MIDAlreadyInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_CmdAddMID_Test_MIDAlreadyInFilterTable");
    UtTest_Add(DS_CmdAddMID_Test_FilterTableFull, DS_Test_Setup, DS_Test_TearDown, "DS_CmdAddMID_Test_FilterTableFull");
} /* end DS_Cmds_Test_AddTestCases */

/************************/
/*  End of File Comment */
/************************/
