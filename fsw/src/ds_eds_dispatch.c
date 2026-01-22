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
 *  The CFS Data Storage (DS) Application file containing the application
 *  initialization routines, the main routine and the command interface.
 */

#include "cfe.h"
#include "ds_perfids.h"
#include "ds_msgids.h"
#include "ds_platform_cfg.h"
#include "ds_dispatch.h"
#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_eventids.h"
#include "ds_msgdefs.h"

#include "ds_eds_dictionary.h"
#include "ds_eds_dispatcher.h"

/* clang-format off */
static const EdsDispatchTable_EdsComponent_DS_Application_CFE_SB_Telecommand_t DS_TC_DISPATCH_TABLE = {
    .CMD =
    {
        .AddMidCmd_indication = DS_AddMidCmd,
        .SetAppStateCmd_indication = DS_SetAppStateCmd,
        .CloseAllCmd_indication = DS_CloseAllCmd,
        .CloseFileCmd_indication = DS_CloseFileCmd,
        .SetDestAgeCmd_indication = DS_SetDestAgeCmd,
        .SetDestBaseCmd_indication = DS_SetDestBaseCmd,
        .SetDestCountCmd_indication = DS_SetDestCountCmd,
        .SetDestExtCmd_indication = DS_SetDestExtCmd,
        .SetDestPathCmd_indication = DS_SetDestPathCmd,
        .SetDestSizeCmd_indication = DS_SetDestSizeCmd,
        .SetDestStateCmd_indication = DS_SetDestStateCmd,
        .SetDestTypeCmd_indication = DS_SetDestTypeCmd,
        .SetFilterFileCmd_indication = DS_SetFilterFileCmd,
        .SetFilterParmsCmd_indication = DS_SetFilterParmsCmd,
        .SetFilterTypeCmd_indication = DS_SetFilterTypeCmd,
        .GetFileInfoCmd_indication = DS_GetFileInfoCmd,
        .NoopCmd_indication = DS_NoopCmd,
        .RemoveMidCmd_indication = DS_RemoveMidCmd,
        .ResetCountersCmd_indication = DS_ResetCountersCmd,
    },
    .SEND_HK          =
    {
        .indication = DS_SendHkCmd
    }
};
/* clang-format on */

const uint16 DS_EVENTID_CC_LOOKUP[] = {
    [DS_NOOP_CC]             = DS_NOOP_CMD_ERR_EID,
    [DS_RESET_COUNTERS_CC]   = DS_RESET_CMD_ERR_EID,
    [DS_SET_APP_STATE_CC]    = DS_ENADIS_CMD_ERR_EID,
    [DS_SET_FILTER_FILE_CC]  = DS_FILE_CMD_ERR_EID,
    [DS_SET_FILTER_TYPE_CC]  = DS_FTYPE_CMD_ERR_EID,
    [DS_SET_FILTER_PARMS_CC] = DS_PARMS_CMD_ERR_EID,
    [DS_SET_DEST_TYPE_CC]    = DS_NTYPE_CMD_ERR_EID,
    [DS_SET_DEST_STATE_CC]   = DS_STATE_CMD_ERR_EID,
    [DS_SET_DEST_PATH_CC]    = DS_PATH_CMD_ERR_EID,
    [DS_SET_DEST_BASE_CC]    = DS_BASE_CMD_ERR_EID,
    [DS_SET_DEST_EXT_CC]     = DS_EXT_CMD_ERR_EID,
    [DS_SET_DEST_SIZE_CC]    = DS_SIZE_CMD_ERR_EID,
    [DS_SET_DEST_AGE_CC]     = DS_AGE_CMD_ERR_EID,
    [DS_SET_DEST_COUNT_CC]   = DS_SEQ_CMD_ERR_EID,
    [DS_CLOSE_FILE_CC]       = DS_CLOSE_CMD_ERR_EID,
    [DS_CLOSE_ALL_CC]        = DS_CLOSE_ALL_CMD_ERR_EID,
    [DS_GET_FILE_INFO_CC]    = DS_GET_FILE_INFO_CMD_ERR_EID,
    [DS_ADD_MID_CC]          = DS_ADD_MID_CMD_ERR_EID,
    [DS_REMOVE_MID_CC]       = DS_REMOVE_MID_CMD_ERR_EID,
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppPipe() -- Process command pipe message                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_AppPipe(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_Status_t      status;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_Size_t    MsgSize;
    CFE_MSG_FcnCode_t MsgFc;
    uint16            FailEventID;
    bool              stored = false;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MsgId);

    if (DS_TableFindMsgID(MsgId) != DS_INDEX_NONE)
    {
        DS_AppStorePacket(MsgId, BufPtr);
        stored = true;
    }

    status = EdsDispatch_EdsComponent_DS_Application_Telecommand(BufPtr, &DS_TC_DISPATCH_TABLE);

    if (status != CFE_SUCCESS && !stored)
    {
        DS_AppData.CmdRejectedCounter++;

        if (status == CFE_STATUS_VALIDATION_FAILURE || status == CFE_STATUS_UNKNOWN_MSG_ID)
        {
            CFE_EVS_SendEvent(DS_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId));
        }
        else
        {
            CFE_MSG_GetFcnCode(&BufPtr->Msg, &MsgFc);
            if (status == CFE_STATUS_WRONG_MSG_LENGTH)
            {
                if (CFE_SB_MsgIdToValue(MsgId) == DS_CMD_MID &&
                    MsgFc < (sizeof(DS_EVENTID_CC_LOOKUP) / sizeof(DS_EVENTID_CC_LOOKUP[0])))
                {
                    FailEventID = DS_EVENTID_CC_LOOKUP[MsgFc];
                }
                else
                {
                    FailEventID = DS_HKREQ_LEN_ERR_EID;
                }
                CFE_MSG_GetSize(&BufPtr->Msg, &MsgSize);
                CFE_EVS_SendEvent(FailEventID, CFE_EVS_EventType_ERROR,
                                  "Invalid command length: MID = 0x%08X, CC = %d, Len = %lu",
                                  (unsigned int)CFE_SB_MsgIdToValue(MsgId), (int)MsgFc, (unsigned long)MsgSize);
            }
            else
            {
                CFE_EVS_SendEvent(DS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid message recvd: MID = 0x%08X, CC = %d, status=%d",
                                  (unsigned int)CFE_SB_MsgIdToValue(MsgId), (int)MsgFc, (int)status);
            }
        }
    }
}
