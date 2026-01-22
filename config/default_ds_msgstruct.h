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
 *  The CFS Data Storage (DS) command and telemetry
 *   messages.
 */
#ifndef DEFAULT_DS_MSGSTRUCT_H
#define DEFAULT_DS_MSGSTRUCT_H

#include "ds_mission_cfg.h"
#include "ds_msgdefs.h"
#include "cfe_msg_hdr.h"

/**
 * \defgroup cfsdscmdstructs CFS Data Storage Command Structures
 * \{
 */

/**
 *  \brief Send HK Request
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */
} DS_SendHkCmd_t;

/**
 *  \brief No-Operation Command
 *
 *  For command details see #DS_NOOP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */
} DS_NoopCmd_t;

/**
 *  \brief Reset Housekeeping Telemetry Command
 *
 *  For command details see #DS_RESET_COUNTERS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */
} DS_ResetCountersCmd_t;

/**
 *  \brief Set Ena/Dis State For DS Application
 *
 *  For command details see #DS_SET_APP_STATE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_AppState_Payload_t Payload;
} DS_SetAppStateCmd_t;

/**
 *  \brief Set File Selection For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterFile_Payload_t Payload;
} DS_SetFilterFileCmd_t;

/**
 *  \brief Set Filter Type For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_TYPE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterType_Payload_t Payload;
} DS_SetFilterTypeCmd_t;

/**
 *  \brief Set Filter Parameters For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_PARMS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterParms_Payload_t Payload;
} DS_SetFilterParmsCmd_t;

/**
 *  \brief Set Filename Type For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_TYPE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestType_Payload_t Payload;
} DS_SetDestTypeCmd_t;

/**
 *  \brief Set Ena/Dis State For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_STATE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestState_Payload_t Payload;
} DS_SetDestStateCmd_t;

/**
 *  \brief Set Path Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_PATH_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestPath_Payload_t Payload;
} DS_SetDestPathCmd_t;

/**
 *  \brief Set Base Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_BASE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestBase_Payload_t Payload;
} DS_SetDestBaseCmd_t;

/**
 *  \brief Set Extension Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_EXT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestExt_Payload_t Payload;
} DS_SetDestExtCmd_t;

/**
 *  \brief Set Max File Size For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_SIZE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestSize_Payload_t Payload;
} DS_SetDestSizeCmd_t;

/**
 *  \brief Set Max File Age For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_AGE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestAge_Payload_t Payload;
} DS_SetDestAgeCmd_t;

/**
 *  \brief Set Sequence Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_COUNT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestCount_Payload_t Payload;
} DS_SetDestCountCmd_t;

/**
 *  \brief Close Destination File
 *
 *  For command details see #DS_CLOSE_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_CloseFile_Payload_t Payload;
} DS_CloseFileCmd_t;

/**
 *  \brief Close All Destination Files
 *
 *  For command details see #DS_CLOSE_ALL_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */
} DS_CloseAllCmd_t;

/**
 *  \brief Get File Info Command
 *
 *  For command details see #DS_GET_FILE_INFO_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */
} DS_GetFileInfoCmd_t;

/**
 *  \brief Add Message ID To Packet Filter Table
 *
 *  For command details see #DS_ADD_MID_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_AddRemoveMid_Payload_t Payload;
} DS_AddMidCmd_t;

/**
 *  \brief Remove Message ID from Packet Filter Table
 *
 *  For command details see #DS_REMOVE_MID_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_AddRemoveMid_Payload_t Payload;
} DS_RemoveMidCmd_t;

/**\}*/

/**
 * \defgroup cfsdstlm CFS Data Storage Telemetry
 * \{
 */

/**
 * \brief Application housekeeping packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief cFE Software Bus telemetry message header */

    DS_HkTlm_Payload_t Payload;
} DS_HkPacket_t;

/**
 * \brief Application file info packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief cFE Software Bus telemetry message header */

    DS_FileInfo_t Payload[DS_DEST_FILE_CNT]; /**< \brief Current state of destination files */
} DS_FileInfoPkt_t;

/**
 * \brief Single application file info packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief cFE Software Bus telemetry message header */

    DS_FileInfo_t Payload; /**< \brief Current state of destination file */
} DS_FileCompletePkt_t;

/**\}*/

#endif
