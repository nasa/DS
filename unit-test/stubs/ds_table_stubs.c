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
 * Auto-Generated stub implementations for functions defined in ds_table header
 */

#include "ds_table.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableAddMsgID()
 * ----------------------------------------------------
 */
int32 DS_TableAddMsgID(CFE_SB_MsgId_t MessageID, int32 FilterIndex)
{
    UT_GenStub_SetupReturnBuffer(DS_TableAddMsgID, int32);

    UT_GenStub_AddParam(DS_TableAddMsgID, CFE_SB_MsgId_t, MessageID);
    UT_GenStub_AddParam(DS_TableAddMsgID, int32, FilterIndex);

    UT_GenStub_Execute(DS_TableAddMsgID, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableAddMsgID, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableCreateCDS()
 * ----------------------------------------------------
 */
CFE_Status_t DS_TableCreateCDS(void)
{
    UT_GenStub_SetupReturnBuffer(DS_TableCreateCDS, CFE_Status_t);

    UT_GenStub_Execute(DS_TableCreateCDS, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableCreateCDS, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableCreateHash()
 * ----------------------------------------------------
 */
void DS_TableCreateHash(void)
{

    UT_GenStub_Execute(DS_TableCreateHash, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableEntryUnused()
 * ----------------------------------------------------
 */
bool DS_TableEntryUnused(const void *TableEntry, int32 BufferSize)
{
    UT_GenStub_SetupReturnBuffer(DS_TableEntryUnused, bool);

    UT_GenStub_AddParam(DS_TableEntryUnused, const void *, TableEntry);
    UT_GenStub_AddParam(DS_TableEntryUnused, int32, BufferSize);

    UT_GenStub_Execute(DS_TableEntryUnused, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableEntryUnused, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableFindMsgID()
 * ----------------------------------------------------
 */
int32 DS_TableFindMsgID(CFE_SB_MsgId_t MessageID)
{
    UT_GenStub_SetupReturnBuffer(DS_TableFindMsgID, int32);

    UT_GenStub_AddParam(DS_TableFindMsgID, CFE_SB_MsgId_t, MessageID);

    UT_GenStub_Execute(DS_TableFindMsgID, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableFindMsgID, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableHashFunction()
 * ----------------------------------------------------
 */
uint32 DS_TableHashFunction(CFE_SB_MsgId_t MessageID)
{
    UT_GenStub_SetupReturnBuffer(DS_TableHashFunction, uint32);

    UT_GenStub_AddParam(DS_TableHashFunction, CFE_SB_MsgId_t, MessageID);

    UT_GenStub_Execute(DS_TableHashFunction, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableHashFunction, uint32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableInit()
 * ----------------------------------------------------
 */
CFE_Status_t DS_TableInit(void)
{
    UT_GenStub_SetupReturnBuffer(DS_TableInit, CFE_Status_t);

    UT_GenStub_Execute(DS_TableInit, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableInit, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableManageDestFile()
 * ----------------------------------------------------
 */
void DS_TableManageDestFile(void)
{

    UT_GenStub_Execute(DS_TableManageDestFile, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableManageFilter()
 * ----------------------------------------------------
 */
void DS_TableManageFilter(void)
{

    UT_GenStub_Execute(DS_TableManageFilter, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableSubscribe()
 * ----------------------------------------------------
 */
void DS_TableSubscribe(void)
{

    UT_GenStub_Execute(DS_TableSubscribe, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableUnsubscribe()
 * ----------------------------------------------------
 */
void DS_TableUnsubscribe(void)
{

    UT_GenStub_Execute(DS_TableUnsubscribe, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableUpdateCDS()
 * ----------------------------------------------------
 */
void DS_TableUpdateCDS(void)
{

    UT_GenStub_Execute(DS_TableUpdateCDS, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyAge()
 * ----------------------------------------------------
 */
bool DS_TableVerifyAge(uint32 MaxFileAge)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyAge, bool);

    UT_GenStub_AddParam(DS_TableVerifyAge, uint32, MaxFileAge);

    UT_GenStub_Execute(DS_TableVerifyAge, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyAge, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyCount()
 * ----------------------------------------------------
 */
bool DS_TableVerifyCount(uint32 SequenceCount)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyCount, bool);

    UT_GenStub_AddParam(DS_TableVerifyCount, uint32, SequenceCount);

    UT_GenStub_Execute(DS_TableVerifyCount, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyCount, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyDestFile()
 * ----------------------------------------------------
 */
CFE_Status_t DS_TableVerifyDestFile(const void *TableData)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyDestFile, CFE_Status_t);

    UT_GenStub_AddParam(DS_TableVerifyDestFile, const void *, TableData);

    UT_GenStub_Execute(DS_TableVerifyDestFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyDestFile, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyDestFileEntry()
 * ----------------------------------------------------
 */
bool DS_TableVerifyDestFileEntry(DS_DestFileEntry_t *DestFileEntry, uint8 TableIndex, int32 ErrorCount)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyDestFileEntry, bool);

    UT_GenStub_AddParam(DS_TableVerifyDestFileEntry, DS_DestFileEntry_t *, DestFileEntry);
    UT_GenStub_AddParam(DS_TableVerifyDestFileEntry, uint8, TableIndex);
    UT_GenStub_AddParam(DS_TableVerifyDestFileEntry, int32, ErrorCount);

    UT_GenStub_Execute(DS_TableVerifyDestFileEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyDestFileEntry, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyFileIndex()
 * ----------------------------------------------------
 */
bool DS_TableVerifyFileIndex(uint16 FileTableIndex)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyFileIndex, bool);

    UT_GenStub_AddParam(DS_TableVerifyFileIndex, uint16, FileTableIndex);

    UT_GenStub_Execute(DS_TableVerifyFileIndex, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyFileIndex, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyFilter()
 * ----------------------------------------------------
 */
CFE_Status_t DS_TableVerifyFilter(const void *TableData)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyFilter, CFE_Status_t);

    UT_GenStub_AddParam(DS_TableVerifyFilter, const void *, TableData);

    UT_GenStub_Execute(DS_TableVerifyFilter, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyFilter, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyFilterEntry()
 * ----------------------------------------------------
 */
bool DS_TableVerifyFilterEntry(DS_PacketEntry_t *PacketEntry, int32 TableIndex, int32 ErrorCount)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyFilterEntry, bool);

    UT_GenStub_AddParam(DS_TableVerifyFilterEntry, DS_PacketEntry_t *, PacketEntry);
    UT_GenStub_AddParam(DS_TableVerifyFilterEntry, int32, TableIndex);
    UT_GenStub_AddParam(DS_TableVerifyFilterEntry, int32, ErrorCount);

    UT_GenStub_Execute(DS_TableVerifyFilterEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyFilterEntry, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyParms()
 * ----------------------------------------------------
 */
bool DS_TableVerifyParms(uint16 Algorithm_N, uint16 Algorithm_X, uint16 Algorithm_O)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyParms, bool);

    UT_GenStub_AddParam(DS_TableVerifyParms, uint16, Algorithm_N);
    UT_GenStub_AddParam(DS_TableVerifyParms, uint16, Algorithm_X);
    UT_GenStub_AddParam(DS_TableVerifyParms, uint16, Algorithm_O);

    UT_GenStub_Execute(DS_TableVerifyParms, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyParms, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifySize()
 * ----------------------------------------------------
 */
bool DS_TableVerifySize(uint32 MaxFileSize)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifySize, bool);

    UT_GenStub_AddParam(DS_TableVerifySize, uint32, MaxFileSize);

    UT_GenStub_Execute(DS_TableVerifySize, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifySize, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyState()
 * ----------------------------------------------------
 */
bool DS_TableVerifyState(uint16 EnableState)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyState, bool);

    UT_GenStub_AddParam(DS_TableVerifyState, uint16, EnableState);

    UT_GenStub_Execute(DS_TableVerifyState, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyState, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_TableVerifyType()
 * ----------------------------------------------------
 */
bool DS_TableVerifyType(uint16 TimeVsCount)
{
    UT_GenStub_SetupReturnBuffer(DS_TableVerifyType, bool);

    UT_GenStub_AddParam(DS_TableVerifyType, uint16, TimeVsCount);

    UT_GenStub_Execute(DS_TableVerifyType, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_TableVerifyType, bool);
}
