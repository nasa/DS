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
#ifndef DEFAULT_DS_MSGDEFS_H
#define DEFAULT_DS_MSGDEFS_H

#include "common_types.h"
#include "cfe_mission_cfg.h"
#include "ds_mission_cfg.h"
#include "cfe_sb_extern_typedefs.h"

/**
 * \defgroup cfsdscmdpayloads CFS Data Storage Command Payload Structures
 * \{
 */

/**
 *  \brief Payload containing Ena/Dis State
 *
 *  Used with #DS_SetAppStateCmd_t
 */
typedef struct
{
    uint16 EnableState; /**< \brief Application enable/disable state */
    uint16 Padding;     /**< \brief Structure Padding on 32-bit boundaries */
} DS_AppState_Payload_t;

/**
 *  \brief Set File Selection Payload
 *
 *  Used with #DS_SetFilterFileCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID of existing entry in Packet Filter Table
                                   \details DS defines Message ID zero to be unused */
    uint16 FilterParmsIndex;  /**< \brief Index into Filter Parms Array */
    uint16 FileTableIndex;    /**< \brief Index into Destination File Table */
} DS_FilterFile_Payload_t;

/**
 *  \brief Set Filter Type Payload
 *
 *  Used with #DS_SetFilterTypeCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID of existing entry in Packet Filter Table
                                   \details DS defines Message ID zero to be unused */
    uint16 FilterParmsIndex;  /**< \brief Index into Filter Parms Array */
    uint16 FilterType;        /**< \brief Filter type (packet count or time) */
} DS_FilterType_Payload_t;

/**
 *  \brief Set Filter Parameters Payload
 *
 *  Used with #DS_SetFilterParmsCmd_t
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
 *  \brief Set Filename Type Payload
 *
 *  Used with #DS_SetDestTypeCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 FileNameType;   /**< \brief Filename type - count vs time */
} DS_DestType_Payload_t;

/**
 *  \brief Set Ena/Dis State For Destination File Table Entry
 *  \brief Set Filename Type Payload
 *
 *  Used with #DS_SetDestTypeCmd_t
 *
 *  For command details see #DS_SET_DEST_STATE_CC
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 EnableState;    /**< \brief File enable/disable state */
} DS_DestState_Payload_t;

/**
 *  \brief Set Path Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_SetDestPathCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                     /**< \brief Index into Destination File Table */
    uint16 Padding;                            /**< \brief Structure Padding on 32-bit boundaries */
    char   Pathname[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Path portion of filename */
} DS_DestPath_Payload_t;

/**
 *  \brief Set Base Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_SetDestBaseCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                     /**< \brief Index into Destination File Table */
    uint16 Padding;                            /**< \brief Structure Padding on 32-bit boundaries */
    char   Basename[CFE_MISSION_MAX_FILE_LEN]; /**< \brief Base portion of filename */
} DS_DestBase_Payload_t;

/**
 *  \brief Set Extension Portion Of Filename For Destination File Payload
 *
 *  Used with #DS_SetDestExtCmd_t
 */
typedef struct
{
    uint16 FileTableIndex;                  /**< \brief Index into Destination File Table */
    uint16 Padding;                         /**< \brief Structure Padding on 32-bit boundaries */
    char   Extension[DS_EXTENSION_BUFSIZE]; /**< \brief Extension portion of filename */
} DS_DestExt_Payload_t;

/**
 *  \brief Set Max File Size For Destination File Table Payload
 *
 *  Used with #DS_SetDestSizeCmd_t
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
 *  \brief Set Max File Age For Destination File Table Payload
 *
 *  Used with #DS_SetDestAgeCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */

    uint32 MaxFileAge; /**< \brief Max file age (seconds) */
} DS_DestAge_Payload_t;

/**
 *  \brief Set Sequence Portion Of Filename For Destination File Table Payload
 *
 *  Used with #DS_SetDestCountCmd_t
 */
typedef struct
{
    uint16 FileTableIndex; /**< \brief Index into Destination File Table */
    uint16 Padding;        /**< \brief Structure Padding on 32-bit boundaries */

    uint32 SequenceCount; /**< \brief Sequence count portion of filename */
} DS_DestCount_Payload_t;

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
 *  \brief Message ID Payload
 *
 *  Used with #DS_AddMidCmd_t, #DS_RemoveMidCmd_t
 */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Message ID to add to Packet Filter Table */
} DS_AddRemoveMid_Payload_t;

/**\}*/

/**
 * \defgroup cfsdstlm CFS Data Storage Telemetry
 * \{
 */

typedef struct
{
    uint8  CmdAcceptedCounter;   /**< \brief Count of valid commands received */
    uint8  CmdRejectedCounter;   /**< \brief Count of invalid commands received */
    uint8  DestTblLoadCounter;   /**< \brief Count of destination file table loads */
    uint8  DestTblErrCounter;    /**< \brief Count of failed attempts to get table data pointer */
    uint8  FilterTblLoadCounter; /**< \brief Count of packet filter table loads */
    uint8  FilterTblErrCounter;  /**< \brief Count of failed attempts to get table data pointer */
    uint8  AppEnableState;       /**< \brief Application enable/disable state */
    uint8  Spare8;               /**< \brief Structure alignment padding */
    uint16 FileWriteCounter;     /**< \brief Count of good destination file writes */
    uint16 FileWriteErrCounter;  /**< \brief Count of bad destination file writes */
    uint16 FileUpdateCounter;    /**< \brief Count of good updates to secondary header */
    uint16 FileUpdateErrCounter; /**< \brief Count of bad updates to secondary header */
    uint32 DisabledPktCounter;   /**< \brief Count of packets discarded (DS was disabled) */
    uint32 IgnoredPktCounter;    /**< \brief Count of packets discarded
                                  *
                                  * Incoming packets will be discarded when:
                                  *  - The File and/or Filter Table has failed to load
                                  *  - A packet (that is not a DS HK or command packet) has been received
                                  *    that is not listed in the Filter Table
                                  */
    uint32 FilteredPktCounter;   /**< \brief Count of packets discarded (failed filter test) */
    uint32 PassedPktCounter;     /**< \brief Count of packets that passed filter test */
    char   FilterTblFilename[CFE_MISSION_MAX_PATH_LEN]; /**< \brief Name of filter table file */
} DS_HkTlm_Payload_t;

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

/**\}*/

#endif
