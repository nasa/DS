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
 *   This file contains unit test cases for the functions contained in the file ds_file.c
 */

/*
 * Includes
 */

#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_file.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_events.h"
#include "ds_version.h"
#include "ds_test_utils.h"
#include "ds_table.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

void UT_CFE_TIME_Print_CustomHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    char *PrintBuffer = UT_Hook_GetArgValueByName(Context, "PrintBuffer", char *);

    snprintf(PrintBuffer, CFE_TIME_PRINTED_STRING_SIZE, "1980-001-00:00.00.00000");
}

/*
 * Helper Functions
 */

void UT_DS_SetDestFileEntry(DS_DestFileEntry_t *DestFileEntryPtr)
{
    strncpy(DestFileEntryPtr->Pathname, "path", sizeof(DestFileEntryPtr->Pathname));
    strncpy(DestFileEntryPtr->Basename, "base", sizeof(DestFileEntryPtr->Basename));
    strncpy(DestFileEntryPtr->Extension, "ext", sizeof(DestFileEntryPtr->Extension));
}

/*
 * Function Definitions
 */

void DS_FileStorePacket_Test_Nominal(void)
{
    CFE_SB_MsgId_t          MessageID = DS_UT_MID_1;
    DS_HashLink_t           HashLink;
    size_t                  forced_Size     = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t          forced_MsgID    = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t       forced_CmdCode  = DS_NOOP_CC;
    CFE_MSG_SequenceCount_t forced_SeqCount = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSequenceCount), &forced_SeqCount, sizeof(forced_SeqCount), false);
    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), 0);

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = forced_Size * 2;

    DS_AppData.HashTable[187]                                   = &HashLink;
    HashLink.Index                                              = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID                = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N    = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X    = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O    = 0;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FileTableIndex = 0;
    DS_AppData.FileStatus[0].FileState                          = DS_ENABLED;
    DS_AppData.FileStatus[0].FileHandle                         = DS_UT_OBJID_1;
    DS_AppData.FileStatus[0].FileSize                           = 0;

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.PassedPktCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileStorePacket_Test_PacketNotInTable(void)
{
    CFE_SB_MsgId_t MessageID = DS_UT_MID_1;
    DS_HashLink_t  HashLink;

    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187] = &HashLink;
    HashLink.Index            = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.IgnoredPktCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileStorePacket_Test_PassedFilterFalse(void)
{
    CFE_SB_MsgId_t          MessageID = DS_UT_MID_1;
    DS_HashLink_t           HashLink;
    size_t                  forced_Size     = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t          forced_MsgID    = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t       forced_CmdCode  = DS_NOOP_CC;
    CFE_MSG_SequenceCount_t forced_SeqCount = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSequenceCount), &forced_SeqCount, sizeof(forced_SeqCount), false);

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                = &HashLink;
    HashLink.Index                                           = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID             = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType  = 1;
    DS_AppData.FileStatus[0].FileState                       = DS_ENABLED;

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilteredPktCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileStorePacket_Test_DisabledDest(void)
{
    CFE_SB_MsgId_t    MessageID = DS_UT_MID_1;
    DS_HashLink_t     HashLink;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                = &HashLink;
    HashLink.Index                                           = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID             = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType  = 1;
    DS_AppData.FileStatus[0].FileState                       = DS_DISABLED;

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilteredPktCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileStorePacket_Test_InvalidIndex(void)
{
    CFE_SB_MsgId_t    MessageID = DS_UT_MID_1;
    DS_HashLink_t     HashLink;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                   = &HashLink;
    HashLink.Index                                              = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID                = DS_UT_MID_1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N    = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X    = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O    = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FileTableIndex = DS_DEST_FILE_CNT;
    DS_AppData.FileStatus[0].FileState                          = DS_ENABLED;

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilteredPktCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileSetupWrite_Test_Nominal(void)
{
    int32             FileIndex      = 0;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    DS_AppData.FileStatus[FileIndex].FileSize              = 3;

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileSetupWrite_Test_FileHandleClosed(void)
{
    int32             FileIndex      = 0;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle             = OS_OBJECT_ID_UNDEFINED;
    DS_AppData.FileStatus[FileIndex].FileCount              = 0;
    DS_AppData.FileStatus[FileIndex].FileSize               = 3;

    /* Fail writing the header so the data won't write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1); /* Don't care about subroutine event */
    UtAssert_STUB_COUNT(OS_write, 0);
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
}

void DS_FileSetupWrite_Test_MaxFileSizeExceeded(void)
{
    int32  FileIndex   = 0;
    size_t forced_Size = sizeof(DS_NoopCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 5;
    DS_AppData.FileStatus[FileIndex].FileSize              = 10;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));

#if (DS_MOVE_FILES == true)
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));
#endif

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileWriteData_Test_Nominal(void)
{
    int32             FileIndex      = 0;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &UT_CmdBuf.Buf, sizeof(UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileWriteCounter, 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, sizeof(UT_CmdBuf.Buf));
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileGrowth, sizeof(UT_CmdBuf.Buf));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileWriteData_Test_Error(void)
{
    int32  FileIndex  = 0;
    uint32 DataLength = 10;

    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Set to reach error case being tested (DS_FileWriteError) */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &UT_CmdBuf.Buf, DataLength);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_WRITE_FILE_ERR_EID);
}

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal(void)
{
    int32 FileIndex = 0;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileWriteCounter, 2);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t));
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileGrowth, sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t));
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_PrimaryHeaderError(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0]  = '\0';
    /* Set to generate primary header error */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_SecondaryHeaderError(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    /* Set to generate secondary header error */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteError_Test(void)
{
    int32  FileIndex   = 0;
    uint32 DataLength  = 10;
    int32  WriteResult = -1;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    /* Execute the function being tested */
    DS_FileWriteError(FileIndex, DataLength, WriteResult);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileWriteErrCounter, 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileState, DS_DISABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_WRITE_FILE_ERR_EID);
}
#endif

void DS_FileCreateDest_Test_Nominal(void)
{
    uint32 FileIndex = 0;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.FileStatus[FileIndex].FileCount  = 1;
    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    /* Set to fail the condition "if (Result < 0)" */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;

    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 3);
#else
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 1);
#endif

    /* the file handle should have been reset and should not be closed */
    UtAssert_BOOL_FALSE(OS_ObjectIdEqual(DS_AppData.FileStatus[FileIndex].FileHandle, DS_UT_OBJID_1));
    UtAssert_BOOL_TRUE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));

    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 2);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(DS_TableUpdateCDS, 1);
}

void DS_FileCreateDest_Test_StringTerminate(void)
{
    uint32 FileIndex = 0;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateDest(FileIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(OS_OpenCreate, 0);
}

void DS_FileCreateDest_Test_NominalRollover(void)
{
    uint32 FileIndex = 0;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.FileStatus[FileIndex].FileCount  = DS_MAX_SEQUENCE_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    /* Set to fail the condition "if (Result < 0)" */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType  = DS_BY_COUNT;
    DS_AppData.DestFileTblPtr->File[FileIndex].SequenceCount = 3;
    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 3);
#else
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 1);
#endif

    /* the file handle should have been reset and should not be closed */
    UtAssert_BOOL_FALSE(OS_ObjectIdEqual(DS_AppData.FileStatus[FileIndex].FileHandle, DS_UT_OBJID_1));
    UtAssert_BOOL_TRUE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));

    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 3);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(DS_TableUpdateCDS, 1);
}

void DS_FileCreateDest_Test_Error(void)
{
    int32 FileIndex = 0;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;
    DS_AppData.FileStatus[FileIndex].FileCount  = DS_MAX_SEQUENCE_COUNT + 1;

    /* Set to generate error message DS_CREATE_FILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), -1);

    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileWriteErrCounter, 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileState, DS_DISABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_CREATE_FILE_ERR_EID);
}

void DS_FileCreateDest_Test_ClosedFileHandle(void)
{
    int32 FileIndex = 0;

    DS_AppData.DestFileTblPtr->File[FileIndex].SequenceCount = 5;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    /* Set to fail header write, which will call OS_close and clear the handle */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 1);
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void DS_FileCreateName_Test_Nominal(void)
{
    int32 FileIndex = 0;
    char  StrFormat[OS_MAX_PATH_LEN];
    char  StrCompare[OS_MAX_PATH_LEN];

    snprintf(StrFormat, sizeof(StrFormat), "path/base%%0%uu.ext", DS_SEQUENCE_DIGITS);
    snprintf(StrCompare, sizeof(StrCompare), StrFormat, 1);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_STRINGBUF_EQ(DS_AppData.FileStatus[FileIndex].FileName, sizeof(DS_AppData.FileStatus[FileIndex].FileName),
                          StrCompare, sizeof(StrCompare));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileCreateName_Test_NominalWithSeparator(void)
{
    int32 FileIndex = 0;
    char  StrFormat[OS_MAX_PATH_LEN];
    char  StrCompare[OS_MAX_PATH_LEN];

    snprintf(StrFormat, sizeof(StrFormat), "path/base%%0%uu.ext", DS_SEQUENCE_DIGITS);
    snprintf(StrCompare, sizeof(StrCompare), StrFormat, 1);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname));

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_STRINGBUF_EQ(DS_AppData.FileStatus[FileIndex].FileName, sizeof(DS_AppData.FileStatus[FileIndex].FileName),
                          StrCompare, sizeof(StrCompare));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileCreateName_Test_NominalWithPeriod(void)
{
    int32 FileIndex = 0;
    char  StrFormat[OS_MAX_PATH_LEN];
    char  StrCompare[OS_MAX_PATH_LEN];

    snprintf(StrFormat, sizeof(StrFormat), "path/base%%0%uu.ext", DS_SEQUENCE_DIGITS);
    snprintf(StrCompare, sizeof(StrCompare), StrFormat, 1);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, ".ext",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Extension));

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_STRINGBUF_EQ(DS_AppData.FileStatus[FileIndex].FileName, sizeof(DS_AppData.FileStatus[FileIndex].FileName),
                          StrCompare, sizeof(StrCompare));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileCreateName_Test_EmptyPath(void)
{
    int32 FileIndex = 0;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    DS_AppData.DestFileTblPtr->File[FileIndex].Pathname[0] = '\0';

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileState, DS_DISABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_CREATE_EMPTY_PATH_ERR_EID);
}

void DS_FileCreateName_Test_Error(void)
{
    int32 FileIndex = 0;
    int32 i;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    for (i = 0; i < DS_TOTAL_FNAME_BUFSIZE - 2; i++)
    {
        DS_AppData.DestFileTblPtr->File[FileIndex].Basename[i] = 'a';
    }

    DS_AppData.DestFileTblPtr->File[FileIndex].Basename[DS_TOTAL_FNAME_BUFSIZE - 1] = '\0';

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileState, DS_DISABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);
}

void DS_FileCreateName_Test_PathBaseSeqTooLarge(void)
{
    int32 FileIndex   = 0;
    int32 PathnameLen = (DS_TOTAL_FNAME_BUFSIZE - DS_SEQUENCE_DIGITS - 1) / 2;
    int32 BasenameLen = DS_TOTAL_FNAME_BUFSIZE - DS_SEQUENCE_DIGITS - 1 - PathnameLen;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    /* Set to fail the condition "if ((strlen(Workname) + strlen(Sequence)) < DS_TOTAL_FNAME_BUFSIZE)" */
    memset(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, 'p', PathnameLen);
    memset(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, 'b', BasenameLen);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);
}

void DS_FileCreateName_Test_PathBaseSeqExtTooLarge(void)
{
    int32 FileIndex   = 0;
    int32 PathnameLen = (DS_TOTAL_FNAME_BUFSIZE - DS_SEQUENCE_DIGITS - 1) / 2;
    int32 BasenameLen = DS_TOTAL_FNAME_BUFSIZE - DS_SEQUENCE_DIGITS - 1 - PathnameLen;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    /* Set to fail the condition "if (strlen(Workname) < DS_TOTAL_FNAME_BUFSIZE)" */
    memset(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, 'p', PathnameLen);
    memset(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, 'b', BasenameLen);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "ext",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Extension));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);
}

void DS_FileCreateName_Test_ExtensionZero(void)
{
    int32 FileIndex = 0;
    char  StrFormat[OS_MAX_PATH_LEN];
    char  StrCompare[OS_MAX_PATH_LEN];

    snprintf(StrFormat, sizeof(StrFormat), "path/base%%0%uu", DS_SEQUENCE_DIGITS);
    snprintf(StrCompare, sizeof(StrCompare), StrFormat, 1);

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    /* Set to fail the condition "if (strlen(DestFile->Extension) > 0)" */
    DS_AppData.DestFileTblPtr->File[FileIndex].Extension[0] = '\0';

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_STRINGBUF_EQ(DS_AppData.FileStatus[FileIndex].FileName, sizeof(DS_AppData.FileStatus[FileIndex].FileName),
                          StrCompare, sizeof(StrCompare));
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_ByCount(void)
{
    const uint32 Count = 1;
    char         StrFormat[DS_TOTAL_FNAME_BUFSIZE];
    char         StrCompare[DS_TOTAL_FNAME_BUFSIZE];
    char         Sequence[DS_TOTAL_FNAME_BUFSIZE];

    snprintf(StrFormat, sizeof(StrFormat), "%%0%uu", DS_SEQUENCE_DIGITS);
    snprintf(StrCompare, sizeof(StrCompare), StrFormat, Count);

    memset(Sequence, 0, sizeof(Sequence));

    /* Execute the function being tested */
    DS_FileCreateSequence(Sequence, DS_BY_COUNT, Count);

    /* Verify results */
    UtAssert_STRINGBUF_EQ(Sequence, sizeof(Sequence), StrCompare, sizeof(StrCompare));
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_ByTime(void)
{
    int32              FileIndex = 0;
    CFE_TIME_SysTime_t FakeTime;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE] = "";

    memset(&FakeTime, 0, sizeof(FakeTime));

    UT_SetDataBuffer(UT_KEY(CFE_TIME_GetTime), &FakeTime, sizeof(FakeTime), false);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_TIME;

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    UT_SetHandlerFunction(UT_KEY(CFE_TIME_Print), &UT_CFE_TIME_Print_CustomHandler, NULL);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateSequence(Sequence, DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType,
                                            DS_AppData.FileStatus[FileIndex].FileCount));

    /* Verify results */
    UtAssert_INT32_EQ(strncmp(Sequence, "1980001000000", DS_TOTAL_FNAME_BUFSIZE), 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_BadFilenameType(void)
{
    int32 FileIndex = 0;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

    memset(Sequence, 0xFF, sizeof(Sequence));

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_TIME;

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateSequence(Sequence, 99, DS_AppData.FileStatus[FileIndex].FileCount);

    /* Verify results */
    UtAssert_UINT32_EQ(strncmp(Sequence, "", DS_TOTAL_FNAME_BUFSIZE), 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal(void)
{
    int32 FileIndex = 0;

    /* Execute the function being tested */
    DS_FileUpdateHeader(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileUpdateCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileUpdateHeader_Test_WriteError(void)
{
    int32 FileIndex = 0;

    /* Set to fail condition "if (Result == sizeof(CFE_TIME_SysTime_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    DS_FileUpdateHeader(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileUpdateErrCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileUpdateHeader_Test_PlatformConfigCFE_SeekError(void)
{
    int32 FileIndex = 0;

    /* Set to fail condition "if (Result == sizeof(CFE_FS_Header_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), -1);

    /* Execute the function being tested */
    DS_FileUpdateHeader(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileUpdateErrCounter, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Set to generate error message DS_MOVE_FILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_mv), -1);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_MOVE_FILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge(void)
{
    int32 FileIndex = 0;
    const char DirName[] = "directory1/";
    
    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);
    
    size_t DirNameLen = sizeof(DirName);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, DirName, DirNameLen);
    memset(&DS_AppData.FileStatus[FileIndex].FileName[DirNameLen-1], 'f', DS_TOTAL_FNAME_BUFSIZE - DirNameLen);
    DS_AppData.FileStatus[FileIndex].FileName[DS_TOTAL_FNAME_BUFSIZE-1] = '\0';
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_MOVE_FILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameNull(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCloseDest(FileIndex));

    /* Verify results */
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_MOVE_FILE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}
#endif

#if (DS_MOVE_FILES == false)
void DS_FileCloseDest_Test_MoveFilesFalse(void)
{
    int32 FileIndex = 0;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileHandle, OS_OBJECT_ID_UNDEFINED);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileSize, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileName[0], 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif

void DS_FileTestAge_Test_Nominal(void)
{
    int32  FileIndex      = 0;
    uint32 ElapsedSeconds = 2;
    uint32 i;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        strncpy(DS_AppData.FileStatus[i].FileName, "directory1/filename", sizeof(DS_AppData.FileStatus[i].FileName));
    }

    DS_AppData.FileStatus[FileIndex].FileAge              = 0;
    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge = 3;

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 2);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileTestAge_Test_NullTable(void)
{
    uint32 ElapsedSeconds     = 2;
    DS_AppData.DestFileTblPtr = NULL;

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_STUB_COUNT(OS_close, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_FileTestAge_Test_ExceedMaxAge(void)
{
    int32  FileIndex      = 0;
    uint32 ElapsedSeconds = 2;

    /* Set up the handle */
    OS_OpenCreate(&DS_AppData.FileStatus[FileIndex].FileHandle, NULL, 0, 0);

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge = 1;

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[FileIndex].FileAge, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_IsPacketFiltered_Test_AlgX0(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 2;
    uint16            Alg_N      = 0;
    uint16            Alg_X      = 0;
    uint16            Alg_O      = 0;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_AlgN0(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 2;
    uint16            Alg_N      = 0;
    uint16            Alg_X      = 1;
    uint16            Alg_O      = 0;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_AlgNGreaterX(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 2;
    uint16            Alg_N      = 2;
    uint16            Alg_X      = 1;
    uint16            Alg_O      = 0;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_Alg0GreaterX(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 2;
    uint16            Alg_N      = 1;
    uint16            Alg_X      = 1;
    uint16            Alg_O      = 2;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_Alg0EqualX(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 2;
    uint16            Alg_N      = 1;
    uint16            Alg_X      = 1;
    uint16            Alg_O      = 1;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_InvalidFilterType(void)
{
    bool              Result;
    CFE_MSG_Message_t Message;
    uint16            FilterType = 0xff;
    uint16            Alg_N      = 1;
    uint16            Alg_X      = 1;
    uint16            Alg_O      = 0;

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_SeqFilter(void)
{
    bool                    Result;
    CFE_MSG_Message_t       Message;
    CFE_MSG_SequenceCount_t SeqCnt     = 0;
    uint16                  FilterType = 1;
    uint16                  Alg_N      = 1;
    uint16                  Alg_X      = 1;
    uint16                  Alg_O      = 0;

    memset(&Message, 0, sizeof(Message));

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSequenceCount), &SeqCnt, sizeof(SeqCnt), false);

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_FALSE(Result);
}

void DS_IsPacketFiltered_Test_TimeFilter1(void)
{
    bool               Result;
    CFE_MSG_Message_t  Message;
    CFE_TIME_SysTime_t PacketTime;
    uint16             FilterType = 2;
    uint16             Alg_N      = 1;
    uint16             Alg_X      = 1;
    uint16             Alg_O      = 0;

    memset(&Message, 0, sizeof(Message));

    PacketTime.Seconds    = 1;
    PacketTime.Subseconds = 1;

    /* This packet will be passed by the filter algorithm */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgTime), &PacketTime, sizeof(PacketTime), false);

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_FALSE(Result);
}

void DS_IsPacketFiltered_Test_TimeFilter2(void)
{
    bool               Result;
    CFE_MSG_Message_t  Message;
    CFE_TIME_SysTime_t PacketTime;
    uint16             FilterType = 2;
    uint16             Alg_N      = 2;
    uint16             Alg_X      = 2;
    uint16             Alg_O      = 1;

    memset(&Message, 0, sizeof(Message));

    /* Value is less than offset of passed range, this packet will be filtered */
    PacketTime.Seconds    = 0;
    PacketTime.Subseconds = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgTime), &PacketTime, sizeof(PacketTime), false);

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_IsPacketFiltered_Test_TimeFilter3(void)
{
    bool               Result;
    CFE_MSG_Message_t  Message;
    CFE_TIME_SysTime_t PacketTime;
    uint16             FilterType = 2;
    uint16             Alg_N      = 3;
    uint16             Alg_X      = 4;
    uint16             Alg_O      = 1;

    memset(&Message, 0, sizeof(Message));

    /* This packet will be filtered by the filter algorithm */
    PacketTime.Seconds    = 3;
    PacketTime.Subseconds = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgTime), &PacketTime, sizeof(PacketTime), false);

    /* Execute the function being tested */
    Result = DS_IsPacketFiltered(&Message, FilterType, Alg_N, Alg_X, Alg_O);

    /* Verify results */
    UtAssert_BOOL_TRUE(Result);
}

void DS_FileTransmit_Test_Nominal(void)
{
    DS_FileCompletePktBuf_t  PktBuf;
    DS_FileCompletePktBuf_t *PktBufPtr = &PktBuf;

    /* setup for a call to CFE_SB_AllocateMessageBuffer() */
    memset(PktBufPtr, 0, sizeof(*PktBufPtr));
    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &PktBufPtr, sizeof(PktBufPtr), true);

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileTransmit(&DS_AppData.FileStatus[0]));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_SB_AllocateMessageBuffer, 1);
    UtAssert_STUB_COUNT(CFE_MSG_Init, 1);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitBuffer, 1);
}

void DS_FileTransmit_Test_NoBuf(void)
{
    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileTransmit(&DS_AppData.FileStatus[0]));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_SB_AllocateMessageBuffer, 1);
    UtAssert_STUB_COUNT(CFE_MSG_Init, 0);
    UtAssert_STUB_COUNT(CFE_SB_TimeStampMsg, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitBuffer, 0);
}

void UtTest_Setup(void)
{
    UT_DS_TEST_ADD(DS_FileStorePacket_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileStorePacket_Test_PacketNotInTable);
    UT_DS_TEST_ADD(DS_FileStorePacket_Test_PassedFilterFalse);
    UT_DS_TEST_ADD(DS_FileStorePacket_Test_DisabledDest);
    UT_DS_TEST_ADD(DS_FileStorePacket_Test_InvalidIndex);

    UT_DS_TEST_ADD(DS_FileSetupWrite_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileSetupWrite_Test_FileHandleClosed);
    UT_DS_TEST_ADD(DS_FileSetupWrite_Test_MaxFileSizeExceeded);

    UT_DS_TEST_ADD(DS_FileWriteData_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileWriteData_Test_Error);

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_DS_TEST_ADD(DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal);
    UT_DS_TEST_ADD(DS_FileWriteHeader_Test_PrimaryHeaderError);
    UT_DS_TEST_ADD(DS_FileWriteHeader_Test_SecondaryHeaderError);
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_DS_TEST_ADD(DS_FileWriteError_Test);
#endif

    UT_DS_TEST_ADD(DS_FileCreateDest_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileCreateDest_Test_StringTerminate);
    UT_DS_TEST_ADD(DS_FileCreateDest_Test_NominalRollover);
    UT_DS_TEST_ADD(DS_FileCreateDest_Test_Error);

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_DS_TEST_ADD(DS_FileCreateDest_Test_ClosedFileHandle);
#endif

    UT_DS_TEST_ADD(DS_FileCreateName_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_NominalWithSeparator);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_NominalWithPeriod);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_EmptyPath);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_Error);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_PathBaseSeqTooLarge);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_PathBaseSeqExtTooLarge);
    UT_DS_TEST_ADD(DS_FileCreateName_Test_ExtensionZero);

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_DS_TEST_ADD(DS_FileCreateSequence_Test_ByCount);
    UT_DS_TEST_ADD(DS_FileCreateSequence_Test_ByTime);
    UT_DS_TEST_ADD(DS_FileCreateSequence_Test_BadFilenameType);
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_DS_TEST_ADD(DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal);
    UT_DS_TEST_ADD(DS_FileUpdateHeader_Test_WriteError);
    UT_DS_TEST_ADD(DS_FileUpdateHeader_Test_PlatformConfigCFE_SeekError);
#endif

#if (DS_MOVE_FILES == true)
    UT_DS_TEST_ADD(DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal);
    UT_DS_TEST_ADD(DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError);
    UT_DS_TEST_ADD(DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge);
    UT_DS_TEST_ADD(DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameNull);
#else
    UT_DS_TEST_ADD(DS_FileCloseDest_Test_MoveFilesFalse);
#endif

    UT_DS_TEST_ADD(DS_FileTestAge_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileTestAge_Test_ExceedMaxAge);
    UT_DS_TEST_ADD(DS_FileTestAge_Test_NullTable);

    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_AlgX0);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_AlgN0);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_AlgNGreaterX);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_Alg0GreaterX);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_Alg0EqualX);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_InvalidFilterType);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_SeqFilter);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_TimeFilter1);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_TimeFilter2);
    UT_DS_TEST_ADD(DS_IsPacketFiltered_Test_TimeFilter3);

    UT_DS_TEST_ADD(DS_FileTransmit_Test_Nominal);
    UT_DS_TEST_ADD(DS_FileTransmit_Test_NoBuf);
}
