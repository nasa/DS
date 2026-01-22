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
 *  The CFS Data Storage (DS) Application command and telemetry
 *   messages.
 */
#ifndef DS_FCNCODES_H
#define DS_FCNCODES_H

#include "ds_fcncode_values.h"

/**
 * \defgroup cfsdscmdcodes CFS Checksum Command Codes
 * \{
 */

/**
 * \brief No-Operation Command
 *
 *  \par Description
 *       This command will increment the command execution counter and send an
 *       event containing the version number of the application.  The command
 *       is often used as a general test for application "aliveness".
 *
 *  \par Command Structure
 *       #DS_NoopCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_NOOP_INF_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_NOOP_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_NOOP_CC DS_CCVAL(NOOP)

/**
 * \brief Reset Housekeeping Telemetry Counters Command
 *
 *  \par Description
 *       This command will set the housekeeping counters to zero
 *
 *  \par Command Structure
 *       #DS_ResetCountersCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will reset to zero
 *       - The #DS_RESET_INF_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_RESET_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_RESET_COUNTERS_CC DS_CCVAL(RESET_COUNTERS)

/**
 * \brief Set Enable/Disable State For DS Application
 *
 *  \par Description
 *       This command will modify the Ena/Dis State selection for the
 *       DS application.  No packets are stored while DS is disabled.
 *
 *  \par Command Structure
 *       #DS_SetAppStateCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_ENADIS_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid enable/disable state selection
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_ENADIS_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_APP_STATE_CC DS_CCVAL(SET_APP_STATE)

/**
 * \brief Set Destination File For Packet Filter Table Entry
 *
 *  \par Description
 *       This command will modify the Destination File selection for the
 *       indicated entry in the Packet Filter Table.
 *
 *  \par Command Structure
 *       #DS_SetFilterFileCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_FILE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid filter table index
 *       - Invalid filter parameters index
 *       - Invalid destination file selection
 *       - Packet filter table is not currently loaded
 *       - Cannot modify unused packet filter table entry
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_FILE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_FILTER_FILE_CC DS_CCVAL(SET_FILTER_FILE)

/**
 * \brief Set Filter Type For Packet Filter Table Entry
 *
 *  \par Description
 *       This command will modify the Filter Type selection for the
 *       indicated entry in the Packet Filter Table.
 *
 *  \par Command Structure
 *       #DS_SetFilterTypeCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_FTYPE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid filter table index
 *       - Invalid filter parameters index
 *       - Invalid filter type selection
 *       - Packet filter table is not currently loaded
 *       - Cannot modify unused packet filter table entry
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_FTYPE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_FILTER_TYPE_CC DS_CCVAL(SET_FILTER_TYPE)

/**
 * \brief Set Algorithm Parameters For Packet Filter Table Entry
 *
 *  \par Description
 *       This command will modify the Algorithm Parameters for the
 *       indicated entry in the Packet Filter Table.
 *
 *  \par Command Structure
 *       #DS_SetFilterParmsCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_PARMS_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid filter table index
 *       - Invalid filter parameters index
 *       - Algorithm_N value cannot be greater than Algorithm_X
 *       - Packet filter table is not currently loaded
 *       - Cannot modify unused packet filter table entry
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_PARMS_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_FILTER_PARMS_CC DS_CCVAL(SET_FILTER_PARMS)

/**
 * \brief Set Filename Type For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the Filename Type selection for the
 *       indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestTypeCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_NTYPE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid filename type selection
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_NTYPE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_TYPE_CC DS_CCVAL(SET_DEST_TYPE)

/**
 * \brief Set Ena/Dis State For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the Ena/Dis State selection for the
 *       indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestStateCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_STATE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid destination state selection
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_STATE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_STATE_CC DS_CCVAL(SET_DEST_STATE)

/**
 * \brief Set Pathname For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the Pathname portion of the filename
 *       for the indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestPathCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_PATH_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid pathname string (empty, no terminator, bad characters)
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_PATH_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_PATH_CC DS_CCVAL(SET_DEST_PATH)

/**
 * \brief Set Basename For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the Basename portion of the filename
 *       for the indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestBaseCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_BASE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid basename string (no terminator, bad characters)
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_BASE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_BASE_CC DS_CCVAL(SET_DEST_BASE)

/**
 * \brief Set Extension For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the Extension portion of the filename
 *       for the indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestExtCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_EXT_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid extension string (empty, no terminator, bad characters)
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_EXT_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_EXT_CC DS_CCVAL(SET_DEST_EXT)

/**
 * \brief Set Max File Size For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the max file size selection for the
 *       indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestSizeCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_SIZE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid max file size selection
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_SIZE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_SIZE_CC DS_CCVAL(SET_DEST_SIZE)

/**
 * \brief Set Max File Age For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the max file age selection for the
 *       indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestAgeCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_AGE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid max file age selection
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_AGE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_AGE_CC DS_CCVAL(SET_DEST_AGE)

/**
 * \brief Set Sequence Count For Destination File Table Entry
 *
 *  \par Description
 *       This command will modify the sequence count value for the
 *       indicated entry in the Destination File Table.
 *
 *  \par Command Structure
 *       #DS_SetDestCountCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_SEQ_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Invalid sequence count value
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_SEQ_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_SET_DEST_COUNT_CC DS_CCVAL(SET_DEST_COUNT)

/**
 * \brief Close Destination File
 *
 *  \par Description
 *       This command will close the indicated Destination File.
 *
 *  \par Command Structure
 *       #DS_CloseFileCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_CLOSE_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Invalid destination file table index
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_CLOSE_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_CLOSE_FILE_CC DS_CCVAL(CLOSE_FILE)

/**
 * \brief Get File Info Packet
 *
 *  \par Description
 *       This command will send the DS File Info Packet.
 *
 *  \par Command Structure
 *       #DS_GetFileInfoCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_FileInfoPkt_t packet will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_GET_FILE_INFO_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_GET_FILE_INFO_CC DS_CCVAL(GET_FILE_INFO)

/**
 * \brief Add Message ID To Packet Filter Table
 *
 *  \par Description
 *       This command will change the Message ID selection for an
 *       unused Packet Filter Table entry to the indicated value.
 *
 *  \par Command Structure
 *       #DS_AddMidCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_ADD_MID_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Message ID is invalid (can be anything but zero)
 *       - Packet filter table is not currently loaded
 *       - Message ID already exists in packet filter table
 *       - All packet filter table entries are already in use
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_ADD_MID_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_ADD_MID_CC DS_CCVAL(ADD_MID)

/**
 * \brief Close All Destination Files
 *
 *  \par Description
 *       This command will close all open Destination Files.
 *       - NOTE: Using this command may incur a performance hit based upon
 *               the number and size of the files being closed.
 *
 *  \par Command Structure
 *       #DS_CloseAllCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_CLOSE_ALL_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Destination file table is not currently loaded
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_CLOSE_ALL_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_CLOSE_ALL_CC DS_CCVAL(CLOSE_ALL)

/**
 * \brief Remove Message ID from Packet Filter Table
 *
 *  \par Description
 *       This command will change the Message ID selection for a
 *       used Packet Filter Table entry to unused (0).
 *
 *  \par Command Structure
 *       #DS_RemoveMidCmd_t
 *
 *  \par Command Verification
 *       Evidence of success may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdAcceptedCounter will increment
 *       - The #DS_REMOVE_MID_CMD_EID informational event message will be sent
 *
 *  \par Error Conditions
 *       This command can fail for the following reasons:
 *       - Invalid command packet length
 *       - Message ID is invalid (can be anything but zero)
 *       - Packet filter table is not currently loaded
 *       - Message ID does not exist in packet filter table
 *
 *       Evidence of failure may be found in the following telemetry:
 *       - #DS_HkTlm_Payload_t.CmdRejectedCounter will increment
 *       - The #DS_REMOVE_MID_CMD_ERR_EID error event message will be sent
 *
 *  \par Criticality
 *       None
 */
#define DS_REMOVE_MID_CC DS_CCVAL(REMOVE_MID)

/**\}*/

#endif
