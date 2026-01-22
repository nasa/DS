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

#include <stdio.h>

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
 *  CFS Data Storage (DS) command handler functions
 */

#include "cfe.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"
#include "ds_msgids.h"
#include "ds_eventids.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"

#include <stdio.h>

bool DS_VerifyLength(const CFE_SB_Buffer_t *BufPtr, size_t ExpectedLength, uint16 FailEventID, const char *CommandName)
{
    size_t ActualLength = 0;

    CFE_MSG_GetSize(&BufPtr->Msg, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        /*
        ** Invalid command packet length...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(FailEventID, CFE_EVS_EventType_ERROR,
                          "Invalid %s command length: expected = %lu, actual = %lu", CommandName,
                          (unsigned long)ExpectedLength, (unsigned long)ActualLength);
    }

    return (ExpectedLength == ActualLength);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* NOOP command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_NoopVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_NoopCmd_t), DS_NOOP_CMD_ERR_EID, "NOOP"))
    {
        DS_NoopCmd((const DS_NoopCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset hk telemetry counters command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_ResetVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_ResetCountersCmd_t), DS_RESET_CMD_ERR_EID, "RESET"))
    {
        DS_ResetCountersCmd((const DS_ResetCountersCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set application ena/dis state                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetAppStateVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetAppStateCmd_t), DS_ENADIS_CMD_ERR_EID, "APP STATE"))
    {
        DS_SetAppStateCmd((const DS_SetAppStateCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set packet filter file index                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterFileVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetFilterFileCmd_t), DS_FILE_CMD_ERR_EID, "FILTER FILE"))
    {
        DS_SetFilterFileCmd((const DS_SetFilterFileCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set pkt filter filename type                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterTypeVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetFilterTypeCmd_t), DS_FTYPE_CMD_ERR_EID, "FILTER TYPE"))
    {
        DS_SetFilterTypeCmd((const DS_SetFilterTypeCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set packet filter parameters                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterParmsVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetFilterParmsCmd_t), DS_PARMS_CMD_ERR_EID, "FILTER PARMS"))
    {
        DS_SetFilterParmsCmd((const DS_SetFilterParmsCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set destination filename type                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestTypeVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestTypeCmd_t), DS_NTYPE_CMD_ERR_EID, "DEST TYPE"))
    {
        DS_SetDestTypeCmd((const DS_SetDestTypeCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set dest file ena/dis state                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestStateVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestStateCmd_t), DS_STATE_CMD_ERR_EID, "DEST STATE"))
    {
        DS_SetDestStateCmd((const DS_SetDestStateCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set path portion of filename                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestPathVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestPathCmd_t), DS_PATH_CMD_ERR_EID, "DEST PATH"))
    {
        DS_SetDestPathCmd((const DS_SetDestPathCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set base portion of filename                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestBaseVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestBaseCmd_t), DS_BASE_CMD_ERR_EID, "DEST BASE"))
    {
        DS_SetDestBaseCmd((const DS_SetDestBaseCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set extension portion of filename                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestExtVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestExtCmd_t), DS_EXT_CMD_ERR_EID, "DEST EXT"))
    {
        DS_SetDestExtCmd((const DS_SetDestExtCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set maximum file size limit                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestSizeVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestSizeCmd_t), DS_SIZE_CMD_ERR_EID, "DEST SIZE"))
    {
        DS_SetDestSizeCmd((const DS_SetDestSizeCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set maximum file age limit                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestAgeVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestAgeCmd_t), DS_AGE_CMD_ERR_EID, "DEST AGE"))
    {
        DS_SetDestAgeCmd((const DS_SetDestAgeCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set seq cnt portion of filename                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestCountVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SetDestCountCmd_t), DS_SEQ_CMD_ERR_EID, "DEST COUNT"))
    {
        DS_SetDestCountCmd((const DS_SetDestCountCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Close destination file                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CloseFileVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_CloseFileCmd_t), DS_CLOSE_CMD_ERR_EID, "DEST CLOSE"))
    {
        DS_CloseFileCmd((const DS_CloseFileCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Close all open destination files                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CloseAllVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_CloseAllCmd_t), DS_CLOSE_ALL_CMD_ERR_EID, "DEST CLOSE ALL"))
    {
        DS_CloseAllCmd((const DS_CloseAllCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get file info packet                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_GetFileInfoVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_GetFileInfoCmd_t), DS_GET_FILE_INFO_CMD_ERR_EID, "GET FILE INFO"))
    {
        DS_GetFileInfoCmd((const DS_GetFileInfoCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Add message ID to packet filter table                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AddMIDVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_AddMidCmd_t), DS_ADD_MID_CMD_ERR_EID, "ADD MID"))
    {
        DS_AddMidCmd((const DS_AddMidCmd_t *)BufPtr);
    }
}

void DS_RemoveMIDVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_RemoveMidCmd_t), DS_REMOVE_MID_CMD_ERR_EID, "REMOVE MID"))
    {
        DS_RemoveMidCmd((const DS_RemoveMidCmd_t *)BufPtr);
    }
}

void DS_SendHkVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    if (DS_VerifyLength(BufPtr, sizeof(DS_SendHkCmd_t), DS_HKREQ_LEN_ERR_EID, "SEND HK"))
    {
        DS_SendHkCmd((const DS_SendHkCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Software Bus messages                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppPipe(const CFE_SB_Buffer_t *BufPtr)
{
    static CFE_SB_MsgId_t CMD_MID     = CFE_SB_MSGID_RESERVED;
    static CFE_SB_MsgId_t SEND_HK_MID = CFE_SB_MSGID_RESERVED;

    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;

    /* cache the local MID Values here, this avoids repeat lookups */
    if (!CFE_SB_IsValidMsgId(CMD_MID))
    {
        CMD_MID     = CFE_SB_ValueToMsgId(DS_CMD_MID);
        SEND_HK_MID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);
    }

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);

    if (CFE_SB_MsgId_Equal(MessageID, SEND_HK_MID))
    {
        /*
        ** DS housekeeping request command...
        */
        DS_SendHkVerifyDispatch(BufPtr);
        if (DS_TableFindMsgID(MessageID) != DS_INDEX_NONE)
        {
            DS_AppStorePacket(MessageID, BufPtr);
        }
    }
    else if (CFE_SB_MsgId_Equal(MessageID, CMD_MID))
    {
        /*
        ** DS application commands...
        */
        DS_AppProcessCmd(BufPtr);
        if (DS_TableFindMsgID(MessageID) != DS_INDEX_NONE)
        {
            DS_AppStorePacket(MessageID, BufPtr);
        }
    }
    else
    {
        /*
        ** Unknown message ID's (must be something to store)...
        */
        DS_AppStorePacket(MessageID, BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process application commands                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        /*
        ** Do nothing command (aliveness test)...
        */
        case DS_NOOP_CC:
            DS_NoopVerifyDispatch(BufPtr);
            break;

        /*
        ** Set housekeeping telemetry counters to zero...
        */
        case DS_RESET_COUNTERS_CC:
            DS_ResetVerifyDispatch(BufPtr);
            break;

        /*
        ** Set DS application enable/disable state...
        */
        case DS_SET_APP_STATE_CC:
            DS_SetAppStateVerifyDispatch(BufPtr);
            break;

        /*
        ** Set packet filter file index...
        */
        case DS_SET_FILTER_FILE_CC:
            DS_SetFilterFileVerifyDispatch(BufPtr);
            break;

        /*
        ** Set packet filter type (time vs count)...
        */
        case DS_SET_FILTER_TYPE_CC:
            DS_SetFilterTypeVerifyDispatch(BufPtr);
            break;

        /*
        ** Set packet filter algorithm parameters...
        */
        case DS_SET_FILTER_PARMS_CC:
            DS_SetFilterParmsVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file filename type (time vs count)...
        */
        case DS_SET_DEST_TYPE_CC:
            DS_SetDestTypeVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file enable/disable state...
        */
        case DS_SET_DEST_STATE_CC:
            DS_SetDestStateVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file path portion of filename...
        */
        case DS_SET_DEST_PATH_CC:
            DS_SetDestPathVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file base portion of filename...
        */
        case DS_SET_DEST_BASE_CC:
            DS_SetDestBaseVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file extension portion of filename...
        */
        case DS_SET_DEST_EXT_CC:
            DS_SetDestExtVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file maximum size limit...
        */
        case DS_SET_DEST_SIZE_CC:
            DS_SetDestSizeVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file maximum age limit...
        */
        case DS_SET_DEST_AGE_CC:
            DS_SetDestAgeVerifyDispatch(BufPtr);
            break;

        /*
        ** Set destination file sequence count portion of filename...
        */
        case DS_SET_DEST_COUNT_CC:
            DS_SetDestCountVerifyDispatch(BufPtr);
            break;

        /*
        ** Close destination file (next packet will re-open)...
        */
        case DS_CLOSE_FILE_CC:
            DS_CloseFileVerifyDispatch(BufPtr);
            break;

        /*
        ** Get file info telemetry packet...
        */
        case DS_GET_FILE_INFO_CC:
            DS_GetFileInfoVerifyDispatch(BufPtr);
            break;

        /*
        ** Add message ID to filter table...
        */
        case DS_ADD_MID_CC:
            DS_AddMIDVerifyDispatch(BufPtr);
            break;

        /*
        ** Remove message ID from filter table...
        */
        case DS_REMOVE_MID_CC:
            DS_RemoveMIDVerifyDispatch(BufPtr);
            break;

        /*
        ** Close all destination files (next packet will re-open)...
        */
        case DS_CLOSE_ALL_CC:
            DS_CloseAllVerifyDispatch(BufPtr);
            break;

        /*
        ** DS application command with unknown command code...
        */
        default:
            CFE_EVS_SendEvent(DS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command code: MID = 0x%08X, CC = %d",
                              DS_CMD_MID, CommandCode);

            DS_AppData.CmdRejectedCounter++;
            break;
    }
}
