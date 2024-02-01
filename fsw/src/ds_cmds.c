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

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"
#include "ds_version.h"

#include <stdio.h>

/**
 * \brief Internal Macro to access the internal payload structure of a message
 *
 * This is done as a macro so it can be applied consistently to all
 * message processing functions, based on the way DS defines its messages.
 */
#define DS_GET_CMD_PAYLOAD(ptr, type) (&((const type *)(ptr))->Payload)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* NOOP command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_NoopCmd(const CFE_SB_Buffer_t *BufPtr)
{
    /*
    ** Do nothing except display "aliveness" event...
    */
    DS_AppData.CmdAcceptedCounter++;

    CFE_EVS_SendEvent(DS_NOOP_CMD_EID, CFE_EVS_EventType_INFORMATION, "NOOP command, Version %d.%d.%d.%d",
                      DS_MAJOR_VERSION, DS_MINOR_VERSION, DS_REVISION, DS_MISSION_REV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset hk telemetry counters command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_ResetCountersCmd(const CFE_SB_Buffer_t *BufPtr)
{
    /*
    ** Reset application command counters...
    */
    DS_AppData.CmdAcceptedCounter = 0;
    DS_AppData.CmdRejectedCounter = 0;

    /*
    ** Reset packet storage counters...
    */
    DS_AppData.DisabledPktCounter = 0;
    DS_AppData.IgnoredPktCounter  = 0;
    DS_AppData.FilteredPktCounter = 0;
    DS_AppData.PassedPktCounter   = 0;

    /*
    ** Reset file I/O counters...
    */
    DS_AppData.FileWriteCounter     = 0;
    DS_AppData.FileWriteErrCounter  = 0;
    DS_AppData.FileUpdateCounter    = 0;
    DS_AppData.FileUpdateErrCounter = 0;

    /*
    ** Reset configuration table counters...
    */
    DS_AppData.DestTblLoadCounter   = 0;
    DS_AppData.DestTblErrCounter    = 0;
    DS_AppData.FilterTblLoadCounter = 0;
    DS_AppData.FilterTblErrCounter  = 0;

    CFE_EVS_SendEvent(DS_RESET_CMD_EID, CFE_EVS_EventType_DEBUG, "Reset counters command");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set application ena/dis state                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetAppStateCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_AppState_Payload_t *DS_AppStateCmd;

    DS_AppStateCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_AppStateCmd_t);

    if (DS_TableVerifyState(DS_AppStateCmd->EnableState) == false)
    {
        /*
        ** Invalid enable/disable state...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_ENADIS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid APP STATE command arg: app state = %d", DS_AppStateCmd->EnableState);
    }
    else
    {
        /*
        ** Set new DS application enable/disable state...
        */
        DS_AppData.AppEnableState = DS_AppStateCmd->EnableState;

        /*
        ** Update the Critical Data Store (CDS)...
        */
        DS_TableUpdateCDS();

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_ENADIS_CMD_EID, CFE_EVS_EventType_INFORMATION, "APP STATE command: state = %d",
                          DS_AppStateCmd->EnableState);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set packet filter file index                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_FilterFile_Payload_t *DS_FilterFileCmd;

    DS_PacketEntry_t *pPacketEntry     = NULL;
    DS_FilterParms_t *pFilterParms     = NULL;
    int32             FilterTableIndex = 0;

    DS_FilterFileCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_FilterFileCmd_t);

    if (!CFE_SB_IsValidMsgId(DS_FilterFileCmd->MessageID))
    {
        /*
        ** Invalid packet messageID...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FILE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER FILE command arg: invalid messageID = 0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(DS_FilterFileCmd->MessageID));
    }
    else if (DS_FilterFileCmd->FilterParmsIndex >= DS_FILTERS_PER_PACKET)
    {
        /*
        ** Invalid packet filter parameters index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FILE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER FILE command arg: filter parameters index = %d",
                          DS_FilterFileCmd->FilterParmsIndex);
    }
    else if (DS_TableVerifyFileIndex(DS_FilterFileCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FILE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER FILE command arg: file table index = %d", DS_FilterFileCmd->FileTableIndex);
    }
    else if (DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL)
    {
        /*
        ** Must have a valid packet filter table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FILE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER FILE command: packet filter table is not loaded");
    }
    else
    {
        /*
        ** Get the index of the filter table entry for this message ID...
        */
        FilterTableIndex = DS_TableFindMsgID(DS_FilterFileCmd->MessageID);

        if (FilterTableIndex == DS_INDEX_NONE)
        {
            /*
            ** Must not create - may only modify existing packet filter...
            */
            DS_AppData.CmdRejectedCounter++;

            CFE_EVS_SendEvent(DS_FILE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid FILTER FILE command: Message ID 0x%08lX is not in filter table",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterFileCmd->MessageID));
        }
        else
        {
            /*
            ** Set new packet filter value (file table index)...
            */
            pPacketEntry = &DS_AppData.FilterTblPtr->Packet[FilterTableIndex];
            pFilterParms = &pPacketEntry->Filter[DS_FilterFileCmd->FilterParmsIndex];

            pFilterParms->FileTableIndex = DS_FilterFileCmd->FileTableIndex;

            /*
            ** Notify cFE that we have modified the table data...
            */
            CFE_TBL_Modified(DS_AppData.FilterTblHandle);

            DS_AppData.CmdAcceptedCounter++;

            CFE_EVS_SendEvent(DS_FILE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                              "FILTER FILE command: MID = 0x%08lX, index = %d, filter = %d, file = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterFileCmd->MessageID), (int)FilterTableIndex,
                              DS_FilterFileCmd->FilterParmsIndex, DS_FilterFileCmd->FileTableIndex);
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set pkt filter filename type                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterTypeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_FilterType_Payload_t *DS_FilterTypeCmd;

    DS_PacketEntry_t *pPacketEntry     = NULL;
    DS_FilterParms_t *pFilterParms     = NULL;
    int32             FilterTableIndex = 0;

    DS_FilterTypeCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_FilterTypeCmd_t);

    if (!CFE_SB_IsValidMsgId(DS_FilterTypeCmd->MessageID))
    {
        /*
        ** Invalid packet messageID...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER TYPE command arg: invalid messageID = 0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(DS_FilterTypeCmd->MessageID));
    }
    else if (DS_FilterTypeCmd->FilterParmsIndex >= DS_FILTERS_PER_PACKET)
    {
        /*
        ** Invalid packet filter parameters index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER TYPE command arg: filter parameters index = %d",
                          DS_FilterTypeCmd->FilterParmsIndex);
    }
    else if (DS_TableVerifyType(DS_FilterTypeCmd->FilterType) == false)
    {
        /*
        ** Invalid packet filter filename type...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER TYPE command arg: filter type = %d", DS_FilterTypeCmd->FilterType);
    }
    else if (DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL)
    {
        /*
        ** Must have a valid packet filter table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_FTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER TYPE command: packet filter table is not loaded");
    }
    else
    {
        /*
        ** Get the index of the filter table entry for this message ID...
        */
        FilterTableIndex = DS_TableFindMsgID(DS_FilterTypeCmd->MessageID);

        if (FilterTableIndex == DS_INDEX_NONE)
        {
            /*
            ** Must not create - may only modify existing packet filter...
            */
            DS_AppData.CmdRejectedCounter++;

            CFE_EVS_SendEvent(DS_FTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid FILTER TYPE command: Message ID 0x%08lX is not in filter table",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterTypeCmd->MessageID));
        }
        else
        {
            /*
            ** Set new packet filter value (filter type)...
            */
            pPacketEntry = &DS_AppData.FilterTblPtr->Packet[FilterTableIndex];
            pFilterParms = &pPacketEntry->Filter[DS_FilterTypeCmd->FilterParmsIndex];

            pFilterParms->FilterType = DS_FilterTypeCmd->FilterType;

            /*
            ** Notify cFE that we have modified the table data...
            */
            CFE_TBL_Modified(DS_AppData.FilterTblHandle);

            DS_AppData.CmdAcceptedCounter++;

            CFE_EVS_SendEvent(DS_FTYPE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                              "FILTER TYPE command: MID = 0x%08lX, index = %d, filter = %d, type = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterTypeCmd->MessageID), (int)FilterTableIndex,
                              DS_FilterTypeCmd->FilterParmsIndex, DS_FilterTypeCmd->FilterType);
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set packet filter parameters                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetFilterParmsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_FilterParms_Payload_t *DS_FilterParmsCmd;

    DS_PacketEntry_t *pPacketEntry     = NULL;
    DS_FilterParms_t *pFilterParms     = NULL;
    int32             FilterTableIndex = 0;

    DS_FilterParmsCmd = &((const DS_FilterParmsCmd_t *)BufPtr)->Payload;

    if (!CFE_SB_IsValidMsgId(DS_FilterParmsCmd->MessageID))
    {
        /*
        ** Invalid packet messageID...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PARMS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER PARMS command arg: invalid messageID = 0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(DS_FilterParmsCmd->MessageID));
    }
    else if (DS_FilterParmsCmd->FilterParmsIndex >= DS_FILTERS_PER_PACKET)
    {
        /*
        ** Invalid packet filter parameters index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PARMS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER PARMS command arg: filter parameters index = %d",
                          DS_FilterParmsCmd->FilterParmsIndex);
    }
    else if (DS_TableVerifyParms(DS_FilterParmsCmd->Algorithm_N, DS_FilterParmsCmd->Algorithm_X,
                                 DS_FilterParmsCmd->Algorithm_O) == false)
    {
        /*
        ** Invalid packet filter algorithm parameters...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PARMS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER PARMS command arg: N = %d, X = %d, O = %d", DS_FilterParmsCmd->Algorithm_N,
                          DS_FilterParmsCmd->Algorithm_X, DS_FilterParmsCmd->Algorithm_O);
    }
    else if (DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL)
    {
        /*
        ** Must have a valid packet filter table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PARMS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid FILTER PARMS command: packet filter table is not loaded");
    }
    else
    {
        /*
        ** Get the index of the filter table entry for this message ID...
        */
        FilterTableIndex = DS_TableFindMsgID(DS_FilterParmsCmd->MessageID);

        if (FilterTableIndex == DS_INDEX_NONE)
        {
            /*
            ** Must not create - may only modify existing packet filter...
            */
            DS_AppData.CmdRejectedCounter++;

            CFE_EVS_SendEvent(DS_PARMS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid FILTER PARMS command: Message ID 0x%08lX is not in filter table",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterParmsCmd->MessageID));
        }
        else
        {
            /*
            ** Set new packet filter values (algorithm)...
            */
            pPacketEntry = &DS_AppData.FilterTblPtr->Packet[FilterTableIndex];
            pFilterParms = &pPacketEntry->Filter[DS_FilterParmsCmd->FilterParmsIndex];

            pFilterParms->Algorithm_N = DS_FilterParmsCmd->Algorithm_N;
            pFilterParms->Algorithm_X = DS_FilterParmsCmd->Algorithm_X;
            pFilterParms->Algorithm_O = DS_FilterParmsCmd->Algorithm_O;

            /*
            ** Notify cFE that we have modified the table data...
            */
            CFE_TBL_Modified(DS_AppData.FilterTblHandle);

            DS_AppData.CmdAcceptedCounter++;

            CFE_EVS_SendEvent(DS_PARMS_CMD_EID, CFE_EVS_EventType_INFORMATION,
                              "FILTER PARMS command: MID = 0x%08lX, index = %d, filter = %d, N = %d, X = %d, O = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(DS_FilterParmsCmd->MessageID), (int)FilterTableIndex,
                              DS_FilterParmsCmd->FilterParmsIndex, pFilterParms->Algorithm_N, pFilterParms->Algorithm_X,
                              pFilterParms->Algorithm_O);
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set destination filename type                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestTypeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestType_Payload_t *DS_DestTypeCmd;

    DS_DestFileEntry_t *pDest = NULL;

    DS_DestTypeCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestTypeCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestTypeCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_NTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST TYPE command arg: file table index = %d", DS_DestTypeCmd->FileTableIndex);
    }
    else if (DS_TableVerifyType(DS_DestTypeCmd->FileNameType) == false)
    {
        /*
        ** Invalid destination filename type...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_NTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST TYPE command arg: filename type = %d", DS_DestTypeCmd->FileNameType);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_NTYPE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST TYPE command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set new destination table filename type...
        */
        pDest               = &DS_AppData.DestFileTblPtr->File[DS_DestTypeCmd->FileTableIndex];
        pDest->FileNameType = DS_DestTypeCmd->FileNameType;

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_NTYPE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST TYPE command: file table index = %d, filename type = %d",
                          DS_DestTypeCmd->FileTableIndex, DS_DestTypeCmd->FileNameType);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set dest file ena/dis state                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestStateCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestState_Payload_t *DS_DestStateCmd;

    DS_DestStateCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestStateCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestStateCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_STATE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST STATE command arg: file table index = %d", DS_DestStateCmd->FileTableIndex);
    }
    else if (DS_TableVerifyState(DS_DestStateCmd->EnableState) == false)
    {
        /*
        ** Invalid destination file state...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_STATE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST STATE command arg: file state = %d", DS_DestStateCmd->EnableState);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_STATE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST STATE command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set new destination table file state in table and in current status...
        */
        DS_AppData.DestFileTblPtr->File[DS_DestStateCmd->FileTableIndex].EnableState = DS_DestStateCmd->EnableState;
        DS_AppData.FileStatus[DS_DestStateCmd->FileTableIndex].FileState             = DS_DestStateCmd->EnableState;

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_STATE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST STATE command: file table index = %d, file state = %d", DS_DestStateCmd->FileTableIndex,
                          DS_DestStateCmd->EnableState);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set path portion of filename                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestPathCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestPath_Payload_t *DS_DestPathCmd;

    DS_DestFileEntry_t *pDest = NULL;

    DS_DestPathCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestPathCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestPathCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PATH_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST PATH command arg: file table index = %d", (int)DS_DestPathCmd->FileTableIndex);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_PATH_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST PATH command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set path portion of destination table filename...
        */
        pDest = &DS_AppData.DestFileTblPtr->File[DS_DestPathCmd->FileTableIndex];
        CFE_SB_MessageStringGet(pDest->Pathname, DS_DestPathCmd->Pathname, NULL, sizeof(pDest->Pathname),
                                sizeof(DS_DestPathCmd->Pathname));

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_PATH_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST PATH command: file table index = %d, pathname = '%s'",
                          (int)DS_DestPathCmd->FileTableIndex, DS_DestPathCmd->Pathname);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set base portion of filename                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestBaseCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestBase_Payload_t *DS_DestBaseCmd;
    DS_DestFileEntry_t *         pDest = NULL;

    DS_DestBaseCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestBaseCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestBaseCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_BASE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST BASE command arg: file table index = %d", (int)DS_DestBaseCmd->FileTableIndex);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_BASE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST BASE command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set base portion of destination table filename...
        */
        pDest = &DS_AppData.DestFileTblPtr->File[DS_DestBaseCmd->FileTableIndex];
        CFE_SB_MessageStringGet(pDest->Basename, DS_DestBaseCmd->Basename, NULL, sizeof(pDest->Basename),
                                sizeof(DS_DestBaseCmd->Basename));

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_BASE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST BASE command: file table index = %d, base filename = '%s'",
                          (int)DS_DestBaseCmd->FileTableIndex, DS_DestBaseCmd->Basename);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set extension portion of filename                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestExtCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestExt_Payload_t *DS_DestExtCmd;
    DS_DestFileEntry_t *        pDest = NULL;

    DS_DestExtCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestExtCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestExtCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_EXT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST EXT command arg: file table index = %d", (int)DS_DestExtCmd->FileTableIndex);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_EXT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST EXT command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set extension portion of destination table filename...
        */
        pDest = &DS_AppData.DestFileTblPtr->File[DS_DestExtCmd->FileTableIndex];
        CFE_SB_MessageStringGet(pDest->Extension, DS_DestExtCmd->Extension, NULL, sizeof(pDest->Extension),
                                sizeof(DS_DestExtCmd->Extension));

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_EXT_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST EXT command: file table index = %d, extension = '%s'",
                          (int)DS_DestExtCmd->FileTableIndex, DS_DestExtCmd->Extension);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set maximum file size limit                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestSizeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestSize_Payload_t *DS_DestSizeCmd;
    DS_DestFileEntry_t *         pDest = NULL;

    DS_DestSizeCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestSizeCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestSizeCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SIZE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST SIZE command arg: file table index = %d", (int)DS_DestSizeCmd->FileTableIndex);
    }
    else if (DS_TableVerifySize(DS_DestSizeCmd->MaxFileSize) == false)
    {
        /*
        ** Invalid destination file size limit...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SIZE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST SIZE command arg: size limit = %d", (int)DS_DestSizeCmd->MaxFileSize);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SIZE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST SIZE command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set size limit for destination file...
        */
        pDest              = &DS_AppData.DestFileTblPtr->File[DS_DestSizeCmd->FileTableIndex];
        pDest->MaxFileSize = DS_DestSizeCmd->MaxFileSize;

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_SIZE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST SIZE command: file table index = %d, size limit = %d",
                          (int)DS_DestSizeCmd->FileTableIndex, (int)DS_DestSizeCmd->MaxFileSize);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set maximum file age limit                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestAgeCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestAge_Payload_t *DS_DestAgeCmd;
    DS_DestFileEntry_t *        pDest = NULL;

    DS_DestAgeCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestAgeCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestAgeCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_AGE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST AGE command arg: file table index = %d", (int)DS_DestAgeCmd->FileTableIndex);
    }
    else if (DS_TableVerifyAge(DS_DestAgeCmd->MaxFileAge) == false)
    {
        /*
        ** Invalid destination file age limit...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_AGE_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid DEST AGE command arg: age limit = %d",
                          (int)DS_DestAgeCmd->MaxFileAge);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_AGE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST AGE command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set age limit for destination file...
        */
        pDest             = &DS_AppData.DestFileTblPtr->File[DS_DestAgeCmd->FileTableIndex];
        pDest->MaxFileAge = DS_DestAgeCmd->MaxFileAge;

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_AGE_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST AGE command: file table index = %d, age limit = %d", (int)DS_DestAgeCmd->FileTableIndex,
                          (int)DS_DestAgeCmd->MaxFileAge);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set seq cnt portion of filename                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_SetDestCountCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_DestCount_Payload_t *DS_DestCountCmd;
    DS_AppFileStatus_t *          FileStatus = NULL;
    DS_DestFileEntry_t *          DestFile   = NULL;

    DS_DestCountCmd = DS_GET_CMD_PAYLOAD(BufPtr, DS_DestCountCmd_t);

    if (DS_TableVerifyFileIndex(DS_DestCountCmd->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SEQ_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST COUNT command arg: file table index = %d",
                          (int)DS_DestCountCmd->FileTableIndex);
    }
    else if (DS_TableVerifyCount(DS_DestCountCmd->SequenceCount) == false)
    {
        /*
        ** Invalid destination file sequence count...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SEQ_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST COUNT command arg: sequence count = %d", (int)DS_DestCountCmd->SequenceCount);
    }
    else if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Must have a valid destination file table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_SEQ_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST COUNT command: destination file table is not loaded");
    }
    else
    {
        /*
        ** Set next sequence count for destination file...
        */
        DestFile   = &DS_AppData.DestFileTblPtr->File[DS_DestCountCmd->FileTableIndex];
        FileStatus = &DS_AppData.FileStatus[DS_DestCountCmd->FileTableIndex];

        /*
        ** Update both destination file table and current status...
        */
        DestFile->SequenceCount = DS_DestCountCmd->SequenceCount;
        FileStatus->FileCount   = DS_DestCountCmd->SequenceCount;

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.DestFileTblHandle);

        /*
        ** Update the Critical Data Store (CDS)...
        */
        DS_TableUpdateCDS();

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_SEQ_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "DEST COUNT command: file table index = %d, sequence count = %d",
                          (int)DS_DestCountCmd->FileTableIndex, (int)DS_DestCountCmd->SequenceCount);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Close destination file                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CloseFileCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_CloseFile_Payload_t *PayloadPtr;

    PayloadPtr = DS_GET_CMD_PAYLOAD(BufPtr, DS_CloseFileCmd_t);

    if (DS_TableVerifyFileIndex(PayloadPtr->FileTableIndex) == false)
    {
        /*
        ** Invalid destination file table index...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_CLOSE_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid DEST CLOSE command arg: file table index = %d",
                          (int)PayloadPtr->FileTableIndex);
    }
    else
    {
        /*
        ** Close destination file (if the file was open)...
        */
        if (OS_ObjectIdDefined(DS_AppData.FileStatus[PayloadPtr->FileTableIndex].FileHandle))
        {
            DS_FileUpdateHeader(PayloadPtr->FileTableIndex);
            DS_FileCloseDest(PayloadPtr->FileTableIndex);
        }

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_CLOSE_CMD_EID, CFE_EVS_EventType_INFORMATION, "DEST CLOSE command: file table index = %d",
                          (int)PayloadPtr->FileTableIndex);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Close all open destination files                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CloseAllCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32 i = 0;

    /*
    ** Close all open destination files...
    */
    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        if (OS_ObjectIdDefined(DS_AppData.FileStatus[i].FileHandle))
        {
            DS_FileUpdateHeader(i);
            DS_FileCloseDest(i);
        }
    }

    DS_AppData.CmdAcceptedCounter++;

    CFE_EVS_SendEvent(DS_CLOSE_ALL_CMD_EID, CFE_EVS_EventType_INFORMATION, "DEST CLOSE ALL command");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Get file info packet                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_GetFileInfoCmd(const CFE_SB_Buffer_t *BufPtr)
{
    DS_FileInfoPkt_t DS_FileInfoPkt;
    DS_FileInfo_t *  FileInfoPtr;
    int32            i = 0;

    /*
    ** Create and send a file info packet...
    */
    DS_AppData.CmdAcceptedCounter++;

    CFE_EVS_SendEvent(DS_GET_FILE_INFO_CMD_EID, CFE_EVS_EventType_INFORMATION, "GET FILE INFO command");

    /*
    ** Initialize file info telemetry packet...
    */
    CFE_MSG_Init(CFE_MSG_PTR(DS_FileInfoPkt.TelemetryHeader), CFE_SB_ValueToMsgId(DS_DIAG_TLM_MID),
                 sizeof(DS_FileInfoPkt_t));

    /*
    ** Process array of destination file info data...
    */
    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        FileInfoPtr = &DS_FileInfoPkt.Payload[i];

        /*
        ** Set file age and size...
        */
        FileInfoPtr->FileAge  = DS_AppData.FileStatus[i].FileAge;
        FileInfoPtr->FileSize = DS_AppData.FileStatus[i].FileSize;

        /*
        ** Set file growth rate (computed when process last HK request)...
        */
        FileInfoPtr->FileRate = DS_AppData.FileStatus[i].FileRate;

        /*
        ** Set current filename sequence count...
        */
        FileInfoPtr->SequenceCount = DS_AppData.FileStatus[i].FileCount;

        /*
        ** Set file enable/disable state...
        */
        if (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL)
        {
            FileInfoPtr->EnableState = DS_DISABLED;
        }
        else
        {
            FileInfoPtr->EnableState = DS_AppData.FileStatus[i].FileState;
        }

        /*
        ** Set file open/closed state...
        */
        if (!OS_ObjectIdDefined(DS_AppData.FileStatus[i].FileHandle))
        {
            FileInfoPtr->OpenState = DS_CLOSED;
        }
        else
        {
            FileInfoPtr->OpenState = DS_OPEN;

            /*
            ** Set current open filename...
            */
            strncpy(FileInfoPtr->FileName, DS_AppData.FileStatus[i].FileName, sizeof(FileInfoPtr->FileName));
        }
    }

    /*
    ** Timestamp and send file info telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(DS_FileInfoPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(DS_FileInfoPkt.TelemetryHeader), true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Add message ID to packet filter table                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AddMIDCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_AddRemoveMid_Payload_t *PayloadPtr;
    DS_PacketEntry_t *               pPacketEntry     = NULL;
    DS_FilterParms_t *               pFilterParms     = NULL;
    int32                            FilterTableIndex = 0;
    int32                            HashTableIndex   = 0;
    int32                            i                = 0;

    PayloadPtr = DS_GET_CMD_PAYLOAD(BufPtr, DS_AddMidCmd_t);

    if (!CFE_SB_IsValidMsgId(PayloadPtr->MessageID))
    {
        /*
        ** Invalid packet message ID - can be anything but unused...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_ADD_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid ADD MID command arg: invalid MID = 0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID));
    }
    else if (DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL)
    {
        /*
        ** Must have a valid packet filter table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_ADD_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid ADD MID command: filter table is not loaded");
    }
    else if ((FilterTableIndex = DS_TableFindMsgID(PayloadPtr->MessageID)) != DS_INDEX_NONE)
    {
        /*
        ** New message ID is already in packet filter table...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_ADD_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid ADD MID command: MID = 0x%08lX is already in filter table at index = %d",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID), (int)FilterTableIndex);
    }
    else if ((FilterTableIndex = DS_TableFindMsgID(CFE_SB_INVALID_MSG_ID)) == DS_INDEX_NONE)
    {
        /*
        ** Packet filter table has no unused entries...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_ADD_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid ADD MID command: filter table is full");
    }
    else
    {
        /*
        ** Initialize unused packet filter entry for new message ID...
        */
        pPacketEntry = &DS_AppData.FilterTblPtr->Packet[FilterTableIndex];

        pPacketEntry->MessageID = PayloadPtr->MessageID;

        /* Add the message ID to the hash table as well */
        HashTableIndex = DS_TableAddMsgID(PayloadPtr->MessageID, FilterTableIndex);

        for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
        {
            pFilterParms = &pPacketEntry->Filter[i];

            pFilterParms->FileTableIndex = 0;
            pFilterParms->FilterType     = DS_BY_COUNT;

            pFilterParms->Algorithm_N = 0;
            pFilterParms->Algorithm_X = 0;
            pFilterParms->Algorithm_O = 0;
        }

        CFE_SB_SubscribeEx(PayloadPtr->MessageID, DS_AppData.CmdPipe, CFE_SB_DEFAULT_QOS, DS_PER_PACKET_PIPE_LIMIT);
        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.FilterTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_ADD_MID_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "ADD MID command: MID = 0x%08lX, filter index = %d, hash index = %d",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID), (int)FilterTableIndex,
                          (int)HashTableIndex);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_RemoveMIDCmd() - remove message ID from packet filter table  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_RemoveMIDCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const DS_AddRemoveMid_Payload_t *PayloadPtr;

    DS_PacketEntry_t *pPacketEntry     = NULL;
    DS_FilterParms_t *pFilterParms     = NULL;
    int32             FilterTableIndex = 0;
    int32             HashTableIndex   = 0;
    int32             i                = 0;

    PayloadPtr  = DS_GET_CMD_PAYLOAD(BufPtr, DS_RemoveMidCmd_t);
    FilterTableIndex = DS_TableFindMsgID(PayloadPtr->MessageID);

    if (!CFE_SB_IsValidMsgId(PayloadPtr->MessageID))
    {
        /*
        ** Invalid packet message ID - can be anything but unused...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_REMOVE_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid REMOVE MID command arg: invalid MID = 0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID));
    }
    else if (DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL)
    {
        /*
        ** Must have a valid packet filter table loaded...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_REMOVE_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid REMOVE MID command: filter table is not loaded");
    }
    else if (FilterTableIndex == DS_INDEX_NONE)
    {
        /*
        ** Message ID is not in packet filter table...
        */
        DS_AppData.CmdRejectedCounter++;

        CFE_EVS_SendEvent(DS_REMOVE_MID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid REMOVE MID command: MID = 0x%08lX is not in filter table",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID));
    }
    else
    {
        /* Convert MID into hash table index */
        HashTableIndex = DS_TableHashFunction(PayloadPtr->MessageID);

        /*
        ** Reset used packet filter entry for used message ID...
        */
        pPacketEntry = &DS_AppData.FilterTblPtr->Packet[FilterTableIndex];

        pPacketEntry->MessageID = CFE_SB_INVALID_MSG_ID;

        /* Create new hash table as well */
        DS_TableCreateHash();

        for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
        {
            pFilterParms = &pPacketEntry->Filter[i];

            pFilterParms->FileTableIndex = 0;
            pFilterParms->FilterType     = DS_BY_COUNT;

            pFilterParms->Algorithm_N = 0;
            pFilterParms->Algorithm_X = 0;
            pFilterParms->Algorithm_O = 0;
        }

        CFE_SB_Unsubscribe(PayloadPtr->MessageID, DS_AppData.CmdPipe);

        /*
        ** Notify cFE that we have modified the table data...
        */
        CFE_TBL_Modified(DS_AppData.FilterTblHandle);

        DS_AppData.CmdAcceptedCounter++;

        CFE_EVS_SendEvent(DS_REMOVE_MID_CMD_EID, CFE_EVS_EventType_INFORMATION,
                          "REMOVE MID command: MID = 0x%08lX, filter index = %d, hash index = %d",
                          (unsigned long)CFE_SB_MsgIdToValue(PayloadPtr->MessageID), (int)FilterTableIndex,
                          (int)HashTableIndex);
    }
}

/************************/
/*  End of File Comment */
/************************/
