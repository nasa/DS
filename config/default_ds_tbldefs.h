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
 *  The CFS Data Storage (DS) tables.
 */
#ifndef DEFAULT_DS_TBLDEFS_H
#define DEFAULT_DS_TBLDEFS_H

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/
#include "cfe_mission_cfg.h"
#include "ds_mission_cfg.h"
#include "cfe_es_extern_typedefs.h"
#include "cfe_sb_api_typedefs.h"
#include "ds_extern_typedefs.h"

/**************************************************************************
 **
 ** Macro definitions
 **
 **************************************************************************/

#define DS_UNUSED   0 /**< \brief Unused entries in DS tables */
#define DS_DISABLED 0 /**< \brief Enable/disable state selection */
#define DS_ENABLED  1 /**< \brief Enable/disable state selection */

#define DS_BY_TIME  2 /**< \brief Action is based on packet timestamp */
#define DS_BY_COUNT 1 /**< \brief Action is based on packet sequence count */

/**************************************************************************
 **
 ** Type definitions
 **
 **************************************************************************/

/** \brief DS Packet Filter Parameters */
typedef struct
{
    uint8 FileTableIndex; /**< \brief Index into Destination File Table */
    uint8 FilterType;     /**< \brief Filter type (packet count or time) */

    uint16 Algorithm_N; /**< \brief Algorithm value N (pass this many) */
    uint16 Algorithm_X; /**< \brief Algorithm value X (out of this many) */
    uint16 Algorithm_O; /**< \brief Algorithm value O (at this offset) */
} DS_FilterParms_t;

/** \brief DS Filter Table Packet Entry */
typedef struct
{
    CFE_SB_MsgId_t MessageID; /**< \brief Packet MessageID (may be cmd or tlm) */

    DS_FilterParms_t Filter[DS_FILTERS_PER_PACKET]; /**< \brief One entry for each packet destination */
} DS_PacketEntry_t;

/**
 * \brief DS Destination File Table Entry
 *
 * Note that the sum of the string buffer sizes exceeds the max for a qualified filename (OS_MAX_PATH_LEN).
 * This allows a variable definition of how many characters may be used in the pathname versus the filename.
 * The qualified filename length will be verified at run-time as each filename is created.
 */
typedef struct
{
    char Movename[DS_PATHNAME_BUFSIZE];   /**< \brief Move files to this dir after close */
    char Pathname[DS_PATHNAME_BUFSIZE];   /**< \brief Path portion of filename */
    char Basename[DS_BASENAME_BUFSIZE];   /**< \brief Base portion of filename */
    char Extension[DS_EXTENSION_BUFSIZE]; /**< \brief Extension portion of filename */

    uint16 FileNameType; /**< \brief Filename type - count vs time */
    uint16 EnableState;  /**< \brief File enable/disable state */

    uint32 MaxFileSize; /**< \brief Max file size (bytes) */
    uint32 MaxFileAge;  /**< \brief Max file age (seconds) */

    uint32 SequenceCount; /**< \brief Sequence count portion of filename */
} DS_DestFileEntry_t;

#endif
