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
 *
 * Auto-Generated stub implementations for functions defined in ds_dispatch header
 */

#include "ds_dispatch.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppProcessCmd()
 * ----------------------------------------------------
 */
void DS_AppProcessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(DS_AppProcessCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(DS_AppProcessCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppProcessMsg()
 * ----------------------------------------------------
 */
void DS_AppProcessMsg(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(DS_AppProcessMsg, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(DS_AppProcessMsg, Basic, NULL);
}
