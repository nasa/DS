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
 *  Unit testing stubs for the ds_app.c file.
 */

#include "cfe.h"

#include "ds_perfids.h"
#include "ds_msgids.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"
#include "ds_msgdefs.h"
#include "ds_version.h"

#include <stdio.h>

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application global data structure                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DS_AppData_t DS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application entry point and main process loop                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppMain(void)
{
    UT_DEFAULT_IMPL(DS_AppMain);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Application initialization                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_AppInitialize(void)
{
    return UT_DEFAULT_IMPL(DS_AppInitialize);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process Software Bus messages                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessMsg(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_AppProcessMsg), BufPtr);
    UT_DEFAULT_IMPL(DS_AppProcessMsg);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process application commands                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_AppProcessCmd), BufPtr);
    UT_DEFAULT_IMPL(DS_AppProcessCmd);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process hk request command                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppProcessHK(void)
{
    UT_DEFAULT_IMPL(DS_AppProcessHK);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Packet storage pre-processor                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_AppStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_AppStorePacket), MessageID);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_AppStorePacket), BufPtr);
    UT_DEFAULT_IMPL(DS_AppStorePacket);
}
