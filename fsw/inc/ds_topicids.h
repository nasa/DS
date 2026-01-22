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
 *   Specification for the CFS Data Storage constants for message IDs
 */
#ifndef DS_TOPICIDS_H
#define DS_TOPICIDS_H

#include "ds_topicid_values.h"

/**
 * \defgroup cfsdscmdtid CFS Data Storage Command Topic IDs
 * \{
 */

#define CFE_MISSION_DS_CMD_TOPICID             CFE_MISSION_DS_TIDVAL(CMD)
#define DEFAULT_CFE_MISSION_DS_CMD_TOPICID     0xBB
#define CFE_MISSION_DS_SEND_HK_TOPICID         CFE_MISSION_DS_TIDVAL(SEND_HK)
#define DEFAULT_CFE_MISSION_DS_SEND_HK_TOPICID 0xBC

/**\}*/

/**
 * \defgroup cfsdstlmtid CFS Data Storage Telemetry Topic IDs
 * \{
 */

#define CFE_MISSION_DS_HK_TLM_TOPICID           CFE_MISSION_DS_TIDVAL(HK_TLM)
#define DEFAULT_CFE_MISSION_DS_HK_TLM_TOPICID   0xB8
#define CFE_MISSION_DS_DIAG_TLM_TOPICID         CFE_MISSION_DS_TIDVAL(DIAG_TLM)
#define DEFAULT_CFE_MISSION_DS_DIAG_TLM_TOPICID 0xB9
#define CFE_MISSION_DS_COMP_TLM_TOPICID         CFE_MISSION_DS_TIDVAL(COMP_TLM)
#define DEFAULT_CFE_MISSION_DS_COMP_TLM_TOPICID 0xBA

/**\}*/

#endif
