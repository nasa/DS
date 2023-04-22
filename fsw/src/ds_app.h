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
 *  The CFS Data Storage (DS) Application header file
 */
#ifndef DS_APP_H
#define DS_APP_H

#include "cfe.h"

#include "ds_appdefs.h"

#include "ds_platform_cfg.h"

#include "ds_table.h"

#include "ds_extern_typedefs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS application data structures                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * \brief Wakeup for DS
 *
 * \par Description
 *      Wakes up DS every 1 second for routine maintenance whether a
 *      message was received or not.
 */
#define DS_SB_TIMEOUT       1000
#define DS_SECS_PER_TIMEOUT (DS_SB_TIMEOUT / 1000)

/**
 * \brief Current state of destination files
 */
typedef struct
{
    osal_id_t FileHandle;                       /**< \brief Current file handle */
    uint32    FileAge;                          /**< \brief Current file age in seconds */
    uint32    FileSize;                         /**< \brief Current file size in bytes */
    uint32    FileGrowth;                       /**< \brief Current file growth in bytes (since HK) */
    uint32    FileRate;                         /**< \brief File growth rate in bytes (at last HK) */
    uint32    FileCount;                        /**< \brief Current file sequence count */
    uint16    FileState;                        /**< \brief Current file enable/disable state */
    uint16    Unused;                           /**< \brief Unused - structure padding */
    char      FileName[DS_TOTAL_FNAME_BUFSIZE]; /**< \brief Current filename (path+base+seq+ext) */
} DS_AppFileStatus_t;

/**
 *  \brief DS global data structure definition
 */
typedef struct
{
    CFE_SB_PipeId_t CmdPipe; /**< \brief Pipe Id for DS command pipe */

    CFE_ES_CDSHandle_t DataStoreHandle; /**< \brief Critical Data Store (CDS) handle */

    CFE_TBL_Handle_t FilterTblHandle;   /**< \brief Packet filter table handle */
    CFE_TBL_Handle_t DestFileTblHandle; /**< \brief Destination file table handle */

    DS_FilterTable_t *  FilterTblPtr;   /**< \brief Packet filter table data pointer */
    DS_DestFileTable_t *DestFileTblPtr; /**< \brief Destination file table data pointer */

    uint8 CmdAcceptedCounter;   /**< \brief Count of valid commands received */
    uint8 CmdRejectedCounter;   /**< \brief Count of invalid commands received */
    uint8 DestTblLoadCounter;   /**< \brief Count of destination file table loads */
    uint8 DestTblErrCounter;    /**< \brief Count of failed attempts to get table data pointer */
    uint8 FilterTblLoadCounter; /**< \brief Count of packet filter table loads */
    uint8 FilterTblErrCounter;  /**< \brief Count of failed attempts to get table data pointer */
    uint8 AppEnableState;       /**< \brief Application enable/disable state */
    uint8 Spare8;               /**< \brief Structure alignment padding */

    uint16 FileWriteCounter;     /**< \brief Count of good destination file writes */
    uint16 FileWriteErrCounter;  /**< \brief Count of bad destination file writes */
    uint16 FileUpdateCounter;    /**< \brief Count of good updates to secondary header */
    uint16 FileUpdateErrCounter; /**< \brief Count of bad updates to secondary header */

    uint32 DisabledPktCounter; /**< \brief Count of packets discarded (DS app disabled) */
    uint32 IgnoredPktCounter;  /**< \brief Count of packets discarded (pkt has no filter) */
    uint32 FilteredPktCounter; /**< \brief Count of packets discarded (failed filter test) */
    uint32 PassedPktCounter;   /**< \brief Count of packets that passed filter test */

    DS_AppFileStatus_t FileStatus[DS_DEST_FILE_CNT]; /**< \brief Current state of destination files */

    DS_HashLink_t  HashLinks[DS_PACKETS_IN_FILTER_TABLE]; /**< \brief Hash table linked list elements */
    DS_HashLink_t *HashTable[DS_HASH_TABLE_ENTRIES];      /**< \brief Each hash table entry is a linked list */
} DS_AppData_t;

/** \brief DS global data structure reference */
extern DS_AppData_t DS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Prototypes for functions defined in ds_app.c                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief CFS Data Storage (DS) application entry point
 *
 *  \par Description
 *       DS application entry point and main process loop.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void DS_AppMain(void);

/**
 *  \brief Application initialization function
 *
 *  \par Description
 *       Performs the following startup initialization:
 *       - register DS application for cFE Event Services
 *       - create a cFE Software Bus message pipe
 *       - subscribe to DS commands via message pipe
 *       - register DS filter and file destination tables
 *       - load default filter and file destination tables
 *       - subscribe to packets referenced in DS filter table
 *       - generate startup initialization event message
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t DS_AppInitialize(void);

/**
 *  \brief Application housekeeping request command handler
 *
 *  \par Description
 *       Check with cFE Table Services for table updates
 *       Generate application housekeeping telemetry packet
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_HkPacket_t
 */
void DS_AppSendHkCmd(void);

/**
 *  \brief Application packet storage pre-processor
 *
 *  \par Description
 *       This function verifies that DS storage is enabled and that
 *       both DS tables (filter and file) are loaded before calling
 *       the file storage function (#DS_FileStorePacket).
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MessageID Message ID
 *  \param[in] BufPtr    Software Bus message pointer
 *
 *  \sa #CFE_SB_Buffer_t*
 */
void DS_AppStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr);

#endif
