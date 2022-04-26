/************************************************************************
** File: ds_cmds_stubs.c
**
**  NASA Docket No. GSC-18448-1, and identified as "cFS Data Storage (DS)
**  application version 2.5.2”
**
**  Copyright © 2019 United States Government as represented by the Administrator
**  of the National Aeronautics and Space Administration.  All Rights Reserved.
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**  http://www.apache.org/licenses/LICENSE-2.0
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
** Purpose:
**  Unit testing stubs for the ds_cmds.c file.
**
*************************************************************************/

#include "cfe.h"

#include "cfs_utils.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"
#include "ds_msgids.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_cmds.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"
#include "ds_version.h"

#include "string.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdNoop() - NOOP command                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdNoop(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdNoop), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdNoop);

} /* End of DS_CmdNoop() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdReset() - reset hk telemetry counters command             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdReset(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdReset), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdReset);

} /* End of DS_CmdReset() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetAppState() - set application ena/dis state             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetAppState(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetAppState), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetAppState);

} /* End of DS_CmdSetAppState() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetFilterFile() - set packet filter file index            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetFilterFile(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetFilterFile), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetFilterFile);

} /* End of DS_CmdSetFilterFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetFilterType() - set pkt filter filename type            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetFilterType(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetFilterType), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetFilterType);

} /* End of DS_CmdSetFilterType() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetFilterParms() - set packet filter parameters           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetFilterParms(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetFilterParms), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetFilterParms);

} /* End of DS_CmdSetFilterParms() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestType() - set destination filename type             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestType(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestType), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestType);

} /* End of DS_CmdSetDestType() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestState() - set dest file ena/dis state              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestState(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestState), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestState);

} /* End of DS_CmdSetDestState() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestPath() - set path portion of filename              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestPath(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestPath), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestPath);

} /* End of DS_CmdSetDestPath() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestBase() - set base portion of filename              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestBase(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestBase), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestBase);

} /* End of DS_CmdSetDestBase() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestExt() - set extension portion of filename          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestExt(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestExt), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestExt);

} /* End of DS_CmdSetDestExt() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestSize() - set maximum file size limit               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestSize(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestSize), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestSize);

} /* End of DS_CmdSetDestSize() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestAge() - set maximum file age limit                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestAge(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestAge), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestAge);

} /* End of DS_CmdSetDestAge() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdSetDestCount() - set seq cnt portion of filename          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdSetDestCount(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdSetDestCount), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdSetDestCount);

} /* End of DS_CmdSetDestCount() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdCloseFile() - close destination file                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdCloseFile(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdCloseFile), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdCloseFile);

} /* End of DS_CmdCloseFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdCloseAll() - close all open destination files             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdCloseAll(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdCloseAll), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdCloseAll);

} /* End of DS_CmdCloseAll() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdGetFileInfo() - get file info packet                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdGetFileInfo(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdGetFileInfo), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdGetFileInfo);

} /* End of DS_CmdGetFileInfo() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_CmdAddMID() - add message ID to packet filter table          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_CmdAddMID(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_CmdAddMID), BufPtr);
    UT_DEFAULT_IMPL(DS_CmdAddMID);

} /* End of DS_CmdAddMID() */

/************************/
/*  End of File Comment */
/************************/
