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
 *  The CFS Data Storage (DS) Message Definitions
 */
#ifndef DS_MSG_H
#define DS_MSG_H

#include <cfe.h>
#include <ds_platform_cfg.h>

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
 *  \brief Payload containing Ena/Dis State
 *
 *  Used with #DS_AppStateCmd_t
 */
typedef struct
{
    uint16 EnableState; /**< \brief Application enable/disable state */
    uint16 Padding;     /**< \brief Structure Padding on 32-bit boundaries */
} DS_AppState_Payload_t;

/**
 *  \brief Set Ena/Dis State For DS Application
 *
 *  For command details see #DS_SET_APP_STATE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_AppState_Payload_t Payload;
} DS_AppStateCmd_t;

/**
 *  \brief Set File Selection Payload
 *
 *  Used with #DS_FilterFileCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID of existing entry in Packet Filter Table
                                   \details DS defines Message ID zero to be unused */
    uint16 FilterParmsIndex;  /**< \brief Index into Filter Parms Array */
    uint16 FileTableIndex;    /**< \brief Index into Destination File Table */
} DS_FilterFile_Payload_t;

/**
 *  \brief Set File Selection For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterFile_Payload_t Payload;
} DS_FilterFileCmd_t;

/**
 *  \brief Set Filter Type Payload
 *
 *  Used with #DS_FilterTypeCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID of existing entry in Packet Filter Table
                                   \details DS defines Message ID zero to be unused */
    uint16 FilterParmsIndex;  /**< \brief Index into Filter Parms Array */
    uint16 FilterType;        /**< \brief Filter type (packet count or time) */
} DS_FilterType_Payload_t;

/**
 *  \brief Set Filter Type For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_TYPE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterType_Payload_t Payload;
} DS_FilterTypeCmd_t;

/**
 *  \brief Set Filter Parameters Payload
 *
 *  Used with #DS_FilterParmsCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID of existing entry in Packet Filter Table
                                   \details DS defines Message ID zero to be unused */
    uint16 FilterParmsIndex;  /**< \brief Index into Filter Parms Array */
    uint16 Algorithm_N;       /**< \brief Algorithm value N (pass this many) */
    uint16 Algorithm_X;       /**< \brief Algorithm value X (out of this many) */
    uint16 Algorithm_O;       /**< \brief Algorithm value O (at this offset) */
} DS_FilterParms_Payload_t;

/**
 *  \brief Set Filter Parameters For Packet Filter Table Entry
 *
 *  For command details see #DS_SET_FILTER_PARMS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_FilterParms_Payload_t Payload;
} DS_FilterParmsCmd_t;

/**
 *  \brief Set Filename Type Payload
 *
 *  Used with #DS_DestTypeCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 FileNameType;   /**< \brief Filename type - count vs time */
} DS_DestType_Payload_t;

/**
 *  \brief Set Filename Type For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_TYPE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestType_Payload_t Payload;
} DS_DestTypeCmd_t;

/**
 *  \brief Set Ena/Dis State For Destination File Table Entry
 *  \brief Set Filename Type Payload
 *
 *  Used with #DS_DestTypeCmd_t
 *
 *  For command details see #DS_SET_DEST_STATE_CC
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 EnableState;    /**< \brief File enable/disable state */
} DS_DestState_Payload_t;

/**
 *  \brief Set Ena/Dis State For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_STATE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestState_Payload_t Payload;
} DS_DestStateCmd_t;

/**
 *  \brief Set Path Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_DestPathCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                /**< \brief Index into Destination File Table */
    uint16 Padding;                       /**< \brief Structure Padding on 32-bit boundaries */
    char   Pathname[DS_PATHNAME_BUFSIZE]; /**< \brief Path portion of filename */
} DS_DestPath_Payload_t;

/**
 *  \brief Set Path Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_PATH_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestPath_Payload_t Payload;
} DS_DestPathCmd_t;

/**
 *  \brief Set Base Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_DestBaseCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                /**< \brief Index into Destination File Table */
    uint16 Padding;                       /**< \brief Structure Padding on 32-bit boundaries */
    char   Basename[DS_BASENAME_BUFSIZE]; /**< \brief Base portion of filename */
} DS_DestBase_Payload_t;

/**
 *  \brief Set Base Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_BASE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestBase_Payload_t Payload;
} DS_DestBaseCmd_t;

/**
 *  \brief Set Extension Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_DestExtCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                  /**< \brief Index into Destination File Table */
    uint16 Padding;                         /**< \brief Structure Padding on 32-bit boundaries */
    char   Extension[DS_EXTENSION_BUFSIZE]; /**< \brief Extension portion of filename */
} DS_DestExt_Payload_t;

/**
 *  \brief Set Extension Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_EXT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestExt_Payload_t Payload;
} DS_DestExtCmd_t;

/**
 *  \brief Set Max File Size For Destination File Table Payload
 *
 *  Used with #DS_DestSizeCmd_t
 *
 *  For command details see #DS_SET_DEST_SIZE_CC
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */
    uint32 MaxFileSize;    /**< \brief Max file size (bytes) before re-open */
} DS_DestSize_Payload_t;

/**
 *  \brief Set Max File Size For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_SIZE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestSize_Payload_t Payload;
} DS_DestSizeCmd_t;

/**
 *  \brief Set Max File Age For Destination File Table Payload
 *
 *  Used with #DS_DestAgeCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */

    uint32 MaxFileAge; /**< \brief Max file age (seconds) */
} DS_DestAge_Payload_t;

/**
 *  \brief Set Max File Age For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_AGE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestAge_Payload_t Payload;
} DS_DestAgeCmd_t;

/**
 *  \brief Set Sequence Portion Of Filename For Destination File Table Payload
 *
 *  Used with #DS_DestCountCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */

    uint32 SequenceCount; /**< \brief Sequence count portion of filename */
} DS_DestCount_Payload_t;

/**
 *  \brief Set Sequence Portion Of Filename For Destination File Table Entry
 *
 *  For command details see #DS_SET_DEST_COUNT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief cFE Software Bus command message header */

    DS_DestCount_Payload_t Payload;
} DS_DestCountCmd_t;

/**
 *  \brief Close Destination File Payload
 *
 *  Used with #DS_CloseFileCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */
} DS_CloseFile_Payload_t;

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
 *  \brief Message ID Payload
 *
 *  Used with #DS_AddMidCmd_t, #DS_RemoveMidCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID to add to Packet Filter Table */
} DS_AddRemoveMid_Payload_t;

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

typedef struct
{
    uint8  CmdAcceptedCounter;                 /**< \brief Count of valid commands received */
    uint8  CmdRejectedCounter;                 /**< \brief Count of invalid commands received */
    uint8  DestTblLoadCounter;                 /**< \brief Count of destination file table loads */
    uint8  DestTblErrCounter;                  /**< \brief Count of failed attempts to get table data pointer */
    uint8  FilterTblLoadCounter;               /**< \brief Count of packet filter table loads */
    uint8  FilterTblErrCounter;                /**< \brief Count of failed attempts to get table data pointer */
    uint8  AppEnableState;                     /**< \brief Application enable/disable state */
    uint8  Spare8;                             /**< \brief Structure alignment padding */
    uint16 FileWriteCounter;                   /**< \brief Count of good destination file writes */
    uint16 FileWriteErrCounter;                /**< \brief Count of bad destination file writes */
    uint16 FileUpdateCounter;                  /**< \brief Count of good updates to secondary header */
    uint16 FileUpdateErrCounter;               /**< \brief Count of bad updates to secondary header */
    uint32 DisabledPktCounter;                 /**< \brief Count of packets discarded (DS was disabled) */
    uint32 IgnoredPktCounter;                  /**< \brief Count of packets discarded
                                                *
                                                * Incoming packets will be discarded when:
                                                *  - The File and/or Filter Table has failed to load
                                                *  - A packet (that is not a DS HK or command packet) has been received
                                                *    that is not listed in the Filter Table
                                                */
    uint32 FilteredPktCounter;                 /**< \brief Count of packets discarded (failed filter test) */
    uint32 PassedPktCounter;                   /**< \brief Count of packets that passed filter test */
    char   FilterTblFilename[OS_MAX_PATH_LEN]; /**< \brief Name of filter table file */
} DS_HkTlm_Payload_t;

/**
 * \brief Application housekeeping packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief cFE Software Bus telemetry message header */

    DS_HkTlm_Payload_t Payload;
} DS_HkPacket_t;

/**
 * \brief Current state of destination files
 */
typedef struct
{
    uint32 FileAge;                          /**< \brief Current file age in seconds */
    uint32 FileSize;                         /**< \brief Current file size in bytes */
    uint32 FileRate;                         /**< \brief Current file data rate (avg since HK) */
    uint32 SequenceCount;                    /**< \brief Sequence count portion of filename */
    uint16 EnableState;                      /**< \brief Current file enable/disable state */
    uint16 OpenState;                        /**< \brief Current file open/close state */
    char   FileName[DS_TOTAL_FNAME_BUFSIZE]; /**< \brief Current filename (path+base+seq+ext) */
} DS_FileInfo_t;

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

/**
 * \brief Single application file info packet buffer
 *
 * This typedef supports CFE_SB_AllocateMessageBuffer use with a DS_FileCompletePkt_t
 * that compiles with the alignment constraints of a CFE_SB_Buffer_t
 */
typedef union
{
    CFE_SB_Buffer_t      SBBuf; /**< \brief Message buffer for alignment */
    DS_FileCompletePkt_t Pkt;   /**< \brief Single application file info packet */
} DS_FileCompletePktBuf_t;

/**\}*/

#endif
