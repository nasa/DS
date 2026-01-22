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

#include "ds_dispatch.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_eventids.h"
#include "ds_test_utils.h"
#include "ds_cmds.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

static void DS_Dispatch_Test_SetupMsg(CFE_SB_MsgId_t MsgId, CFE_MSG_FcnCode_t FcnCode, size_t MsgSize)
{
    /* Note some paths get the MsgId/FcnCode multiple times, so register accordingly, just in case */
    CFE_SB_MsgId_t    RegMsgId[2]   = {MsgId, MsgId};
    CFE_MSG_FcnCode_t RegFcnCode[2] = {FcnCode, FcnCode};
    size_t            RegMsgSize[2] = {MsgSize, MsgSize};

    UT_ResetState(UT_KEY(CFE_MSG_GetMsgId));
    UT_ResetState(UT_KEY(CFE_MSG_GetFcnCode));
    UT_ResetState(UT_KEY(CFE_MSG_GetSize));

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), RegMsgId, sizeof(RegMsgId), true);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), RegFcnCode, sizeof(RegFcnCode), true);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), RegMsgSize, sizeof(RegMsgSize), true);
}

void DS_AppPipe_Test_CmdStore(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_NOOP_CC, sizeof(DS_NoopCmd_t));

    DS_AppData.AppEnableState                  = DS_DISABLED;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    /* an attempt was made to store this packet */
    UtAssert_STUB_COUNT(DS_AppStorePacket, 1);
}

void DS_AppPipe_Test_CmdNoStore(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_NOOP_CC, sizeof(DS_NoopCmd_t));

    DS_AppData.AppEnableState                  = DS_DISABLED;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* no attempt was made to store this packet */
    UtAssert_STUB_COUNT(DS_AppStorePacket, 0);
}

void DS_AppPipe_Test_HKStore(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_SEND_HK_MID), 0, sizeof(DS_SendHkCmd_t));

    DS_AppData.AppEnableState                  = DS_DISABLED;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    UtAssert_STUB_COUNT(DS_SendHkCmd, 1);

    /* an attempt was made to store this packet */
    UtAssert_STUB_COUNT(DS_AppStorePacket, 1);
}

void DS_AppPipe_Test_HKNoStore(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_SEND_HK_MID), 0, sizeof(DS_SendHkCmd_t));

    DS_AppData.AppEnableState                  = DS_DISABLED;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    UtAssert_STUB_COUNT(DS_SendHkCmd, 1);

    /* an attempt was made to store this packet */
    UtAssert_STUB_COUNT(DS_AppStorePacket, 0);
}

void DS_AppPipe_Test_HKInvalidRequest(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_SEND_HK_MID), 0, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_HKREQ_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AppPipe_Test_UnknownMID(void)
{
    DS_Dispatch_Test_SetupMsg(DS_UT_MID_1, 0, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppPipe(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* an attempt was made to store this packet */
    UtAssert_STUB_COUNT(DS_AppStorePacket, 1);
}

void DS_AppProcessCmd_Test_Noop(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_NOOP_CC, sizeof(DS_NoopCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_NoopCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_NOOP_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_NoopCmd, 1);
}

void DS_AppProcessCmd_Test_Reset(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_RESET_COUNTERS_CC, sizeof(DS_ResetCountersCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_ResetCountersCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_RESET_COUNTERS_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_ResetCountersCmd, 1);
}

void DS_AppProcessCmd_Test_SetAppState(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_APP_STATE_CC, sizeof(DS_SetAppStateCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetAppStateCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_APP_STATE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetAppStateCmd, 1);
}

void DS_AppProcessCmd_Test_SetFilterFile(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_FILE_CC, sizeof(DS_SetFilterFileCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetFilterFileCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_FILE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetFilterFileCmd, 1);
}

void DS_AppProcessCmd_Test_SetFilterType(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_TYPE_CC, sizeof(DS_SetFilterTypeCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetFilterTypeCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_TYPE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetFilterTypeCmd, 1);
}

void DS_AppProcessCmd_Test_SetFilterParms(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_PARMS_CC, sizeof(DS_SetFilterParmsCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetFilterParmsCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_FILTER_PARMS_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetFilterParmsCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestType(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_TYPE_CC, sizeof(DS_SetDestTypeCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestTypeCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_TYPE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestTypeCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestState(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_STATE_CC, sizeof(DS_SetDestStateCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestStateCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_STATE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestStateCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestPath(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_PATH_CC, sizeof(DS_SetDestPathCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestPathCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_PATH_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestPathCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestBase(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_BASE_CC, sizeof(DS_SetDestBaseCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestBaseCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_BASE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestBaseCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestExt(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_EXT_CC, sizeof(DS_SetDestExtCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestExtCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_EXT_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestExtCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestSize(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_SIZE_CC, sizeof(DS_SetDestSizeCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestSizeCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_SIZE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestSizeCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestAge(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_AGE_CC, sizeof(DS_SetDestAgeCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestAgeCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_AGE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestAgeCmd, 1);
}

void DS_AppProcessCmd_Test_SetDestCount(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_COUNT_CC, sizeof(DS_SetDestCountCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_SetDestCountCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_SET_DEST_COUNT_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_SetDestCountCmd, 1);
}

void DS_AppProcessCmd_Test_CloseFile(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_CLOSE_FILE_CC, sizeof(DS_CloseFileCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CloseFileCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_CLOSE_FILE_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_CloseFileCmd, 1);
}

void DS_AppProcessCmd_Test_GetFileInfo(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_GET_FILE_INFO_CC, sizeof(DS_GetFileInfoCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_GetFileInfoCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_GET_FILE_INFO_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_GetFileInfoCmd, 1);
}

void DS_AppProcessCmd_Test_AddMID(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_ADD_MID_CC, sizeof(DS_AddMidCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_AddMidCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_ADD_MID_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_AddMidCmd, 1);
}

void DS_AppProcessCmd_Test_RemoveMID(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_REMOVE_MID_CC, sizeof(DS_RemoveMidCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_RemoveMidCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_REMOVE_MID_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_RemoveMidCmd, 1);
}

void DS_AppProcessCmd_Test_CloseAll(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_CLOSE_ALL_CC, sizeof(DS_CloseAllCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(DS_CloseAllCmd, 1);

    /* Now with an invalid size */
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_CLOSE_ALL_CC, 1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(DS_CloseAllCmd, 1);
}

void DS_AppProcessCmd_Test_InvalidCommandCode(void)
{
    DS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(DS_CMD_MID), 99, sizeof(DS_CloseAllCmd_t));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_AppProcessCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void UtTest_Setup(void)
{
    UT_DS_TEST_ADD(DS_AppPipe_Test_CmdStore);
    UT_DS_TEST_ADD(DS_AppPipe_Test_CmdNoStore);
    UT_DS_TEST_ADD(DS_AppPipe_Test_HKStore);
    UT_DS_TEST_ADD(DS_AppPipe_Test_HKNoStore);
    UT_DS_TEST_ADD(DS_AppPipe_Test_HKInvalidRequest);
    UT_DS_TEST_ADD(DS_AppPipe_Test_UnknownMID);

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
}
