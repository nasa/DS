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
 *  The CFS Data Storage (DS) Application Message IDs header file
 */
#ifndef DEFAULT_DS_TBLSTRUCT_H
#define DEFAULT_DS_TBLSTRUCT_H

#include "ds_mission_cfg.h"
#include "ds_tbldefs.h"

/** \brief DS Filter Table */
typedef struct
{
    char             Descriptor[DS_DESCRIPTOR_BUFSIZE];  /**< \brief Description such as "Safehold Filter Table" */
    DS_PacketEntry_t Packet[DS_PACKETS_IN_FILTER_TABLE]; /**< \brief One entry for each filtered packet */
} DS_FilterTable_t;

/** \brief DS Destination File Table */
typedef struct
{
    char               Descriptor[DS_DESCRIPTOR_BUFSIZE]; /**< \brief Description such as "HK Telemetry File" */
    DS_DestFileEntry_t File[DS_DEST_FILE_CNT];            /**< \brief One entry for each destination data file */
} DS_DestFileTable_t;

#endif
