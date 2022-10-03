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
 *  The CFS Data Storage (DS) Application file containing the application
 *  initialization routines, the main routine and the command interface.
 */

#include "cfe.h"

#include "ds_perfids.h"
#include "ds_msgids.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"
#include "ds_msgdefs.h"
#include "ds_version.h"

#include <stdio.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppData -- application global data structure                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DS_AppData_t DS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppMain() -- application entry point and main process loop   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppMain(void)
{
    CFE_SB_Buffer_t *BufPtr    = NULL;
    int32            Result    = CFE_SUCCESS;
    uint32           RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Performance Log (start time counter)...
    */
    CFE_ES_PerfLogEntry(DS_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization...
    */
    Result = DS_AppInitialize();

    /*
    ** Check for start-up error...
    */
    if (Result != CFE_SUCCESS)
    {
        /*
        ** Set request to terminate main loop...
        */
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main process loop...
    */
    while (CFE_ES_RunLoop(&RunStatus))
    {
        /*
        ** Performance Log (stop time counter)...
        */
        CFE_ES_PerfLogExit(DS_APPMAIN_PERF_ID);

        /*
        ** Wait for next Software Bus message...
        */
        Result = CFE_SB_ReceiveBuffer(&BufPtr, DS_AppData.InputPipe, DS_SB_TIMEOUT);

        /*
        ** Performance Log (start time counter)...
        */
        CFE_ES_PerfLogEntry(DS_APPMAIN_PERF_ID);

        /*
        ** Process Software Bus messages...
        */
        if (Result == CFE_SUCCESS)
        {
            DS_AppProcessMsg(BufPtr);
        }
        else if (Result == CFE_SB_TIME_OUT)
        {
            /*
             * Process data storage file age limits and check for
             * table updates.  This is usually done during the
             * housekeeping cycle, but if housekeeping requests are
             * coming at a rate slower than 1Hz, we perform the operations
             * here.
             */
            DS_FileTestAge(DS_SECS_PER_TIMEOUT);
            DS_TableManageDestFile();
            DS_TableManageFilter();
        }
        else
        {
            /*
            ** Set request to terminate main loop...
            */
            RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }

        /*
        ** Note: If there were some reason to exit normally
        **       (without error) then we would set
        **       RunStatus = CFE_ES_APP_EXIT
        */
    }

    /*
    ** Check for "fatal" process error...
    */
    if (Result != CFE_SUCCESS)
    {
        /*
        ** Send an event describing the reason for the termination...
        */
        CFE_EVS_SendEvent(DS_EXIT_ERR_EID, CFE_EVS_EventType_CRITICAL, "Application terminating, err = 0x%08X",
                          (unsigned int)Result);

        /*
        ** In case cFE Event Services is not working...
        */
        CFE_ES_WriteToSysLog("DS application terminating, err = 0x%08X\n", (unsigned int)Result);
    }

    /*
    ** Performance Log (stop time counter)...
    */
    CFE_ES_PerfLogExit(DS_APPMAIN_PERF_ID);

    /*
    ** Let cFE kill the application (and any child tasks)...
    */
    CFE_ES_ExitApp(RunStatus);

} /* End of DS_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppInitialize() -- application initialization                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_AppInitialize(void)
{
    int32 Result = CFE_SUCCESS;
    int32 i      = 0;

    /*
    ** Initialize global data structure...
    */
    memset(&DS_AppData, 0, sizeof(DS_AppData));

    DS_AppData.AppEnableState = DS_DEF_ENABLE_STATE;

    /*
    ** Mark files as closed
    */
    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    /*
    ** Initialize interface to cFE Event Services...
    */
    Result = CFE_EVS_Register(NULL, 0, 0);

    if (Result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(DS_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "Unable to register for EVS services, err = 0x%08X",
                          (unsigned int)Result);
    }

    /*
    ** Create application Software Bus message pipe...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_CreatePipe(&DS_AppData.InputPipe, DS_APP_PIPE_DEPTH, DS_APP_PIPE_NAME);
        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(DS_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "Unable to create input pipe, err = 0x%08X",
                              (unsigned int)Result);
        }
    }

    /*
    ** Subscribe to application housekeeping request command...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DS_SEND_HK_MID), DS_AppData.InputPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(DS_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Unable to subscribe to HK request, err = 0x%08X", (unsigned int)Result);
        }
    }

    /*
    ** Subscribe to application commands...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_AppData.InputPipe);

        if (Result != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(DS_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Unable to subscribe to DS commands, err = 0x%08X", (unsigned int)Result);
        }
    }

    /*
    ** Initialize application tables...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = DS_TableInit();
    }

    /*
    ** Initialize access to Critical Data Store (CDS)...
    */
    if (Result == CFE_SUCCESS)
    {
        Result = DS_TableCreateCDS();
    }

    /*
    ** Generate application startup event message...
    */
    if (Result == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(DS_INIT_EID, CFE_EVS_EventType_INFORMATION,
                          "Application initialized, version %d.%d.%d.%d, data at %p", DS_MAJOR_VERSION,
                          DS_MINOR_VERSION, DS_REVISION, DS_MISSION_REV, (void *)&DS_AppData);
    }

    return Result;

} /* End of DS_AppInitialize() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppProcessMsg() -- process Software Bus messages             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessMsg(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_SB_MsgId_t MessageID      = CFE_SB_INVALID_MSG_ID;
    size_t         ActualLength   = 0;
    size_t         ExpectedLength = 0;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);

    switch (CFE_SB_MsgIdToValue(MessageID))
    {
        /*
        ** DS application commands...
        */
        case DS_CMD_MID:
            DS_AppProcessCmd(BufPtr);
            if (DS_TableFindMsgID(MessageID) != DS_INDEX_NONE)
            {
                DS_AppStorePacket(MessageID, BufPtr);
            }
            break;

        /*
        ** DS housekeeping request command...
        */
        case DS_SEND_HK_MID:

            CFE_MSG_GetSize(&BufPtr->Msg, &ActualLength);
            ExpectedLength = sizeof(DS_NoopCmd_t);
            if (ExpectedLength != ActualLength)
            {
                CFE_EVS_SendEvent(DS_HK_REQUEST_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Invalid HK request length: expected = %d, actual = %d", (int)ExpectedLength,
                                  (int)ActualLength);
            }
            else
            {
                DS_AppProcessHK();
                if (DS_TableFindMsgID(MessageID) != DS_INDEX_NONE)
                {
                    DS_AppStorePacket(MessageID, BufPtr);
                }
            }
            break;

        /*
        ** Unknown message ID's (must be something to store)...
        */
        default:
            DS_AppStorePacket(MessageID, BufPtr);
            break;
    }

    return;

} /* End of DS_AppProcessMsg() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppProcessCmd() -- process application commands              */
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
            DS_CmdNoop(BufPtr);
            break;

        /*
        ** Set housekeeping telemetry counters to zero...
        */
        case DS_RESET_CC:
            DS_CmdReset(BufPtr);
            break;

        /*
        ** Set DS application enable/disable state...
        */
        case DS_SET_APP_STATE_CC:
            DS_CmdSetAppState(BufPtr);
            break;

        /*
        ** Set packet filter file index...
        */
        case DS_SET_FILTER_FILE_CC:
            DS_CmdSetFilterFile(BufPtr);
            break;

        /*
        ** Set packet filter type (time vs count)...
        */
        case DS_SET_FILTER_TYPE_CC:
            DS_CmdSetFilterType(BufPtr);
            break;

        /*
        ** Set packet filter algorithm parameters...
        */
        case DS_SET_FILTER_PARMS_CC:
            DS_CmdSetFilterParms(BufPtr);
            break;

        /*
        ** Set destination file filename type (time vs count)...
        */
        case DS_SET_DEST_TYPE_CC:
            DS_CmdSetDestType(BufPtr);
            break;

        /*
        ** Set destination file enable/disable state...
        */
        case DS_SET_DEST_STATE_CC:
            DS_CmdSetDestState(BufPtr);
            break;

        /*
        ** Set destination file path portion of filename...
        */
        case DS_SET_DEST_PATH_CC:
            DS_CmdSetDestPath(BufPtr);
            break;

        /*
        ** Set destination file base portion of filename...
        */
        case DS_SET_DEST_BASE_CC:
            DS_CmdSetDestBase(BufPtr);
            break;

        /*
        ** Set destination file extension portion of filename...
        */
        case DS_SET_DEST_EXT_CC:
            DS_CmdSetDestExt(BufPtr);
            break;

        /*
        ** Set destination file maximum size limit...
        */
        case DS_SET_DEST_SIZE_CC:
            DS_CmdSetDestSize(BufPtr);
            break;

        /*
        ** Set destination file maximum age limit...
        */
        case DS_SET_DEST_AGE_CC:
            DS_CmdSetDestAge(BufPtr);
            break;

        /*
        ** Set destination file sequence count portion of filename...
        */
        case DS_SET_DEST_COUNT_CC:
            DS_CmdSetDestCount(BufPtr);
            break;

        /*
        ** Close destination file (next packet will re-open)...
        */
        case DS_CLOSE_FILE_CC:
            DS_CmdCloseFile(BufPtr);
            break;

        /*
        ** Get file info telemetry packet...
        */
        case DS_GET_FILE_INFO_CC:
            DS_CmdGetFileInfo(BufPtr);
            break;

        /*
        ** Add message ID to filter table...
        */
        case DS_ADD_MID_CC:
            DS_CmdAddMID(BufPtr);
            break;

        /*
        ** Close all destination files (next packet will re-open)...
        */
        case DS_CLOSE_ALL_CC:
            DS_CmdCloseAll(BufPtr);
            break;

        /*
        ** DS application command with unknown command code...
        */
        default:
            CFE_EVS_SendEvent(DS_CMD_CODE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid command code: MID = 0x%08X, CC = %d", DS_CMD_MID, CommandCode);

            DS_AppData.CmdRejectedCounter++;
            break;
    }

    return;

} /* End of DS_AppProcessCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppProcessHK() -- process hk request command                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessHK(void)
{
    DS_HkPacket_t  HkPacket;
    int32          i                                              = 0;
    int32          Status                                         = 0;
    char           FilterTblName[CFE_MISSION_TBL_MAX_NAME_LENGTH] = {0};
    CFE_TBL_Info_t FilterTblInfo;

    memset(&HkPacket, 0, sizeof(HkPacket));

    /*
    ** Initialize housekeeping packet...
    */
    CFE_MSG_Init(&HkPacket.TlmHeader.Msg, CFE_SB_ValueToMsgId(DS_HK_TLM_MID), sizeof(DS_HkPacket_t));

    /*
    ** Process data storage file age limits...
    */
    DS_FileTestAge(DS_SECS_PER_HK_CYCLE);

    /*
    ** Take this opportunity to check for table updates...
    */
    DS_TableManageDestFile();
    DS_TableManageFilter();

    /*
    ** Copy application command counters to housekeeping telemetry packet...
    */
    HkPacket.CmdAcceptedCounter = DS_AppData.CmdAcceptedCounter;
    HkPacket.CmdRejectedCounter = DS_AppData.CmdRejectedCounter;

    /*
    ** Copy packet storage counters to housekeeping telemetry packet...
    */
    HkPacket.DisabledPktCounter = DS_AppData.DisabledPktCounter;
    HkPacket.IgnoredPktCounter  = DS_AppData.IgnoredPktCounter;
    HkPacket.FilteredPktCounter = DS_AppData.FilteredPktCounter;
    HkPacket.PassedPktCounter   = DS_AppData.PassedPktCounter;

    /*
    ** Copy file I/O counters to housekeeping telemetry packet...
    */
    HkPacket.FileWriteCounter     = DS_AppData.FileWriteCounter;
    HkPacket.FileWriteErrCounter  = DS_AppData.FileWriteErrCounter;
    HkPacket.FileUpdateCounter    = DS_AppData.FileUpdateCounter;
    HkPacket.FileUpdateErrCounter = DS_AppData.FileUpdateErrCounter;

    /*
    ** Copy configuration table counters to housekeeping telemetry packet...
    */
    HkPacket.DestTblLoadCounter   = DS_AppData.DestTblLoadCounter;
    HkPacket.DestTblErrCounter    = DS_AppData.DestTblErrCounter;
    HkPacket.FilterTblLoadCounter = DS_AppData.FilterTblLoadCounter;
    HkPacket.FilterTblErrCounter  = DS_AppData.FilterTblErrCounter;

    /*
    ** Copy app enable/disable state to housekeeping telemetry packet...
    */
    HkPacket.AppEnableState = DS_AppData.AppEnableState;

    /*
    ** Compute file growth rate from number of bytes since last HK request...
    */
    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileRate   = DS_AppData.FileStatus[i].FileGrowth / DS_SECS_PER_HK_CYCLE;
        DS_AppData.FileStatus[i].FileGrowth = 0;
    }

    /* Get the filter table info, put the file name in the hk pkt. */
    Status = snprintf(FilterTblName, CFE_MISSION_TBL_MAX_NAME_LENGTH, "DS.%s", DS_FILTER_TBL_NAME);
    if (Status >= 0)
    {
        Status = CFE_TBL_GetInfo(&FilterTblInfo, FilterTblName);
        if (Status == CFE_SUCCESS)
        {
            strncpy(HkPacket.FilterTblFilename, FilterTblInfo.LastFileLoaded, OS_MAX_PATH_LEN - 1);
            HkPacket.FilterTblFilename[OS_MAX_PATH_LEN - 1] = '\0';
        }

        else
        {
            /* If the filter table name is invalid, send an event and erase any
             * stale/misleading filename from the HK packet */
            CFE_EVS_SendEvent(DS_APPHK_FILTER_TBL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid filter tbl name in DS_AppProcessHK. Name=%s, Err=0x%08X", FilterTblName, Status);

            memset(HkPacket.FilterTblFilename, 0, sizeof(HkPacket.FilterTblFilename));
        }
    }
    else
    {
        /* If the filter table name couldn't be copied, send an event and erase
         * any stale/misleading filename from the HK packet */
        CFE_EVS_SendEvent(DS_APPHK_FILTER_TBL_PRINT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Filter tbl name copy fail in DS_AppProcessHK. Err=%d", (int)Status);

        memset(HkPacket.FilterTblFilename, 0, sizeof(HkPacket.FilterTblFilename));
    }

    /*
    ** Timestamp and send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&HkPacket.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&HkPacket.TlmHeader.Msg, true);

    return;

} /* End of DS_AppProcessHK() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_AppStorePacket() -- packet storage pre-processor             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{

    if (DS_AppData.AppEnableState == DS_DISABLED)
    {
        /*
        ** Application must be enabled in order to store data...
        */
        DS_AppData.DisabledPktCounter++;
    }
    else if ((DS_AppData.FilterTblPtr == (DS_FilterTable_t *)NULL) ||
             (DS_AppData.DestFileTblPtr == (DS_DestFileTable_t *)NULL))
    {
        /*
        ** Must have both tables loaded in order to store data...
        */
        DS_AppData.IgnoredPktCounter++;
    }
    else
    {
        /*
        ** Store packet (if permitted by filter table)...
        */
        DS_FileStorePacket(MessageID, BufPtr);
    }

    return;

} /* End of DS_AppStorePacket() */

/************************/
/*  End of File Comment */
/************************/
