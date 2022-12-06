/************************************************************************
 * NASA Docket No. GSC-18,447-1, and identified as “CFS CFDP (CF)
 * Application version 3.0.0”
 *
 * Copyright (c) 2019 United States Government as represented by the
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
 *
 * Declarations and prototypes for ds_extern_typedefs module
 */

#ifndef DS_EXTERN_TYPEDEFS_H
#define DS_EXTERN_TYPEDEFS_H

#include "cfe.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS common application macro definitions                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_UNUSED   0 /**< \brief Unused entries in DS tables */
#define DS_DISABLED 0 /**< \brief Enable/disable state selection */

#define DS_BY_TIME 2 /**< \brief Action is based on packet timestamp */

#define DS_BY_COUNT     1  /**< \brief Action is based on packet sequence count */
#define DS_EMPTY_STRING "" /**< \brief Empty string buffer entries in DS tables */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS application table structures                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

/** \brief DS Filter Table */
typedef struct
{
    char             Descriptor[DS_DESCRIPTOR_BUFSIZE];  /**< \brief Description such as "Safehold Filter Table" */
    DS_PacketEntry_t Packet[DS_PACKETS_IN_FILTER_TABLE]; /**< \brief One entry for each filtered packet */
} DS_FilterTable_t;

/**
 * \brief DS Destination File Table Entry
 *
 * Note that the sum of the string buffer sizes exceeds the max for a qualified filename (OS_MAX_PATH_LEN).
 * This allows a variable definition of how many characters may be used in the pathname versus the filename.
 * The qualified filename length will be verified at run-time as each filename is created.
 */
typedef struct
{
#if (DS_MOVE_FILES == true)
    char Movename[DS_PATHNAME_BUFSIZE]; /**< \brief Move files to this dir after close */
#endif
    char Pathname[DS_PATHNAME_BUFSIZE];   /**< \brief Path portion of filename */
    char Basename[DS_BASENAME_BUFSIZE];   /**< \brief Base portion of filename */
    char Extension[DS_EXTENSION_BUFSIZE]; /**< \brief Extension portion of filename */

    uint16 FileNameType; /**< \brief Filename type - count vs time */
    uint16 EnableState;  /**< \brief File enable/disable state */

    uint32 MaxFileSize; /**< \brief Max file size (bytes) */
    uint32 MaxFileAge;  /**< \brief Max file age (seconds) */

    uint32 SequenceCount; /**< \brief Sequence count portion of filename */
} DS_DestFileEntry_t;

/** \brief DS Destination File Table */
typedef struct
{
    char               Descriptor[DS_DESCRIPTOR_BUFSIZE]; /**< \brief Description such as "HK Telemetry File" */
    DS_DestFileEntry_t File[DS_DEST_FILE_CNT];            /**< \brief One entry for each destination data file */
} DS_DestFileTable_t;

#endif /* DS_EXTERN_TYPEDEFS_H */