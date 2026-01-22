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
 *  The CFS Data Storage (DS) dispatch header file
 */
#ifndef DS_DISPATCH_H
#define DS_DISPATCH_H

#include "cfe.h"
#include "ds_platform_cfg.h"

/**
 *  \brief Software Bus message handler
 *
 *  \par Description
 *       Process packets received via Software Bus message pipe
 *       - may call application housekeeping request command handler
 *       - may call 1Hz wakeup command handler (if enabled)
 *       - may call application ground command handler
 *       All packets are processed for possible data storage
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] BufPtr Software Bus message pointer
 */
void DS_AppPipe(const CFE_SB_Buffer_t *BufPtr);

/**
 *  \brief Application ground command handler
 *
 *  \par Description
 *       Call command code specific DS command handler function
 *       Generate command error event for unknown command codes
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] BufPtr Software Bus message pointer
 */
void DS_AppProcessCmd(const CFE_SB_Buffer_t *BufPtr);

#endif
