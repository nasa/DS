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

int32 TestDataLength = 10;

/* Some minimal hooks for OS functions for which the usual stubs have too much
 * implementation */
void UT_OS_CLOSE_SuccessHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // UT_Stub_SetReturnValue(FuncKey, OS_SUCCESS);
    int32 status = OS_SUCCESS;
    UT_Stub_GetInt32StatusCode(Context, &status);
} /* end UT_OS_CLOSE_SuccessHandler */

void UT_OS_CLOSE_FailHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // return OS_ERROR;
    int32 status = OS_ERROR;
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_OS_CLOSE_FailHandler */

void UT_CFE_FS_WriteHeader_FailHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // return -1;
    int32 status = -1;
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_CFE_FS_WriteHeader_FailHandler */

void UT_CFE_FS_WriteHeader_SuccessHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // return sizeof(CFE_FS_Header_t);
    int32 status = sizeof(CFE_FS_Header_t);
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_CFE_FS_WriteHeader_SuccessHandler */

void UT_OS_write_FailHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // return -1;
    int32 status = -1;
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_OS_write_FailHandler */

void UT_OS_write_SuccessHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    // return sizeof(DS_FileHeader_t);
    int32 status = sizeof(DS_FileHeader_t);
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_OS_write_SuccessHandler */

void UT_OS_write_DataHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    //  return 10;
    int32 status = TestDataLength;
    UT_Stub_GetInt32StatusCode(Context, &status);

} /* end UT_OS_write_DataHandler */

void UT_CFE_TIME_Print_CustomHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    char *PrintBuffer = UT_Hook_GetArgValueByName(Context, "PrintBuffer", char *);

    snprintf(PrintBuffer, CFE_TIME_PRINTED_STRING_SIZE, "1980-001-00:00.00.00000");
}
/*
 * Helper Functions
 *
 * This file ds_file.c has several functions which call other functions within
 * the same file.  This section includes utilities used to complete the setup
 * steps which require the called functions to succeed.
 */

void UT_DS_SetupFileUpdateHeaderSuccess(void)
{
#if (DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE)
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(CFE_TIME_SysTime_t));
#endif
}

void UT_DS_SetupFileCloseDestSuccess(void)
{

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
#if (DS_MOVE_FILES == true)
    UT_SetDefaultReturnValue(UT_KEY(OS_mv), OS_SUCCESS);
#endif
}

void UT_DS_SetDestFileEntry(DS_DestFileEntry_t *DestFileEntryPtr)
{
    strncpy(DestFileEntryPtr->Pathname, "path", sizeof(DestFileEntryPtr->Pathname));
    strncpy(DestFileEntryPtr->Basename, "base", sizeof(DestFileEntryPtr->Basename));
    strncpy(DestFileEntryPtr->Extension, "ext", sizeof(DestFileEntryPtr->Extension));
}

/*
 * Function Definitions
 */

uint8 call_count_CFE_EVS_SendEvent;

void DS_FileStorePacket_Test_Nominal(void)
{
    CFE_SB_MsgId_t          MessageID = DS_UT_MID_1;
    DS_FilterTable_t        FilterTable;
    DS_DestFileTable_t      DestFileTable;
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

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = forced_Size * 2;

    for (int i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].Algorithm_N    = DS_UNUSED;
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].FileTableIndex = 0;
    }

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
    UtAssert_True(DS_AppData.PassedPktCounter == 1, "DS_AppData.PassedPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_FileStorePacket_Test_Nominal */

void DS_FileStorePacket_Test_PacketNotInTable(void)
{
    CFE_SB_MsgId_t     MessageID = DS_UT_MID_1;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;

    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187] = &HashLink;
    HashLink.Index            = 0;

    UT_SetDefaultReturnValue(UT_KEY(DS_TableFindMsgID), DS_INDEX_NONE);

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.IgnoredPktCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_PacketNotInTable */

void DS_FileStorePacket_Test_PassedFilterFalse(void)
{
    CFE_SB_MsgId_t          MessageID = DS_UT_MID_1;
    DS_FilterTable_t        FilterTable;
    DS_DestFileTable_t      DestFileTable;
    DS_HashLink_t           HashLink;
    size_t                  forced_Size     = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t          forced_MsgID    = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t       forced_CmdCode  = DS_NOOP_CC;
    CFE_MSG_SequenceCount_t forced_SeqCount = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSequenceCount), &forced_SeqCount, sizeof(forced_SeqCount), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    for (int i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].Algorithm_N    = DS_UNUSED;
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].FileTableIndex = 0;
    }

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
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_PassedFilterFalse */

void DS_FileStorePacket_Test_DisabledDest(void)
{
    CFE_SB_MsgId_t     MessageID = DS_UT_MID_1;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (int i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].Algorithm_N    = DS_UNUSED;
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].FileTableIndex = 0;
    }
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
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_DisabledDest */

void DS_FileStorePacket_Test_InvalidIndex(void)
{
    CFE_SB_MsgId_t     MessageID = DS_UT_MID_1;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (int i = 0; i < DS_FILTERS_PER_PACKET; i++)
    {
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].Algorithm_N    = DS_UNUSED;
        DS_AppData.FilterTblPtr->Packet[0].Filter[i].FileTableIndex = 0;
    }

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
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_InvalidIndex */

void DS_FileSetupWrite_Test_Nominal(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    DS_AppData.FileStatus[FileIndex].FileSize              = 3;

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileSetupWrite_Test_Nominal */

void DS_FileSetupWrite_Test_FileHandleClosed(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    UT_SetHandlerFunction(UT_KEY(OS_OpenCreate), &UT_OS_write_SuccessHandler, NULL);

    DS_AppData.DestFileTblPtr                              = &DestFileTable;
    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle             = OS_OBJECT_ID_UNDEFINED;
    DS_AppData.FileStatus[FileIndex].FileCount              = 0;
    DS_AppData.FileStatus[FileIndex].FileSize               = 3;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(DS_FileWriteData, 0);
    UtAssert_STUB_COUNT(OS_OpenCreate, 1);
} /* end DS_FileSetupWrite_Test_FileHandleClosed */

void DS_FileSetupWrite_Test_MaxFileSizeExceeded(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size = sizeof(DS_NoopCmd_t);

    memset(&DestFileTable, 0, sizeof(DestFileTable));

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;
    DS_AppData.DestFileTblPtr                   = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 5;
    DS_AppData.FileStatus[FileIndex].FileSize              = 10;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    /* Set to prevent an error message that we don't care about in this test */
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

#if (DS_MOVE_FILES == true)
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));
#endif

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &UT_CmdBuf.Buf);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileSetupWrite_Test_MaxFileSizeExceeded */

void DS_FileWriteData_Test_Nominal(void)
{
    int32             FileIndex      = 0;
    uint32            DataLength     = TestDataLength;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Set to return DataLength to satisfy condition "if (Result == DataLength)" */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_DataHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), DataLength);
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &UT_CmdBuf.Buf, DataLength);

    /* Verify results */
    UtAssert_True(DS_AppData.FileWriteCounter == 1, "DS_AppData.FileWriteCounter == 1");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 10, "DS_AppData.FileStatus[FileIndex].FileSize == 10");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileGrowth == 10,
                  "DS_AppData.FileStatus[FileIndex].FileGrowth == 10");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileWriteData_Test_Nominal */

void DS_FileWriteData_Test_Error(void)
{
    int32              FileIndex  = 0;
    uint32             DataLength = 10;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr        = &DestFileTable;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = CFE_SB_ValueToMsgId(DS_CMD_MID);
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Set to reach error case being tested (DS_FileWriteError) */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_FailHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &UT_CmdBuf.Buf, DataLength);

    /* Verify results */
    /*UtAssert_True
        (Ut_CFE_EVS_EventSent(DS_WRITE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE WRITE error: result = -1, length = 10, dest =
       0, name = 'directory1/'"), "FILE WRITE error: result = -1, length = 10, dest = 0, name = 'directory1/'");
*/
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileWriteData_Test_Error */

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    /* Set to satisfy condition "if (Result == sizeof(CFE_FS_Header_t))" */
    // UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "if (Result == sizeof(DS_FileHeader_t))" */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_SuccessHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(DS_FileHeader_t));

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_True(DS_AppData.FileWriteCounter == 2, "DS_AppData.FileWriteCounter == 2");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t),
                  "sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t)");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileGrowth == sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t),
                  "sizeof(CFE_FS_Header_t) + sizeof(DS_FileHeader_t)");

} /* end DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_PrimaryHeaderError(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable[2];

    DS_AppData.DestFileTblPtr = DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0]  = '\0';
    /* Set to generate primary header error */
    // UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_FailHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), -1);

    /* Handle closure in response to the error */
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */

} /* end DS_FileWriteHeader_Test_PrimaryHeaderError */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteHeader_Test_SecondaryHeaderError(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';
    /* Set to satisfy condition "if (Result == sizeof(CFE_FS_Header_t))" */
    // UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

    /* Handle closure in response to the error */
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Set to generate secondary header error */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_FailHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    DS_FileWriteHeader(FileIndex);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */

} /* end DS_FileWriteHeader_Test_SecondaryHeaderError */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileWriteError_Test(void)
{
    int32              FileIndex   = 0;
    uint32             DataLength  = 10;
    int32              WriteResult = -1;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileWriteError(FileIndex, DataLength, WriteResult);

    /* Verify results */
    UtAssert_True(DS_AppData.FileWriteErrCounter == 1, "DS_AppData.FileWriteErrCounter == 1");
    /*
        UtAssert_True
            (Ut_CFE_EVS_EventSent(DS_WRITE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE WRITE error: result = -1, length = 10,
       dest = 0, name = 'filename'"), "FILE WRITE error: result = -1, length = 10, dest = 0, name = 'filename'");
    */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED,
                  "DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileWriteError_Test */
#endif

void DS_FileCreateDest_Test_Nominal(void)
{
    uint32             FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    /* Setup for DS_FileCreateName to succeed */
    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.FileStatus[FileIndex].FileCount  = 1;
    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    /* Setup for DS_FileWriteHeader to succeed */
#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(DS_FileHeader_t));
#endif

    /* Set to fail the condition "if (Result < 0)" */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);

    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_SuccessHandler, NULL);
#endif

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

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_TableUpdateCDS)), 1);

} /* end DS_FileCreateDest_Test_Nominal */

void DS_FileCreateDest_Test_StringTerminate(void)
{
    uint32             FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    /* Setup for DS_FileCreateName to succeed */
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.FileStatus[FileIndex].FileName[0] = DS_STRING_TERMINATOR;

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateDest(FileIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(OS_OpenCreate, 0);

} /* end DS_FileCreateDest_Test_StringTerminate */

void DS_FileCreateDest_Test_NominalRollover(void)
{
    uint32             FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    /* Setup for DS_FileCreateName to succeed */
    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.FileStatus[FileIndex].FileCount  = DS_MAX_SEQUENCE_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;

    /* Setup for DS_FileWriteHeader to succeed */
#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(DS_FileHeader_t));
#endif

    /* Set to fail the condition "if (Result < 0)" */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);

    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_SuccessHandler, NULL);
#endif

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

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_TableUpdateCDS)), 1);

} /* end DS_FileCreateDest_Test_NominalRollover */

void DS_FileCreateDest_Test_Error(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;
    DS_AppData.FileStatus[FileIndex].FileCount  = DS_MAX_SEQUENCE_COUNT + 1;

    /* Set to generate error message DS_CREATE_FILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), -1);

    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileWriteErrCounter == 1, "DS_AppData.FileWriteErrCounter == 1");

    /* For this test it doesn't matter what filename results from the call to DS_FileCreateName. */
    /* UtAssert_True
         (Ut_CFE_EVS_EventSent(DS_CREATE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE CREATE error: result = -1, dest = 0, name =
       'path/basename.extension'"), "FILE CREATE error: result = -1, dest = 0, name = 'path/basename.extension'");
 */
    UtAssert_True(strncmp(DS_AppData.FileStatus[FileIndex].FileName, "", DS_TOTAL_FNAME_BUFSIZE) == 0,
                  "strncmp (DS_AppData.FileStatus[FileIndex].FileName, "
                  ", DS_TOTAL_FNAME_BUFSIZE) == 0");

    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileState, DS_DISABLED);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCreateDest_Test_Error */

void DS_FileCreateDest_Test_ClosedFileHandle(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    osal_id_t          LocalFileHandle = OS_OBJECT_ID_UNDEFINED;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].SequenceCount = 5;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", sizeof(DS_AppData.FileStatus[FileIndex].FileName));

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle             = DS_UT_OBJID_1;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    /* Set to fail the condition "if (Result < 0)" */
    UT_SetDefaultReturnValue(UT_KEY(OS_OpenCreate), OS_SUCCESS);
    UT_SetDataBuffer(UT_KEY(OS_OpenCreate), &LocalFileHandle, sizeof(osal_id_t), false);
    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);

    /* Set to prevent error messages in subfunction DS_FileWriteHeader that we don't care about in this test */
    UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileCreateDest(FileIndex);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.FileWriteCounter, 1);
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCreateDest_Test_ClosedFileHandle */

void DS_FileCreateName_Test_Nominal(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    char               StrCompare[] = "path/base00000001.ext";

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_STRINGBUF_EQ(DS_AppData.FileStatus[FileIndex].FileName, sizeof(DS_AppData.FileStatus[FileIndex].FileName),
                          StrCompare, sizeof(StrCompare));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_Nominal */

void DS_FileCreateName_Test_NominalWithSeparator(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    char               StrCompare[] = "path/base00000001.ext";

    DS_AppData.DestFileTblPtr = &DestFileTable;

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

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_NominalWithSeparator */

void DS_FileCreateName_Test_NominalWithPeriod(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    char               StrCompare[] = "path/base00000001.ext";

    DS_AppData.DestFileTblPtr = &DestFileTable;

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

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_NominalWithPeriod */

void DS_FileCreateName_Test_EmptyPath(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);
    DS_AppData.DestFileTblPtr->File[FileIndex].Pathname[0] = '\0';

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    /*   UtAssert_True
           (Ut_CFE_EVS_EventSent(DS_FILE_CREATE_EMPTY_PATH_ERR_EID, CFE_EVS_ERROR, "FILE NAME error: Path empty. dest =
       0, path = ''"), "FILE NAME error: Path empty. dest = 0, path = ''");
   */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED,
                  "DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCreateName_Test_EmptyPath */

void DS_FileCreateName_Test_Error(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    int32              i;
    char               EventMessage[300];

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    for (i = 0; i < DS_TOTAL_FNAME_BUFSIZE - 2; i++)
    {
        DS_AppData.DestFileTblPtr->File[FileIndex].Basename[i] = 'a';
    }

    DS_AppData.DestFileTblPtr->File[FileIndex].Basename[DS_TOTAL_FNAME_BUFSIZE - 1] = DS_STRING_TERMINATOR;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    sprintf(EventMessage, "FILE NAME error: dest = 0, path = 'path', base = '%s', seq = ",
            DS_AppData.DestFileTblPtr->File[FileIndex].Basename);
    /*UtAssert_True
        (Ut_CFE_EVS_EventSent(DS_FILE_NAME_ERR_EID, CFE_EVS_ERROR, EventMessage),
        EventMessage);
*/
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED,
                  "DS_AppData.FileStatus[FileIndex].FileState == DS_DISABLED");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCreateName_Test_Error */

void DS_FileCreateName_Test_MaxPathnameLength(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    int32              WorknameLen = 2 * DS_TOTAL_FNAME_BUFSIZE;
    int32              i;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_DS_SetDestFileEntry(&DS_AppData.DestFileTblPtr->File[FileIndex]);

    /* Set to fail the condition "if (TotalLength != (WorknameLen - 1))" */
    for (i = 0; i < WorknameLen - 1; i++)
    {
        DS_AppData.DestFileTblPtr->File[FileIndex].Pathname[i] = 'a';
    }

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_INT32_EQ(strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname), WorknameLen - 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);

} /* end DS_FileCreateName_Test_MaxPathnameLength */

void DS_FileCreateName_Test_PathBaseSeqTooLarge(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    /* Set to fail the condition "if ((strlen(Workname) + strlen(Sequence)) < DS_TOTAL_FNAME_BUFSIZE)" */
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "PathnamePathnamePathnamePat",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "BasenameBasenameBasenameBase",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Basename));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_INT32_EQ(strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname) +
                          strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Basename) + DS_SEQUENCE_DIGITS +
                          strlen("/"),
                      DS_TOTAL_FNAME_BUFSIZE);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);

} /* end DS_FileCreateName_Test_PathBaseSeqTooLarge */

void DS_FileCreateName_Test_PathBaseSeqExtTooLarge(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

    /* Set to fail the condition "if (strlen(Workname) < DS_TOTAL_FNAME_BUFSIZE)" */
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "PathnamePathnamePathname",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "BasenameBasenameBasenameBase",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Basename));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "ext",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Extension));

    /* Execute the function being tested */
    UtAssert_VOIDCALL(DS_FileCreateName(FileIndex));

    /* Verify results */
    UtAssert_INT32_EQ(strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname) +
                          strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Basename) +
                          strlen(DS_AppData.DestFileTblPtr->File[FileIndex].Extension) + DS_SEQUENCE_DIGITS +
                          strlen("/"),
                      DS_TOTAL_FNAME_BUFSIZE);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FILE_NAME_ERR_EID);

} /* end DS_FileCreateName_Test_PathBaseSeqExtTooLarge */

void DS_FileCreateName_Test_ExtensionZero(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    char               StrCompare[] = "path/base00000001";

    DS_AppData.DestFileTblPtr = &DestFileTable;

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

} /* end DS_FileCreateName_Test_ExtensionZero */

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_ByCount(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

    memset(Sequence, 0, sizeof(Sequence));

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateSequence(Sequence, DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType,
                          DS_AppData.FileStatus[FileIndex].FileCount);

    /* Verify results */
    UtAssert_True(strncmp(Sequence, "00000001", DS_TOTAL_FNAME_BUFSIZE) == 0,
                  "strncmp(Sequence, '00000001', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateSequence_Test_ByCount */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
/**
 * TODO: Need to find the correct way to force the output of CFE_TIME_GetTime.
 */
void DS_FileCreateSequence_Test_ByTime(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    CFE_TIME_SysTime_t FakeTime;

    char timeBuf[CFE_TIME_PRINTED_STRING_SIZE];

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

    snprintf(timeBuf, sizeof(timeBuf), "1980-001-00:00:00.00000");

    FakeTime.Seconds    = 0;
    FakeTime.Subseconds = 0;
    UT_SetDataBuffer(UT_KEY(CFE_TIME_GetTime), &FakeTime, sizeof(FakeTime), false);

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_TIME;

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    UT_SetHandlerFunction(UT_KEY(CFE_TIME_Print), &UT_CFE_TIME_Print_CustomHandler, NULL);
    // UT_SetDataBuffer(UT_KEY(CFE_TIME_Print), &timeBuf, sizeof(timeBuf), false);

    /* Execute the function being tested */
    DS_FileCreateSequence(Sequence, DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType,
                          DS_AppData.FileStatus[FileIndex].FileCount);

    /* Verify results */
    printf("sequence = %s\n", Sequence);
    UtAssert_True(strncmp(Sequence, "1980001000000", DS_TOTAL_FNAME_BUFSIZE) == 0,
                  "strncmp(Sequence, '1980001000000', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateSequence_Test_ByTime */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_BadFilenameType(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

    memset(Sequence, 0xFF, sizeof(Sequence));

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_TIME;

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateSequence(Sequence, 99, DS_AppData.FileStatus[FileIndex].FileCount);

    /* Verify results */
    UtAssert_True(strncmp(Sequence, "", DS_TOTAL_FNAME_BUFSIZE) == 0,
                  "strncmp(Sequence, '', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateSequence_Test_BadFilenameType */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal(void)
{
    int32 FileIndex = 0;

    /* Set to satisfy condition "if (Result == sizeof(CFE_FS_Header_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t));

    /* Set to satisfy condition "if (Result == sizeof(CFE_TIME_SysTime_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(CFE_TIME_SysTime_t));

    /* Execute the function being tested */
    DS_FileUpdateHeader(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileUpdateCounter == 1, "DS_AppData.FileUpdateCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal */
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileUpdateHeader_Test_WriteError(void)
{
    int32 FileIndex = 0;

    /* Set to satisfy condition "if (Result == sizeof(CFE_FS_Header_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t));

    /* Set to fail condition "if (Result == sizeof(CFE_TIME_SysTime_t))" */
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);

    /* Execute the function being tested */
    DS_FileUpdateHeader(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileUpdateErrCounter == 1, "DS_AppData.FileUpdateErrCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileUpdateHeader_Test_WriteError */
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
    UtAssert_True(DS_AppData.FileUpdateErrCounter == 1, "DS_AppData.FileUpdateErrCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileUpdateHeader_Test_PlatformConfigCFE_SeekError */
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 0, "DS_AppData.FileStatus[FileIndex].FileSize == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileName[0] == 0,
                  "DS_AppData.FileStatus[FileIndex].FileName[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal */
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Set to generate error message DS_MOVE_FILE_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(OS_mv), -1);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    /*   UtAssert_True
           (Ut_CFE_EVS_EventSent(DS_MOVE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE MOVE error: src = 'directory1/filename', tgt
       = 'directory2/movename/filename', result = -1"), "FILE MOVE error: src = 'directory1/filename', tgt =
       'directory2/movename/filename', result = -1");
   */
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 0, "DS_AppData.FileStatus[FileIndex].FileSize == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileName[0] == 0,
                  "DS_AppData.FileStatus[FileIndex].FileName[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError */
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filenamefilenamefilenamefilenamefilenamefilename",
            sizeof(DS_AppData.FileStatus[FileIndex].FileName));
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/",
            sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    /*  UtAssert_True
          (Ut_CFE_EVS_EventSent(DS_MOVE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE MOVE error: dir name = 'directory2/movename',
       filename = '/filenamefilenamefilenamefilenamefilenamefilename'"), "FILE MOVE error: dir name =
       'directory2/movename', filename = '/filenamefilenamefilen'");
  */
    UtAssert_BOOL_FALSE(OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle));
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 0, "DS_AppData.FileStatus[FileIndex].FileSize == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileName[0] == 0,
                  "DS_AppData.FileStatus[FileIndex].FileName[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge */
#endif

#if (DS_MOVE_FILES == true)
void DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameNull(void)
{
    int32              FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    DS_AppData.DestFileTblPtr = &DestFileTable;

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

} /* end DS_FileCloseDest_Test_PlatformConfigMoveFiles_Null */
#endif

#if (DS_MOVE_FILES == false)
void DS_FileCloseDest_Test_MoveFilesFalse(void)
{
    int32 FileIndex = 0;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle == OS_OBJECT_ID_UNDEFINED,
                  "DS_AppData.FileStatus[FileIndex].FileHandle == OS_OBJECT_ID_UNDEFINED");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 0, "DS_AppData.FileStatus[FileIndex].FileSize == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileName[0] == 0,
                  "DS_AppData.FileStatus[FileIndex].FileName[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCloseDest_Test_MoveFilesFalse */
#endif

void DS_FileTestAge_Test_Nominal(void)
{
    int32              FileIndex      = 0;
    uint32             ElapsedSeconds = 2;
    DS_DestFileTable_t DestFileTable;
    uint32             i;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        strncpy(DS_AppData.FileStatus[i].FileName, "directory1/filename", sizeof(DS_AppData.FileStatus[i].FileName));
#if (DS_MOVE_FILES == true)
        DS_AppData.DestFileTblPtr->File[i].Movename[0] = '\0';
#endif

        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }

    DS_AppData.FileStatus[FileIndex].FileHandle           = DS_UT_OBJID_1;
    DS_AppData.FileStatus[FileIndex].FileAge              = 0;
    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge = 3;

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 2, "DS_AppData.FileStatus[FileIndex].FileAge == 2");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileTestAge_Test_Nominal */

void DS_FileTestAge_Test_NullTable(void)
{
    uint32 ElapsedSeconds      = 2;
    uint8  call_count_OS_close = 0;
    DS_AppData.DestFileTblPtr  = NULL;

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    call_count_OS_close = UT_GetStubCount(UT_KEY(OS_close));
    UtAssert_INT32_EQ(call_count_OS_close, 0);
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileTestAge_Test_NullTable */

void DS_FileTestAge_Test_ExceedMaxAge(void)
{
    int32              FileIndex      = 0;
    uint32             ElapsedSeconds = 2;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_UT_OBJID_1;
    for (int32 i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = OS_OBJECT_ID_UNDEFINED;
    }
    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge  = 1;
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    UT_DS_SetupFileUpdateHeaderSuccess();
    UT_DS_SetupFileCloseDestSuccess();

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileTestAge_Test_ExceedMaxAge */

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
    UtTest_Add(DS_FileStorePacket_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileStorePacket_Test_Nominal");
    UtTest_Add(DS_FileStorePacket_Test_PacketNotInTable, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileStorePacket_Test_PacketNotInTable");
    UtTest_Add(DS_FileStorePacket_Test_PassedFilterFalse, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileStorePacket_Test_PassedFilterFalse");
    UtTest_Add(DS_FileStorePacket_Test_DisabledDest, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileStorePacket_Test_DisabledDest");
    UtTest_Add(DS_FileStorePacket_Test_InvalidIndex, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileStorePacket_Test_InvalidIndex");

    UtTest_Add(DS_FileSetupWrite_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileSetupWrite_Test_Nominal");
    UtTest_Add(DS_FileSetupWrite_Test_FileHandleClosed, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileSetupWrite_Test_FileHandleClosed");
    UtTest_Add(DS_FileSetupWrite_Test_MaxFileSizeExceeded, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileSetupWrite_Test_MaxFileSizeExceeded");

    UtTest_Add(DS_FileWriteData_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileWriteData_Test_Nominal");
    UtTest_Add(DS_FileWriteData_Test_Error, DS_Test_Setup, DS_Test_TearDown, "DS_FileWriteData_Test_Error");

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtTest_Add(DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileWriteHeader_Test_PlatformConfigCFE_Nominal");
    UtTest_Add(DS_FileWriteHeader_Test_PrimaryHeaderError, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileWriteHeader_Test_PrimaryHeaderError");
    UtTest_Add(DS_FileWriteHeader_Test_SecondaryHeaderError, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileWriteHeader_Test_SecondaryHeaderError");
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtTest_Add(DS_FileWriteError_Test, DS_Test_Setup, DS_Test_TearDown, "DS_FileWriteError_Test");
#endif

    UtTest_Add(DS_FileCreateDest_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateDest_Test_Nominal");
    UtTest_Add(DS_FileCreateDest_Test_StringTerminate, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateDest_Test_StringTerminate");
    UtTest_Add(DS_FileCreateDest_Test_NominalRollover, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateDest_Test_NominalRollover");
    UtTest_Add(DS_FileCreateDest_Test_Error, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateDest_Test_Error");

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtTest_Add(DS_FileCreateDest_Test_ClosedFileHandle, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateDest_Test_ClosedFileHandle");
#endif

    UtTest_Add(DS_FileCreateName_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateName_Test_Nominal");
    UtTest_Add(DS_FileCreateName_Test_NominalWithSeparator, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_NominalWithSeparator");
    UtTest_Add(DS_FileCreateName_Test_NominalWithPeriod, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_NominalWithPeriod");
    UtTest_Add(DS_FileCreateName_Test_EmptyPath, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateName_Test_EmptyPath");
    UtTest_Add(DS_FileCreateName_Test_Error, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateName_Test_Error");
    UtTest_Add(DS_FileCreateName_Test_MaxPathnameLength, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_MaxPathnameLength");
    UtTest_Add(DS_FileCreateName_Test_PathBaseSeqTooLarge, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_PathBaseSeqTooLarge");
    UtTest_Add(DS_FileCreateName_Test_PathBaseSeqExtTooLarge, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_PathBaseSeqExtTooLarge");
    UtTest_Add(DS_FileCreateName_Test_ExtensionZero, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateName_Test_ExtensionZero");

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtTest_Add(DS_FileCreateSequence_Test_ByCount, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateSequence_Test_ByCount");
    UtTest_Add(DS_FileCreateSequence_Test_ByTime, DS_Test_Setup, DS_Test_TearDown, "DS_FileCreateSequence_Test_ByTime");
    UtTest_Add(DS_FileCreateSequence_Test_BadFilenameType, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCreateSequence_Test_BadFilenameType");
#endif

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
    UtTest_Add(DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileUpdateHeader_Test_PlatformConfigCFE_Nominal");
    UtTest_Add(DS_FileUpdateHeader_Test_WriteError, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileUpdateHeader_Test_WriteError");
    UtTest_Add(DS_FileUpdateHeader_Test_PlatformConfigCFE_SeekError, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileUpdateHeader_Test_PlatformConfigCFE_SeekError");
#endif

#if (DS_MOVE_FILES == true)
    UtTest_Add(DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_PlatformConfigMoveFiles_Nominal");
    UtTest_Add(DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_PlatformConfigMoveFiles_MoveError");
    UtTest_Add(DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge");
    UtTest_Add(DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameNull, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameNull");
#else
    UtTest_Add(DS_FileCloseDest_Test_MoveFilesFalse, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_MoveFilesFalse");
#endif

    UtTest_Add(DS_FileTestAge_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_Nominal");
    UtTest_Add(DS_FileTestAge_Test_ExceedMaxAge, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_ExceedMaxAge");
    UtTest_Add(DS_FileTestAge_Test_NullTable, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_NullTable");

    UtTest_Add(DS_IsPacketFiltered_Test_AlgX0, DS_Test_Setup, DS_Test_TearDown, "DS_IsPacketFiltered_Test_AlgX0");
    UtTest_Add(DS_IsPacketFiltered_Test_AlgN0, DS_Test_Setup, DS_Test_TearDown, "DS_IsPacketFiltered_Test_AlgN0");
    UtTest_Add(DS_IsPacketFiltered_Test_AlgNGreaterX, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_AlgNGreaterX");
    UtTest_Add(DS_IsPacketFiltered_Test_Alg0GreaterX, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_Alg0GreaterThanX");
    UtTest_Add(DS_IsPacketFiltered_Test_Alg0EqualX, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_Alg0EqualX");
    UtTest_Add(DS_IsPacketFiltered_Test_InvalidFilterType, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_InvalidFilterType");
    UtTest_Add(DS_IsPacketFiltered_Test_SeqFilter, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_SeqFilter");
    UtTest_Add(DS_IsPacketFiltered_Test_TimeFilter1, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_TimeFilter1");
    UtTest_Add(DS_IsPacketFiltered_Test_TimeFilter2, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_TimeFilter2");
    UtTest_Add(DS_IsPacketFiltered_Test_TimeFilter3, DS_Test_Setup, DS_Test_TearDown,
               "DS_IsPacketFiltered_Test_TimeFilter3");

    UtTest_Add(DS_FileTransmit_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileTransmit_Test_Nominal");
    UtTest_Add(DS_FileTransmit_Test_NoBuf, DS_Test_Setup, DS_Test_TearDown, "DS_FileTransmit_Test_NoBuf");

} /* end DS_File_Test_AddTestCases */

/************************/
/*  End of File Comment */
/************************/
