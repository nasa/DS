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
 *  The CFS Data Storage (DS) Application interface configuration header file
 */
#ifndef DS_INTERFACE_CFG_H
#define DS_INTERFACE_CFG_H

#include "ds_interface_cfg_values.h"

/**
 * \defgroup cfsdsmissioncfg CFS Data Storage Mission Configuration
 * \{
 */

/**
 *  \brief Destination File Table -- number of files
 *
 *  \par Description:
 *       This parameter defines the size of the DS Destination File
 *       Table by setting the number of file entries in the table.  The
 *       number should be large enough to provide an entry for all the
 *       destination files defined for the project.  Maintenance will
 *       be simplified if file index 'n' always describes the same
 *       file - even if that file is not in use at the present time.
 *
 *  \par Limits:
 *       The number must be greater than zero but there is no upper
 *       enforced limit for this parameter.
 */
#define DS_DEST_FILE_CNT                   DS_INTERFACE_CFGVAL(DEST_FILE_CNT)
#define DEFAULT_DS_INTERFACE_DEST_FILE_CNT 16

/**
 *  \brief Destination File Table -- pathname buffer size
 *
 *  \par Description:
 *       This parameter further defines the size of the Destination
 *       File Table by setting the size of the pathname buffer for
 *       each file entry.  Note that the buffer must contain both
 *       the string and the string terminator - so the max string
 *       length is one less than the buffer size.
 *
 *  \par Limits:
 *       The buffer size must be greater than zero and a multiple
 *       of four bytes for alignment.  The value cannot exceed the
 *       maximum filename size allowed by the OS (#OS_MAX_PATH_LEN).
 */
#define DS_PATHNAME_BUFSIZE                   DS_INTERFACE_CFGVAL(PATHNAME_BUFSIZE)
#define DEFAULT_DS_INTERFACE_PATHNAME_BUFSIZE CFE_MISSION_MAX_PATH_LEN

/**
 *  \brief Data Storage File -- total filename size
 *
 *  \par Description:
 *       This parameter defines the maximum size of a filename after
 *       combining the pathname, basename, sequence and extension.
 *
 *  \par Limits:
 *       The buffer size must be greater than zero and a multiple
 *       of four bytes for alignment.  The buffer size (including
 *       string terminator) cannot exceed #CFE_MISSION_MAX_PATH_LEN.
 */
#define DS_TOTAL_FNAME_BUFSIZE                   DS_INTERFACE_CFGVAL(TOTAL_FNAME_BUFSIZE)
#define DEFAULT_DS_INTERFACE_TOTAL_FNAME_BUFSIZE CFE_MISSION_MAX_PATH_LEN

/**
 *  \brief Destination File Table -- basename buffer size
 *
 *  \par Description:
 *       This parameter further defines the size of the Destination
 *       File Table by setting the size of the basename buffer for
 *       each file entry.  Note that the buffer must contain both
 *       the string and the string terminator - so the max string
 *       length is one less than the buffer size.
 *
 *  \par Limits:
 *       The buffer size must be greater than zero and a multiple
 *       of four bytes for alignment.  The value cannot exceed the
 *       maximum filename size allowed by the OS (#OS_MAX_PATH_LEN).
 */
#define DS_BASENAME_BUFSIZE                   DS_INTERFACE_CFGVAL(BASENAME_BUFSIZE)
#define DEFAULT_DS_INTERFACE_BASENAME_BUFSIZE CFE_MISSION_MAX_FILE_LEN

/**
 *  \brief Destination File Table -- extension buffer size
 *
 *  \par Description:
 *       This parameter further defines the size of the Destination
 *       File Table by setting the size of the extension buffer for
 *       each file entry.  Note that the buffer must contain both
 *       the string and the string terminator - so the max string
 *       length is one less than the buffer size.
 *
 *  \par Limits:
 *       The buffer size must be greater than zero and a multiple
 *       of four bytes for alignment.  The value cannot exceed the
 *       maximum filename size allowed by the OS (#OS_MAX_PATH_LEN).
 */
#define DS_EXTENSION_BUFSIZE                   DS_INTERFACE_CFGVAL(EXTENSION_BUFSIZE)
#define DEFAULT_DS_INTERFACE_EXTENSION_BUFSIZE 8

/**
 *  \brief Packet Filter Table -- number of packets
 *
 *  \par Description:
 *       This parameter defines the size of the DS Packet Filter Table
 *       by setting the number of packet entries in the table.  The
 *       number should be large enough to provide an entry for each
 *       command and telemetry packet subject to data storage.
 *
 *  \par Limits:
 *       The number must be greater than zero but there is no upper
 *       enforced limit for this parameter.  However, setting the
 *       size equal to the number of packets that might be subject
 *       to data storage, rather than the total number of packets
 *       defined for this project, will reduce the table file size,
 *       possibly significantly.
 */
#define DS_PACKETS_IN_FILTER_TABLE                   DS_INTERFACE_CFGVAL(PACKETS_IN_FILTER_TABLE)
#define DEFAULT_DS_INTERFACE_PACKETS_IN_FILTER_TABLE 256

/**
 *  \brief Packet Filter Table -- filters per packet
 *
 *  \par Description:
 *       This parameter further defines the size of the DS Packet
 *       Filter Table by setting the number of filters per packet
 *       entry.  This is the maximum number of destination files
 *       to which a single packet can be written (at one time).
 *
 *  \par Limits:
 *       The number of filters per packet must be greater than zero
 *       and not greater than #DS_DEST_FILE_CNT.
 */
#define DS_FILTERS_PER_PACKET                   DS_INTERFACE_CFGVAL(FILTERS_PER_PACKET)
#define DEFAULT_DS_INTERFACE_FILTERS_PER_PACKET 4

/**
 *  \brief Common Table File -- descriptor text buffer size
 *
 *  \par Description:
 *       This parameter defines the size of the Descriptor Text
 *       fields in both the Destination File Table and the Packet
 *       Filter Table.  The buffer includes the string terminator.
 *
 *  \par Limits:
 *       The buffer size must be greater than zero and a multiple
 *       of four bytes for alignment.  There is no upper limit.
 */
#define DS_DESCRIPTOR_BUFSIZE                   DS_INTERFACE_CFGVAL(DESCRIPTOR_BUFSIZE)
#define DEFAULT_DS_INTERFACE_DESCRIPTOR_BUFSIZE 32

/**\}*/

#endif
