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
#ifndef DEFAULT_DS_MSGIDS_H
#define DEFAULT_DS_MSGIDS_H

#include "ds_msgid_values.h"

/**
 * \defgroup cfsdscmdmid CFS Data Storage Message IDs
 * \{
 */

#define DS_CMD_MID     CFE_PLATFORM_DS_CMD_MIDVAL(CMD)
#define DS_SEND_HK_MID CFE_PLATFORM_DS_CMD_MIDVAL(SEND_HK)

/**\}*/

/**
 * \defgroup cfsdstlmmid CFS Data Storage Message IDs
 * \{
 */

#define DS_HK_TLM_MID   CFE_PLATFORM_DS_TLM_MIDVAL(HK_TLM)   /**< \brief DS Hk Telemetry Message ID ****/
#define DS_DIAG_TLM_MID CFE_PLATFORM_DS_TLM_MIDVAL(DIAG_TLM) /**< \brief DS File Info Telemetry Message ID ****/
#define DS_COMP_TLM_MID \
    CFE_PLATFORM_DS_TLM_MIDVAL(COMP_TLM) /**< \brief DS Completed File Info Telemetry Message ID ****/

/**\}*/

#endif
