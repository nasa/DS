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
 *  The CFS Data Storage (DS) Application table related
 *   constant definitions.
 */
#ifndef EDS_DS_TBLDEFS_H
#define EDS_DS_TBLDEFS_H

#include "ds_extern_typedefs.h"
#include "ds_eds_typedefs.h"

#define DS_UNUSED   DS_EnableState_UNUSED   /**< \brief Unused entries in DS tables */
#define DS_DISABLED DS_EnableState_DISABLED /**< \brief Enable/disable state selection */
#define DS_ENABLED  DS_EnableState_ENABLED  /**< \brief Enable/disable state selection */

#define DS_BY_TIME  DS_FilterType_BY_TIME  /**< \brief Action is based on packet timestamp */
#define DS_BY_COUNT DS_FilterType_BY_COUNT /**< \brief Action is based on packet sequence count */


#endif
