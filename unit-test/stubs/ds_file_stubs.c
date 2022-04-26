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
 *  Unit testing stubs for the ds_file.c file.
 */

#include "cfe.h"
#include "cfe_fs.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"

#include <stdio.h>

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileStorePacket() - store packet in file(s)                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileStorePacket), MessageID);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileStorePacket), BufPtr);
    UT_DEFAULT_IMPL(DS_FileStorePacket);
} /* End of DS_FileStorePacket() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileSetupWrite() - prepare to write packet data to file      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileSetupWrite(int32 FileIndex, const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileSetupWrite), FileIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileSetupWrite), BufPtr);
    UT_DEFAULT_IMPL(DS_FileSetupWrite);

} /* End of DS_FileSetupWrite() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileWriteData() - write data to destination file             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileWriteData(int32 FileIndex, const void *FileData, uint32 DataLength)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteData), FileIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteData), FileData);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteData), DataLength);
    UT_DEFAULT_IMPL(DS_FileWriteData);

} /* End of DS_FileWriteData() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileWriteHeader() - write header to destination file         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileWriteHeader(int32 FileIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteHeader), FileIndex);
    UT_DEFAULT_IMPL(DS_FileWriteHeader);

} /* End of DS_FileWriteHeader() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileWriteError() - file write error handler                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileWriteError(uint32 FileIndex, uint32 DataLength, int32 WriteResult)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteError), FileIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteError), DataLength);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileWriteError), WriteResult);
    UT_DEFAULT_IMPL(DS_FileWriteError);

} /* End of DS_FileWriteError() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileCreateDest() - create destination file                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateDest(uint32 FileIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCreateDest), FileIndex);
    UT_DEFAULT_IMPL(DS_FileCreateDest);
} /* End of DS_FileCreateDest() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create destination filename                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateName(uint32 FileIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCreateName), FileIndex);
    UT_DEFAULT_IMPL(DS_FileCreateName);

} /* End of DS_FileCreateName() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileCreateSequence() - set text from count or time           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateSequence(char *Buffer, uint32 Type, uint32 Count)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCreateSequence), Buffer);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCreateSequence), Type);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCreateSequence), Count);
    UT_DEFAULT_IMPL(DS_FileCreateSequence);

} /* End of DS_FileCreateSequence() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileUpdateHeader() - update destination file header          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileUpdateHeader(int32 FileIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileUpdateHeader), FileIndex);
    UT_DEFAULT_IMPL(DS_FileUpdateHeader);

} /* End of DS_FileUpdateHeader() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileCloseDest() - close destination file                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCloseDest(int32 FileIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileCloseDest), FileIndex);
    UT_DEFAULT_IMPL(DS_FileCloseDest);

} /* End of DS_FileCloseDest() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_FileTestAge() -- file age processor                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileTestAge(uint32 ElapsedSeconds)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_FileTestAge), ElapsedSeconds);
    UT_DEFAULT_IMPL(DS_FileTestAge);

} /* End of DS_FileTestAge() */

/************************/
/*  End of File Comment */
/************************/
