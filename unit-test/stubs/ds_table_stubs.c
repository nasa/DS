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
 *  CFS Data Storage (DS) table management functions
 */

#include "cfe.h"

#include "ds_msgids.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"

#include "ds_app.h"
#include "ds_table.h"
#include "ds_msg.h"
#include "ds_events.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#define DS_CDS_NAME "DS_CDS"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableInit() - DS application table initialization            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableInit(void)
{
    return UT_DEFAULT_IMPL(DS_TableInit);
} /* End of DS_TableInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableManageDestFile() - manage table data updates            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableManageDestFile(void)
{
    UT_DEFAULT_IMPL(DS_TableManageDestFile);
} /* End of DS_TableManageDestFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableManageFilter() - manage table data updates              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableManageFilter(void)
{
    UT_DEFAULT_IMPL(DS_TableManageFilter);
} /* End of DS_TableManageFilter() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyDestFile() - validate table data                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableVerifyDestFile(const void *TableData)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyDestFile), TableData);
    return UT_DEFAULT_IMPL(DS_TableVerifyDestFile);
} /* End of DS_TableVerifyDestFile() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyDestFileEntry() - verify dest table entry         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyDestFileEntry(DS_DestFileEntry_t *DestFileEntry, uint8 TableIndex, int32 ErrorCount)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyDestFileEntry), DestFileEntry);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyDestFileEntry), TableIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyDestFileEntry), ErrorCount);
    return UT_DEFAULT_IMPL(DS_TableVerifyDestFileEntry);

} /* End of DS_TableVerifyDestFileEntry() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyFilter() - validate table data                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableVerifyFilter(const void *TableData)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyFilter), TableData);
    return UT_DEFAULT_IMPL(DS_TableVerifyFilter);

} /* End of DS_TableVerifyFilter() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyFilterEntry() - verify filter table entry         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyFilterEntry(DS_PacketEntry_t *PacketEntry, int32 TableIndex, int32 ErrorCount)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyFilterEntry), PacketEntry);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyFilterEntry), TableIndex);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyFilterEntry), ErrorCount);
    return UT_DEFAULT_IMPL(DS_TableVerifyFilterEntry);

} /* End of DS_TableVerifyFilterEntry() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableEntryUnused() - find unused table entries               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableEntryUnused(const void *TableEntry, int32 BufferSize)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableEntryUnused), TableEntry);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableEntryUnused), BufferSize);
    return UT_DEFAULT_IMPL(DS_TableEntryUnused);

} /* End of DS_TableEntryUnused() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyFileIndex() - verify dest file index              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyFileIndex(uint16 FileTableIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyFileIndex), FileTableIndex);
    return UT_DEFAULT_IMPL(DS_TableVerifyFileIndex);

} /* End of DS_TableVerifyFileIndex() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyParms() - verify algorithm parameters             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyParms(uint16 Algorithm_N, uint16 Algorithm_X, uint16 Algorithm_O)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyParms), Algorithm_N);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyParms), Algorithm_X);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyParms), Algorithm_O);
    return UT_DEFAULT_IMPL(DS_TableVerifyParms);
} /* End of DS_TableVerifyParms() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyType() - verify filter or filename type           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyType(uint16 TimeVsCount)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyType), TimeVsCount);
    return UT_DEFAULT_IMPL(DS_TableVerifyType);

} /* End of DS_TableVerifyType() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyState() - verify file ena/dis state               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyState(uint16 EnableState)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyState), EnableState);
    return UT_DEFAULT_IMPL(DS_TableVerifyState);

} /* End of DS_TableVerifyState() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifySize() - verify file size limit                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifySize(uint32 MaxFileSize)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifySize), MaxFileSize);
    return UT_DEFAULT_IMPL(DS_TableVerifySize);

} /* End of DS_TableVerifySize() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyAge() - verify file age limit                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyAge(uint32 MaxFileAge)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyAge), MaxFileAge);
    return UT_DEFAULT_IMPL(DS_TableVerifyAge);

} /* End of DS_TableVerifyAge() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableVerifyCount() - verify sequence count                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool DS_TableVerifyCount(uint32 SequenceCount)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableVerifyCount), SequenceCount);
    return UT_DEFAULT_IMPL(DS_TableVerifyCount);

} /* End of DS_TableVerifyCount() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableSubscribe() - process new filter table                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableSubscribe(void)
{
    UT_DEFAULT_IMPL(DS_TableSubscribe);
} /* End of DS_TableSubscribe() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableUnsubscribe() - process old filter table                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableUnsubscribe(void)
{
    UT_DEFAULT_IMPL(DS_TableUnsubscribe);
} /* End of DS_TableUnsubscribe() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableCreateCDS() - create DS storage area in CDS             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableCreateCDS(void)
{
    return UT_DEFAULT_IMPL(DS_TableCreateCDS);
} /* End of DS_TableCreateCDS() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableUpdateCDS() - update DS storage area in CDS             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableUpdateCDS(void)
{
    UT_DEFAULT_IMPL(DS_TableUpdateCDS);
} /* End of DS_TableUpdateCDS() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableHashFunction() - convert messageID to hash table index  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

uint32 DS_TableHashFunction(CFE_SB_MsgId_t MessageID)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableHashFunction), MessageID);
    return UT_DEFAULT_IMPL(DS_TableHashFunction);
} /* End of DS_TableHashFunction() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableCreateHash() - create and populate hash table           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_TableCreateHash(void)
{
    UT_DEFAULT_IMPL(DS_TableCreateHash);
} /* End of DS_TableCreateHash() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableFindMsgID() - get filter table index for MID            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableAddMsgID(CFE_SB_MsgId_t MessageID, int32 FilterIndex)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableAddMsgID), MessageID);
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableAddMsgID), FilterIndex);
    return UT_DEFAULT_IMPL(DS_TableAddMsgID);

} /* End of DS_TableAddMsgID() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS_TableFindMsgID() - get filter table index for MID            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32 DS_TableFindMsgID(CFE_SB_MsgId_t MessageID)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(DS_TableFindMsgID), MessageID);
    return UT_DEFAULT_IMPL(DS_TableFindMsgID);

} /* End of DS_TableFindMsgID() */

/************************/
/*  End of File Comment */
/************************/
