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

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

#define CMD_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_CommandHeader_t)) % 4) == 0
#define TLM_STRUCT_DATA_IS_32_ALIGNED(x) ((sizeof(x) - sizeof(CFE_MSG_TelemetryHeader_t)) % 4) == 0

/*
 * Function Definitions
 */

void DS_NoopCmd_Test_Nominal(void)
{
    /* Execute the function being tested */
    DS_NoopCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NOOP_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_NoopCmd_t), "DS_NoopCmd_t is 32-bit aligned");
}

void DS_ResetCountersCmd_Test_Nominal(void)
{
    /* Execute the function being tested */
    DS_ResetCountersCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_ZERO(DS_AppData.CmdAcceptedCounter);
    UtAssert_ZERO(DS_AppData.CmdRejectedCounter);
    UtAssert_ZERO(DS_AppData.DisabledPktCounter);
    UtAssert_ZERO(DS_AppData.IgnoredPktCounter);
    UtAssert_ZERO(DS_AppData.FilteredPktCounter);
    UtAssert_ZERO(DS_AppData.PassedPktCounter);
    UtAssert_ZERO(DS_AppData.FileWriteCounter);
    UtAssert_ZERO(DS_AppData.FileWriteErrCounter);
    UtAssert_ZERO(DS_AppData.FileUpdateCounter);
    UtAssert_ZERO(DS_AppData.FileUpdateErrCounter);
    UtAssert_ZERO(DS_AppData.DestTblLoadCounter);
    UtAssert_ZERO(DS_AppData.DestTblErrCounter);
    UtAssert_ZERO(DS_AppData.FilterTblLoadCounter);
    UtAssert_ZERO(DS_AppData.FilterTblErrCounter);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_RESET_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_ResetCountersCmd_t), "DS_ResetCountersCmd_t is 32-bit aligned");
}

void DS_SetAppStateCmd_Test_Nominal(void)
{
    DS_AppState_Payload_t *CmdPayload = &UT_CmdBuf.AppStateCmd.Payload;

    CmdPayload->EnableState = true;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_SetAppStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.CmdRejectedCounter, 0);
    UtAssert_INT32_EQ(DS_AppData.CmdAcceptedCounter, 1);
    UtAssert_True(DS_AppData.AppEnableState == true, "DS_AppData.AppEnableState == true");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ENADIS_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_AppStateCmd_t), "DS_AppStateCmd_t is 32-bit aligned");
}

void DS_SetAppStateCmd_Test_InvalidAppState(void)
{
    DS_AppState_Payload_t *CmdPayload = &UT_CmdBuf.AppStateCmd.Payload;

    CmdPayload->EnableState = 99;

    /* Execute the function being tested */
    DS_SetAppStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ENADIS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterFileCmd_Test_Nominal(void)
{
    int32                    forced_FilterTableIndex = 1;
    DS_FilterFile_Payload_t *CmdPayload              = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FileTableIndex   = 4;

    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex].Filter[CmdPayload->FilterParmsIndex].FileTableIndex = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), forced_FilterTableIndex);

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_True(
        DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex].Filter[CmdPayload->FilterParmsIndex].FileTableIndex ==
            CmdPayload->FileTableIndex,
        "DS_AppData.FilterTblPtr->Packet[forced_FilterTableIndex].Filter[CmdPayload->"
        "FilterParmsIndex].FileTableIndex == CmdPayload->FileTableIndex");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterFileCmd_t), "DS_FilterFileCmd_t is 32-bit aligned");
}

void DS_SetFilterFileCmd_Test_InvalidMessageID(void)
{
    DS_FilterFile_Payload_t *CmdPayload = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterFileCmd_Test_InvalidFilterParametersIndex(void)
{
    DS_FilterFile_Payload_t *CmdPayload = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterFileCmd_Test_InvalidFileTableIndex(void)
{
    DS_FilterFile_Payload_t *CmdPayload = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = 1;
    CmdPayload->FileTableIndex   = 99;

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterFileCmd_Test_FilterTableNotLoaded(void)
{
    DS_FilterFile_Payload_t *CmdPayload = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = 1;
    CmdPayload->FileTableIndex   = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Reset table pointer to NULL (set in test setup) */
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterFileCmd_Test_MessageIDNotInFilterTable(void)
{
    DS_FilterFile_Payload_t *CmdPayload = &UT_CmdBuf.FilterFileCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_2;
    CmdPayload->FileTableIndex   = 4;

    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_SetFilterFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterTypeCmd_Test_Nominal(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterType       = 1;

    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.FilterTblPtr->Packet[0].Filter[CmdPayload->FilterParmsIndex].FilterType, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterTypeCmd_t), "DS_FilterTypeCmd_t is 32-bit aligned");
}

void DS_SetFilterTypeCmd_Test_InvalidMessageID(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterTypeCmd_Test_InvalidFilterParametersIndex(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterTypeCmd_Test_InvalidFilterType(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = 1;
    CmdPayload->FilterType       = false;

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterTypeCmd_Test_FilterTableNotLoaded(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = 1;
    CmdPayload->FilterType       = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Reset table pointer to NULL (set in test setup) */
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterTypeCmd_Test_MessageIDNotInFilterTable(void)
{
    DS_FilterType_Payload_t *CmdPayload = &UT_CmdBuf.FilterTypeCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_2;
    CmdPayload->FilterParmsIndex = 1;
    CmdPayload->FilterType       = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_SetFilterTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterParmsCmd_Test_Nominal(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->Algorithm_N      = 0;
    CmdPayload->Algorithm_X      = 0;
    CmdPayload->Algorithm_O      = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[0].Filter[CmdPayload->FilterParmsIndex].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[0].Filter[CmdPayload->FilterParmsIndex].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[0].Filter[CmdPayload->FilterParmsIndex].Algorithm_O);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_FilterParmsCmd_t), "DS_FilterParmsCmd_t is 32-bit aligned");
}

void DS_SetFilterParmsCmd_Test_InvalidMessageID(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterParmsCmd_Test_InvalidFilterParametersIndex(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = DS_FILTERS_PER_PACKET;

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterParmsCmd_Test_InvalidFilterAlgorithm(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->Algorithm_N      = 1;
    CmdPayload->Algorithm_X      = 1;
    CmdPayload->Algorithm_O      = 1;

    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterParmsCmd_Test_FilterTableNotLoaded(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->MessageID        = DS_UT_MID_1;
    CmdPayload->FilterParmsIndex = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);

    /* Reset table pointer to NULL (set in test setup) */
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetFilterParmsCmd_Test_MessageIDNotInFilterTable(void)
{
    DS_FilterParms_Payload_t *CmdPayload = &UT_CmdBuf.FilterParmsCmd.Payload;

    CmdPayload->FilterParmsIndex = 2;
    CmdPayload->MessageID        = DS_UT_MID_2;

    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyParms), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_SetFilterParmsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PARMS_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestTypeCmd_Test_Nominal(void)
{
    DS_DestType_Payload_t *CmdPayload = &UT_CmdBuf.DestTypeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->FileNameType   = 2;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_SetDestTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].FileNameType, 2);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestTypeCmd_t), "DS_DestTypeCmd_t is 32-bit aligned");
}

void DS_SetDestTypeCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestType_Payload_t *CmdPayload = &UT_CmdBuf.DestTypeCmd.Payload;

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_SetDestTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestTypeCmd_Test_InvalidFilenameType(void)
{
    DS_DestType_Payload_t *CmdPayload = &UT_CmdBuf.DestTypeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->FileNameType   = 99;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestTypeCmd_Test_FileTableNotLoaded(void)
{
    DS_DestType_Payload_t *CmdPayload = &UT_CmdBuf.DestTypeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->FileNameType   = 2;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyType), true);

    /* Execute the function being tested */
    DS_SetDestTypeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_NTYPE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestStateCmd_Test_Nominal(void)
{
    DS_DestState_Payload_t *CmdPayload = &UT_CmdBuf.DestStateCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->EnableState    = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_SetDestStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_True(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].EnableState == CmdPayload->EnableState,
                  "DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].EnableState == "
                  "CmdPayload->EnableState");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestStateCmd_t), "DS_DestStateCmd_t is 32-bit aligned");
}

void DS_SetDestStateCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestState_Payload_t *CmdPayload = &UT_CmdBuf.DestStateCmd.Payload;

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_SetDestStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestStateCmd_Test_InvalidFileState(void)
{
    DS_DestState_Payload_t *CmdPayload = &UT_CmdBuf.DestStateCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->EnableState    = 99;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestStateCmd_Test_FileTableNotLoaded(void)
{
    DS_DestState_Payload_t *CmdPayload = &UT_CmdBuf.DestStateCmd.Payload;

    CmdPayload->FileTableIndex = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyState), true);

    /* Execute the function being tested */
    DS_SetDestStateCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_STATE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestPathCmd_Test_Nominal(void)
{
    DS_DestPath_Payload_t *CmdPayload = &UT_CmdBuf.DestPathCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Pathname, "pathname", sizeof(CmdPayload->Pathname) - 1);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestPathCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Pathname, "pathname",
                          sizeof(DS_AppData.DestFileTblPtr->File[0].Pathname)) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Pathname, "
                  "'pathname', sizeof(DestFileTable.File[0].Pathname) - 1) == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestPathCmd_t), "DS_DestPathCmd_t is 32-bit aligned");
}

void DS_SetDestPathCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestPath_Payload_t *CmdPayload = &UT_CmdBuf.DestPathCmd.Payload;

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_SetDestPathCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestPathCmd_Test_FileTableNotLoaded(void)
{
    DS_DestPath_Payload_t *CmdPayload = &UT_CmdBuf.DestPathCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Pathname, "pathname", sizeof(CmdPayload->Pathname) - 1);

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestPathCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_PATH_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestBaseCmd_Test_Nominal(void)
{
    DS_DestBase_Payload_t *CmdPayload = &UT_CmdBuf.DestBaseCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Basename, "base", sizeof(CmdPayload->Basename) - 1);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestBaseCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Basename, "base",
                          sizeof(DS_AppData.DestFileTblPtr->File[0].Basename)) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Basename, 'base', "
                  "sizeof(DestFileTable.File[0].Basename)) == 0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestBaseCmd_t), "DS_DestBaseCmd_t is 32-bit aligned");
}

void DS_SetDestBaseCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestBase_Payload_t *CmdPayload = &UT_CmdBuf.DestBaseCmd.Payload;

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_SetDestBaseCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestBaseCmd_Test_FileTableNotLoaded(void)
{
    DS_DestBase_Payload_t *CmdPayload = &UT_CmdBuf.DestBaseCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Basename, "base", sizeof(CmdPayload->Basename));

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestBaseCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_BASE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestExtCmd_Test_Nominal(void)
{
    DS_DestExt_Payload_t *CmdPayload = &UT_CmdBuf.DestExtCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Extension, "txt", DS_EXTENSION_BUFSIZE);

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestExtCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_True(strncmp(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Extension, "txt",
                          DS_EXTENSION_BUFSIZE) == 0,
                  "strncmp (DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].Extension, 'txt', "
                  "DS_EXTENSION_BUFSIZE) == "
                  "0");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestExtCmd_t), "DS_DestExtCmd_t is 32-bit aligned");
}

void DS_SetDestExtCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestExt_Payload_t *CmdPayload = &UT_CmdBuf.DestExtCmd.Payload;

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_SetDestExtCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestExtCmd_Test_FileTableNotLoaded(void)
{
    DS_DestExt_Payload_t *CmdPayload = &UT_CmdBuf.DestExtCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    strncpy(CmdPayload->Extension, "txt", DS_EXTENSION_BUFSIZE);

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestExtCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_EXT_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestSizeCmd_Test_Nominal(void)
{
    DS_DestSize_Payload_t *CmdPayload = &UT_CmdBuf.DestSizeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileSize    = 100000000;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifySize), true);

    /* Execute the function being tested */
    DS_SetDestSizeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].MaxFileSize, 100000000);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestSizeCmd_t), "DS_DestSizeCmd_t is 32-bit aligned");
}

void DS_SetDestSizeCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestSize_Payload_t *CmdPayload = &UT_CmdBuf.DestSizeCmd.Payload;

    CmdPayload->FileTableIndex = 99;
    CmdPayload->MaxFileSize    = 100000000;

    /* Execute the function being tested */
    DS_SetDestSizeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestSizeCmd_Test_InvalidFileSizeLimit(void)
{
    DS_DestSize_Payload_t *CmdPayload = &UT_CmdBuf.DestSizeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileSize    = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestSizeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestSizeCmd_Test_FileTableNotLoaded(void)
{
    DS_DestSize_Payload_t *CmdPayload = &UT_CmdBuf.DestSizeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileSize    = 100000000;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifySize), true);

    /* Execute the function being tested */
    DS_SetDestSizeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SIZE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestAgeCmd_Test_Nominal(void)
{
    DS_DestAge_Payload_t *CmdPayload = &UT_CmdBuf.DestAgeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileAge     = 1000;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyAge), true);

    /* Execute the function being tested */
    DS_SetDestAgeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].MaxFileAge, 1000);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestAgeCmd_t), "DS_DestAgeCmd_t is 32-bit aligned");
}

void DS_SetDestAgeCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestAge_Payload_t *CmdPayload = &UT_CmdBuf.DestAgeCmd.Payload;

    CmdPayload->FileTableIndex = 99;
    CmdPayload->MaxFileAge     = 1000;

    /* Execute the function being tested */
    DS_SetDestAgeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestAgeCmd_Test_InvalidFileAgeLimit(void)
{
    DS_DestAge_Payload_t *CmdPayload = &UT_CmdBuf.DestAgeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileAge     = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestAgeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestAgeCmd_Test_FileTableNotLoaded(void)
{
    DS_DestAge_Payload_t *CmdPayload = &UT_CmdBuf.DestAgeCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->MaxFileAge     = 1000;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyAge), true);

    /* Execute the function being tested */
    DS_SetDestAgeCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_AGE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestCountCmd_Test_Nominal(void)
{
    DS_DestCount_Payload_t *CmdPayload = &UT_CmdBuf.DestCountCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->SequenceCount  = 1;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyCount), true);

    /* Execute the function being tested */
    DS_SetDestCountCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.DestFileTblPtr->File[CmdPayload->FileTableIndex].SequenceCount, 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[CmdPayload->FileTableIndex].FileCount, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_DestCountCmd_t), "DS_DestCountCmd_t is 32-bit aligned");
}

void DS_SetDestCountCmd_Test_InvalidFileTableIndex(void)
{
    DS_DestCount_Payload_t *CmdPayload = &UT_CmdBuf.DestCountCmd.Payload;

    CmdPayload->FileTableIndex = 99;
    CmdPayload->SequenceCount  = 1;

    /* Execute the function being tested */
    DS_SetDestCountCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestCountCmd_Test_InvalidFileSequenceCount(void)
{
    DS_DestCount_Payload_t *CmdPayload = &UT_CmdBuf.DestCountCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->SequenceCount  = -1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_SetDestCountCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_SetDestCountCmd_Test_FileTableNotLoaded(void)
{
    DS_DestCount_Payload_t *CmdPayload = &UT_CmdBuf.DestCountCmd.Payload;

    CmdPayload->FileTableIndex = 1;
    CmdPayload->SequenceCount  = 1;

    DS_AppData.DestFileTblPtr = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyCount), true);

    /* Execute the function being tested */
    DS_SetDestCountCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_SEQ_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_CloseFileCmd_Test_Nominal(void)
{
    uint32                  i;
    DS_CloseFile_Payload_t *CmdPayload = &UT_CmdBuf.CloseFileCmd.Payload;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    CmdPayload->FileTableIndex = 0;

    DS_AppData.FileStatus[CmdPayload->FileTableIndex].FileHandle = DS_UT_OBJID_1;

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CloseFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_STUB_COUNT(DS_FileUpdateHeader, 1);
    UtAssert_STUB_COUNT(DS_FileCloseDest, 1);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseFileCmd_t), "DS_CloseFileCmd_t is 32-bit aligned");
}

void DS_CloseFileCmd_Test_NominalAlreadyClosed(void)
{
    uint32                  i;
    DS_CloseFile_Payload_t *CmdPayload = &UT_CmdBuf.CloseFileCmd.Payload;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    CmdPayload->FileTableIndex = 0;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), true);

    /* Execute the function being tested */
    DS_CloseFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_STUB_COUNT(DS_FileUpdateHeader, 0);
    UtAssert_STUB_COUNT(DS_FileCloseDest, 0);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseFileCmd_t), "DS_CloseFileCmd_t is 32-bit aligned");
}

void DS_CloseFileCmd_Test_InvalidFileTableIndex(void)
{
    DS_CloseFile_Payload_t *CmdPayload = &UT_CmdBuf.CloseFileCmd.Payload;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableVerifyFileIndex), false);

    CmdPayload->FileTableIndex = 99;

    /* Execute the function being tested */
    DS_CloseFileCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_CloseAllCmd_Test_Nominal(void)
{
    uint32 i;

    for (i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

#if (DS_MOVE_FILES == true)
    strncpy(DS_AppData.DestFileTblPtr->File[0].Movename, "", DS_PATHNAME_BUFSIZE);
#endif

    /* Execute the function being tested */
    DS_CloseAllCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_ALL_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseAllCmd_t), "DS_CloseAllCmd_t is 32-bit aligned");
}

void DS_CloseAllCmd_Test_CloseAll(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = DS_UT_OBJID_1;
    }

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_CloseAllCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.CmdAcceptedCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(DS_FileUpdateHeader, DS_DEST_FILE_CNT);
    UtAssert_STUB_COUNT(DS_FileCloseDest, DS_DEST_FILE_CNT);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CLOSE_ALL_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_BOOL_TRUE(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_CloseAllCmd_t));
}

void DS_GetFileInfoCmd_Test_EnabledOpen(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileAge    = 1;
        DS_AppData.FileStatus[i].FileSize   = 2;
        DS_AppData.FileStatus[i].FileRate   = 3;
        DS_AppData.FileStatus[i].FileCount  = 4;
        DS_AppData.FileStatus[i].FileState  = 5;
        DS_AppData.FileStatus[i].FileHandle = DS_UT_OBJID_1;
        strncpy(DS_AppData.FileStatus[i].FileName, "filename", sizeof(DS_AppData.FileStatus[i].FileName) - 1);
    }

    /* Execute the function being tested */
    DS_GetFileInfoCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);
    /* Changes to DS_FileInfoPkt cannot easily be verified because DS_FileInfoPkt is a local variable */

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_GET_FILE_INFO_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(TLM_STRUCT_DATA_IS_32_ALIGNED(DS_FileInfoPkt_t), "DS_FileInfoPkt_t is 32-bit aligned");
}

void DS_GetFileInfoCmd_Test_DisabledClosed(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileAge    = 1;
        DS_AppData.FileStatus[i].FileSize   = 2;
        DS_AppData.FileStatus[i].FileRate   = 3;
        DS_AppData.FileStatus[i].FileCount  = 4;
        DS_AppData.FileStatus[i].FileState  = 5;
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
        strncpy(DS_AppData.FileStatus[i].FileName, "filename", sizeof(DS_AppData.FileStatus[i].FileName) - 1);
    }

    /* Also hits table NULL branch */
    DS_AppData.DestFileTblPtr = NULL;

    /* Execute the function being tested */
    DS_GetFileInfoCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_GET_FILE_INFO_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */
}

void DS_AddMIDCmd_Test_Nominal(void)
{
    int32 FilterTableIndex;

    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.AddMidCmd.Payload;

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_AddMidCmd_t), "DS_AddMidCmd_t is 32-bit aligned");

    CmdPayload->MessageID = DS_UT_MID_1;

    DS_AppData.FilterTblPtr->Packet[0].MessageID = CFE_SB_INVALID_MSG_ID;
    DS_AppData.FilterTblPtr->Packet[1].MessageID = DS_UT_MID_2;

    FilterTableIndex = 0;

    /* for nominal case, first call to DS_TableFindMsgID must return
     * DS_INDEX_NONE and the second call must return something other than
     * DS_INDEX_NONE */
    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, DS_INDEX_NONE);
    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, 0);

    /* Execute the function being tested */
    DS_AddMIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdAcceptedCounter, 1);

    UtAssert_BOOL_TRUE(CFE_SB_MsgId_Equal(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].MessageID, DS_UT_MID_1));

    /* Check first elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FilterType, DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_O);

    /* Check middle elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FilterType,
                      DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_O);

    /* Check last elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FilterType,
                      DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_O);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_AddMIDCmd_Test_InvalidMessageID(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.AddMidCmd.Payload;

    CmdPayload->MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    DS_AddMIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AddMIDCmd_Test_FilterTableNotLoaded(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.AddMidCmd.Payload;

    CmdPayload->MessageID = DS_UT_MID_1;

    /* Reset table pointer to NULL (set in test setup) */
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_AddMIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AddMIDCmd_Test_MIDAlreadyInFilterTable(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.AddMidCmd.Payload;

    CmdPayload->MessageID                      = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet->MessageID = DS_UT_MID_1;

    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, 1);

    /* Execute the function being tested */
    DS_AddMIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_AddMIDCmd_Test_FilterTableFull(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.AddMidCmd.Payload;

    CmdPayload->MessageID = DS_UT_MID_1;

    /* both calls to DS_TableFindMsgID must return DS_INDEX_NONE */
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_AddMIDCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_ADD_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_RemoveMIDCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t             MessageID        = DS_UT_MID_1;
    int32                      FilterTableIndex = 0;
    int32                      HashTableIndex   = 1;
    DS_AddRemoveMid_Payload_t *CmdPayload       = &UT_CmdBuf.RemoveMidCmd.Payload;

    /* Verify command struct size minus header is at least explicitly padded to 32-bit boundaries */
    UtAssert_True(CMD_STRUCT_DATA_IS_32_ALIGNED(DS_RemoveMidCmd_t), "DS_RemoveMidCmd_t is 32-bit aligned");

    CmdPayload->MessageID                                       = MessageID;
    DS_AppData.FilterTblPtr->Packet[FilterTableIndex].MessageID = MessageID;

    UT_SetDeferredRetcode(UT_KEY(DS_TableFindMsgID), 1, FilterTableIndex);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_RemoveMIDCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_INT32_EQ(CFE_SB_MsgIdToValue(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].MessageID),
                      CFE_SB_MsgIdToValue(CFE_SB_INVALID_MSG_ID));
    UtAssert_INT32_EQ(CFE_SB_MsgIdToValue(DS_AppData.HashLinks[HashTableIndex].MessageID),
                      CFE_SB_MsgIdToValue(CFE_SB_INVALID_MSG_ID));

    /* Check first elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].FilterType, DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[0].Algorithm_O);

    /* Check middle elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].FilterType,
                      DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET / 2].Algorithm_O);

    /* Check last elements */
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FileTableIndex);
    UtAssert_UINT8_EQ(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].FilterType,
                      DS_BY_COUNT);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_N);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_X);
    UtAssert_ZERO(DS_AppData.FilterTblPtr->Packet[FilterTableIndex].Filter[DS_FILTERS_PER_PACKET - 1].Algorithm_O);

    UtAssert_STUB_COUNT(CFE_SB_IsValidMsgId, 1);
    UtAssert_STUB_COUNT(DS_TableHashFunction, 1);
    UtAssert_STUB_COUNT(DS_TableCreateHash, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_REMOVE_MID_CMD_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_RemoveMIDCmd_Test_InvalidMessageID(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.RemoveMidCmd.Payload;

    CmdPayload->MessageID = CFE_SB_INVALID_MSG_ID;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_RemoveMIDCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_REMOVE_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_RemoveMIDCmd_Test_FilterTableNotLoaded(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.RemoveMidCmd.Payload;

    CmdPayload->MessageID = DS_UT_MID_1;

    /* Reset table pointer to NULL (set in test setup) */
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_RemoveMIDCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_REMOVE_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_RemoveMIDCmd_Test_MessageIDNotAdded(void)
{
    DS_AddRemoveMid_Payload_t *CmdPayload = &UT_CmdBuf.RemoveMidCmd.Payload;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    CmdPayload->MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_RemoveMIDCmd(&UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.CmdRejectedCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_REMOVE_MID_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void UtTest_Setup(void)
{
    UtTest_Add(DS_NoopCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_NoopCmd_Test_Nominal");

    UtTest_Add(DS_ResetCountersCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_ResetCountersCmd_Test_Nominal");

    UtTest_Add(DS_SetAppStateCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetAppStateCmd_Test_Nominal");
    UtTest_Add(DS_SetAppStateCmd_Test_InvalidAppState, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetAppStateCmd_Test_InvalidAppState");

    UtTest_Add(DS_SetFilterFileCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetFilterFileCmd_Test_Nominal");
    UtTest_Add(DS_SetFilterFileCmd_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterFileCmd_Test_InvalidMessageID");
    UtTest_Add(DS_SetFilterFileCmd_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterFileCmd_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_SetFilterFileCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterFileCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetFilterFileCmd_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterFileCmd_Test_FilterTableNotLoaded");
    UtTest_Add(DS_SetFilterFileCmd_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterFileCmd_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_SetFilterTypeCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetFilterTypeCmd_Test_Nominal");
    UtTest_Add(DS_SetFilterTypeCmd_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterTypeCmd_Test_InvalidMessageID");
    UtTest_Add(DS_SetFilterTypeCmd_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterTypeCmd_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_SetFilterTypeCmd_Test_InvalidFilterType, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterTypeCmd_Test_InvalidFilterType");
    UtTest_Add(DS_SetFilterTypeCmd_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterTypeCmd_Test_FilterTableNotLoaded");
    UtTest_Add(DS_SetFilterTypeCmd_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterTypeCmd_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_SetFilterParmsCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetFilterParmsCmd_Test_Nominal");
    UtTest_Add(DS_SetFilterParmsCmd_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterParmsCmd_Test_InvalidMessageID");
    UtTest_Add(DS_SetFilterParmsCmd_Test_InvalidFilterParametersIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterParmsCmd_Test_InvalidFilterParametersIndex");
    UtTest_Add(DS_SetFilterParmsCmd_Test_InvalidFilterAlgorithm, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterParmsCmd_Test_InvalidFilterAlgorithm");
    UtTest_Add(DS_SetFilterParmsCmd_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterParmsCmd_Test_FilterTableNotLoaded");
    UtTest_Add(DS_SetFilterParmsCmd_Test_MessageIDNotInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetFilterParmsCmd_Test_MessageIDNotInFilterTable");

    UtTest_Add(DS_SetDestTypeCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestTypeCmd_Test_Nominal");
    UtTest_Add(DS_SetDestTypeCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestTypeCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestTypeCmd_Test_InvalidFilenameType, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestTypeCmd_Test_InvalidFilenameType");
    UtTest_Add(DS_SetDestTypeCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestTypeCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestStateCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestStateCmd_Test_Nominal");
    UtTest_Add(DS_SetDestStateCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestStateCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestStateCmd_Test_InvalidFileState, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestStateCmd_Test_InvalidFileState");
    UtTest_Add(DS_SetDestStateCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestStateCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestPathCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestPathCmd_Test_Nominal");
    UtTest_Add(DS_SetDestPathCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestPathCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestPathCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestPathCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestBaseCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestBaseCmd_Test_Nominal");
    UtTest_Add(DS_SetDestBaseCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestBaseCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestBaseCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestBaseCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestExtCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestExtCmd_Test_Nominal");
    UtTest_Add(DS_SetDestExtCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestExtCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestExtCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestExtCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestSizeCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestSizeCmd_Test_Nominal");
    UtTest_Add(DS_SetDestSizeCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestSizeCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestSizeCmd_Test_InvalidFileSizeLimit, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestSizeCmd_Test_InvalidFileSizeLimit");
    UtTest_Add(DS_SetDestSizeCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestSizeCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestAgeCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestAgeCmd_Test_Nominal");
    UtTest_Add(DS_SetDestAgeCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestAgeCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestAgeCmd_Test_InvalidFileAgeLimit, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestAgeCmd_Test_InvalidFileAgeLimit");
    UtTest_Add(DS_SetDestAgeCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestAgeCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_SetDestCountCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_SetDestCountCmd_Test_Nominal");
    UtTest_Add(DS_SetDestCountCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestCountCmd_Test_InvalidFileTableIndex");
    UtTest_Add(DS_SetDestCountCmd_Test_InvalidFileSequenceCount, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestCountCmd_Test_InvalidFileSequenceCount");
    UtTest_Add(DS_SetDestCountCmd_Test_FileTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_SetDestCountCmd_Test_FileTableNotLoaded");

    UtTest_Add(DS_CloseFileCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CloseFileCmd_Test_Nominal");
    UtTest_Add(DS_CloseFileCmd_Test_NominalAlreadyClosed, DS_Test_Setup, DS_Test_TearDown,
               "DS_CloseFileCmd_Test_NominalAlreadyClosed");
    UtTest_Add(DS_CloseFileCmd_Test_InvalidFileTableIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_CloseFileCmd_Test_InvalidFileTableIndex");

    UtTest_Add(DS_CloseAllCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_CloseAllCmd_Test_Nominal");
    UtTest_Add(DS_CloseAllCmd_Test_CloseAll, DS_Test_Setup, DS_Test_TearDown, "DS_CloseAllCmd_Test_CloseAll");

    UtTest_Add(DS_GetFileInfoCmd_Test_EnabledOpen, DS_Test_Setup, DS_Test_TearDown,
               "DS_GetFileInfoCmd_Test_EnabledOpen");
    UtTest_Add(DS_GetFileInfoCmd_Test_DisabledClosed, DS_Test_Setup, DS_Test_TearDown,
               "DS_GetFileInfoCmd_Test_DisabledClosed");

    UtTest_Add(DS_AddMIDCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_AddMIDCmd_Test_Nominal");
    UtTest_Add(DS_AddMIDCmd_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_AddMIDCmd_Test_InvalidMessageID");
    UtTest_Add(DS_AddMIDCmd_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_AddMIDCmd_Test_FilterTableNotLoaded");
    UtTest_Add(DS_AddMIDCmd_Test_MIDAlreadyInFilterTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_AddMIDCmd_Test_MIDAlreadyInFilterTable");
    UtTest_Add(DS_AddMIDCmd_Test_FilterTableFull, DS_Test_Setup, DS_Test_TearDown, "DS_AddMIDCmd_Test_FilterTableFull");

    UtTest_Add(DS_RemoveMIDCmd_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_RemoveMIDCmd_Test_Nominal");
    UtTest_Add(DS_RemoveMIDCmd_Test_InvalidMessageID, DS_Test_Setup, DS_Test_TearDown,
               "DS_RemoveMIDCmd_Test_InvalidMessageID");
    UtTest_Add(DS_RemoveMIDCmd_Test_FilterTableNotLoaded, DS_Test_Setup, DS_Test_TearDown,
               "DS_RemoveMIDCmd_Test_FilterTableNotLoaded");
    UtTest_Add(DS_RemoveMIDCmd_Test_MessageIDNotAdded, DS_Test_Setup, DS_Test_TearDown,
               "DS_RemoveMIDCmd_Test_MessageIDNotAdded");
}
