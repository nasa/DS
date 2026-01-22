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
#include "ds_verify.h"

#include "ds_appdefs.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_dispatch.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_eventids.h"
#include "ds_msgdefs.h"
#include "ds_version.h"

#include <stdio.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application global data structure                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DS_AppData_t DS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application entry point and main process loop                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppMain(void)
{
    CFE_SB_Buffer_t *BufPtr = NULL;
    int32            Result;
    uint32           RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Performance Log (start time counter)...
    */
    CFE_ES_PerfLogEntry(DS_APPMAIN_PERF_ID);

    /*
    ** Perform application-specific initialization...
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
        Result = CFE_SB_ReceiveBuffer(&BufPtr, DS_AppData.CmdPipe, DS_SB_TIMEOUT);

        /*
        ** Performance Log (start time counter)...
        */
        CFE_ES_PerfLogEntry(DS_APPMAIN_PERF_ID);

        /*
        ** Process Software Bus messages...
        */
        if (Result == CFE_SUCCESS)
        {
            DS_AppPipe(BufPtr);
        }
        else if (Result == CFE_SB_TIME_OUT)
        {
            /*
             * Check for table updates.  This is usually done during the
             * housekeeping cycle, but if housekeeping requests are
             * coming at a rate slower than 1Hz, we perform the operations
             * here.
             */
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
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application initialization                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFE_Status_t DS_AppInitialize(void)
{
    CFE_Status_t Result;
    int32        i = 0;

    /*
    ** Initialize global data structure...
    */
    memset(&DS_AppData, 0, sizeof(DS_AppData));

    DS_AppData.AppEnableState  = DS_DEF_ENABLE_STATE;
    DS_AppData.EnableMoveFiles = DS_MOVE_FILES;

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
        CFE_ES_WriteToSysLog("DS App: Error registering for Event Services, RC = 0x%08X\n", (unsigned int)Result);
    }
    else
    {
        Result = CFE_SB_CreatePipe(&DS_AppData.CmdPipe, DS_APP_PIPE_DEPTH, DS_APP_PIPE_NAME);
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
        Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DS_SEND_HK_MID), DS_AppData.CmdPipe);

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
        Result = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DS_CMD_MID), DS_AppData.CmdPipe);

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
        CFE_EVS_SendEvent(DS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Application initialized, version %d.%d.%d.%d, data at %p", DS_MAJOR_VERSION,
                          DS_MINOR_VERSION, DS_REVISION, DS_MISSION_REV, (void *)&DS_AppData);
    }

    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Packet storage pre-processor                                    */
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
}
