/*************************************************************************
** File: ds_file_test.c
**
** NASA Docket No. GSC-16,126-1, and identified as "Core Flight Software System
** (CFS) Data Storage Application Version 2”
**
** Copyright © 2007-2014 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration. All Rights
** Reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** Purpose:
**   This file contains unit test cases for the functions contained in the file ds_file.c
**
** References:
**   Flight Software Branch C Coding Standard Version 1.2
**   CFS Development Standards Document
**
** Notes:
**
*************************************************************************/

/*
 * Includes
 */

#include "ds_file_tests.h"
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
#include "cfs_utils.h"
/* #include "ut_utils_lib.h" */

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <sys/fcntl.h>
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
    char *             PrintBuffer = UT_Hook_GetArgValueByName(Context, "PrintBuffer", char *);
    CFE_TIME_SysTime_t TimeToPrint = UT_Hook_GetArgValueByName(Context, "TimeToPrint", CFE_TIME_SysTime_t);

    snprintf(PrintBuffer, CFE_TIME_PRINTED_STRING_SIZE, "1980-001-00:00.00.00000");
}
/*
 * Helper Functions
 *
 * This file ds_file.c has several functions which call other functions within
 * the same file.  This section includes utilities used to complete the setup
 * steps which require the called functions to succeed.
 */

void DS_SetupFileUpdateHeaderSuccess(void)
{
#if (DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE)
    UT_SetDefaultReturnValue(UT_KEY(OS_lseek), sizeof(CFE_FS_Header_t));
    UT_SetDefaultReturnValue(UT_KEY(OS_write), sizeof(CFE_TIME_SysTime_t));
#endif
}

void DS_SetupFileCloseDestSuccess(void)
{

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
#if (DS_MOVE_FILES == true)
    UT_SetDefaultReturnValue(UT_KEY(OS_mv), OS_SUCCESS);
#endif
}

/*
 * Function Definitions
 */

uint8 call_count_CFE_EVS_SendEvent;

void DS_FileStorePacket_Test_Nominal(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     MessageID = 0x18BB;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                = &HashLink;
    HashLink.Index                                           = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID             = 0x18BB;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O = 0;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType  = 1;
    DS_AppData.FileStatus[0].FileState                       = DS_ENABLED;

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &CmdBuf);

    /* Verify results */
    UtAssert_True(DS_AppData.PassedPktCounter == 1, "DS_AppData.PassedPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);
} /* end DS_FileStorePacket_Test_Nominal */

void DS_FileStorePacket_Test_PacketNotInTable(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     MessageID = 0x0001;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;

    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
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
    DS_FileStorePacket(MessageID, &CmdBuf);

    /* Verify results */
    UtAssert_INT32_EQ(DS_AppData.IgnoredPktCounter, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_PacketNotInTable */

void DS_FileStorePacket_Test_PassedFilterFalse(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     MessageID = 0x18BB;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                = &HashLink;
    HashLink.Index                                           = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID             = 0x18BB;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType  = 1;
    DS_AppData.FileStatus[0].FileState                       = DS_ENABLED;

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsPacketFiltered), true);

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &CmdBuf);

    /* Verify results */
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_PassedFilterFalse */

void DS_FileStorePacket_Test_DisabledDest(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     MessageID = 0x18BB;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                = &HashLink;
    HashLink.Index                                           = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID             = 0x18BB;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType  = 1;
    DS_AppData.FileStatus[0].FileState                       = DS_DISABLED;

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsPacketFiltered), true);

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &CmdBuf);

    /* Verify results */
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_DisabledDest */

void DS_FileStorePacket_Test_InvalidIndex(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     MessageID = 0x18BB;
    DS_FilterTable_t   FilterTable;
    DS_DestFileTable_t DestFileTable;
    DS_HashLink_t      HashLink;
    size_t             forced_Size                     = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID                    = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode                  = DS_NOOP_CC;
    uint8              call_count_CFS_IsPacketFiltered = 0;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    DS_AppData.FilterTblPtr   = &FilterTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[0].MaxFileSize = 10;

    DS_AppData.HashTable[187]                                   = &HashLink;
    HashLink.Index                                              = 0;
    DS_AppData.FilterTblPtr->Packet[0].MessageID                = 0x18BB;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_N    = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_X    = 3;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].Algorithm_O    = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FilterTblPtr->Packet[0].Filter[0].FileTableIndex = DS_DEST_FILE_CNT;
    DS_AppData.FileStatus[0].FileState                          = DS_ENABLED;

    UT_SetDefaultReturnValue(UT_KEY(CFS_IsPacketFiltered), true);

    /* Execute the function being tested */
    DS_FileStorePacket(MessageID, &CmdBuf);

    /* Verify results */
    UtAssert_True(DS_AppData.FilteredPktCounter == 1, "DS_AppData.FilteredPktCounter == 1");
    call_count_CFS_IsPacketFiltered = UT_GetStubCount(UT_KEY(CFS_IsPacketFiltered));
    UtAssert_INT32_EQ(call_count_CFS_IsPacketFiltered, 0);
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileStorePacket_Test_InvalidIndex */

void DS_FileSetupWrite_Test_Nominal(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    DS_AppData.FileStatus[FileIndex].FileSize              = 3;

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &CmdBuf); //(CFE_SB_Buffer_t)&CmdPacket);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileSetupWrite_Test_Nominal */

void DS_FileSetupWrite_Test_FileHandleClosed(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t     forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t  forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    DS_AppData.FileStatus[FileIndex].FileHandle = DS_CLOSED_FILE_HANDLE;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 100;
    DS_AppData.FileStatus[FileIndex].FileSize              = 3;

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &CmdBuf); //(CFE_SB_Buffer_t)&CmdPacket);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);
    /* Generates 1 event message we don't care about in this test */
} /* end DS_FileSetupWrite_Test_FileHandleClosed */

void DS_FileSetupWrite_Test_MaxFileSizeExceeded(void)
{
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    size_t             forced_Size = sizeof(DS_NoopCmd_t);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;
    DS_AppData.DestFileTblPtr                   = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileSize = 5;
    DS_AppData.FileStatus[FileIndex].FileSize              = 10;

    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/", DS_TOTAL_FNAME_BUFSIZE);

    /* Set to prevent an error message that we don't care about in this test */
    //   UT_SetHandlerFunction(UT_KEY(CFE_FS_WriteHeader), &UT_CFE_FS_WriteHeader_SuccessHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_FS_WriteHeader), sizeof(CFE_FS_Header_t));

#if (DS_MOVE_FILES == true)
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/", DS_PATHNAME_BUFSIZE);
#endif

    /* Execute the function being tested */
    DS_FileSetupWrite(FileIndex, &CmdBuf); //(CFE_SB_Buffer_t)&CmdPacket);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileSetupWrite_Test_MaxFileSizeExceeded */

void DS_FileWriteData_Test_Nominal(void)
{
    CFE_SB_Buffer_t   CmdBuf;
    CFE_SB_MsgId_t    FileIndex      = 0;
    uint32            DataLength     = TestDataLength;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Set to return DataLength to satisfy condition "if (Result == DataLength)" */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_DataHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), DataLength);
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &CmdBuf, DataLength); //(CFE_SB_Buffer_t)&CmdPacket, DataLength);

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
    CFE_SB_Buffer_t    CmdBuf;
    CFE_SB_MsgId_t     FileIndex  = 0;
    uint32             DataLength = 10;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr        = &DestFileTable;
    size_t            forced_Size    = sizeof(DS_NoopCmd_t);
    CFE_SB_MsgId_t    forced_MsgID   = DS_CMD_MID;
    CFE_MSG_FcnCode_t forced_CmdCode = DS_NOOP_CC;

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &forced_MsgID, sizeof(forced_MsgID), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &forced_Size, sizeof(forced_Size), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &forced_CmdCode, sizeof(forced_CmdCode), false);

    /* Set to reach error case being tested (DS_FileWriteError) */
    // UT_SetHandlerFunction(UT_KEY(OS_write), &UT_OS_write_FailHandler, NULL);
    UT_SetDefaultReturnValue(UT_KEY(OS_write), -1);
    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/", DS_TOTAL_FNAME_BUFSIZE);

    /* Execute the function being tested */
    DS_FileWriteData(FileIndex, &CmdBuf, DataLength); //(CFE_SB_Buffer_t)&CmdPacket, DataLength);

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
    CFE_SB_MsgId_t     FileIndex = 0;
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
    CFE_SB_MsgId_t     FileIndex = 0;
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
    CFE_SB_MsgId_t     FileIndex = 0;
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
    CFE_SB_MsgId_t     FileIndex   = 0;
    uint32             DataLength  = 10;
    int32              WriteResult = -1;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = 1;

    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", OS_MAX_PATH_LEN);

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
    osal_id_t          LocalFileHandle = 5;
    osal_id_t *        LocalFileHdlPtr = &LocalFileHandle;

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Setup for DS_FileCreateName to succeed */
    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;

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
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle != 99,
                  "DS_AppData.FileStatus[FileIndex].FileHandle != 99");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle != DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle != DS_CLOSED_FILE_HANDLE");

    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 2);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_TableUpdateCDS)), 1);

} /* end DS_FileCreateDest_Test_Nominal */

void DS_FileCreateDest_Test_NominalRollover(void)
{
    uint32             FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    osal_id_t          LocalFileHandle = 5;
    osal_id_t *        LocalFileHdlPtr = &LocalFileHandle;

    CFE_EVS_SendEvent_context_t context_CFE_EVS_SendEvent;
    UT_SetHookFunction(UT_KEY(CFE_EVS_SendEvent), UT_Utils_stub_reporter_hook, &context_CFE_EVS_SendEvent);

    /* Setup for DS_FileCreateName to succeed */
    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileCount = DS_MAX_SEQUENCE_COUNT;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;

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
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle != 99,
                  "DS_AppData.FileStatus[FileIndex].FileHandle != 99");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle != DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle != DS_CLOSED_FILE_HANDLE");

    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 3);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

    UtAssert_INT32_EQ(UT_GetStubCount(UT_KEY(DS_TableUpdateCDS)), 1);

} /* end DS_FileCreateDest_Test_NominalRollover */

void DS_FileCreateDest_Test_Error(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;
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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    osal_id_t          LocalFileHandle = DS_CLOSED_FILE_HANDLE;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].SequenceCount = 5;

    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "filename", OS_MAX_PATH_LEN);

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    DS_AppData.FileStatus[FileIndex].FileHandle             = 99;
    DS_AppData.FileStatus[FileIndex].FileCount              = 1;

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
    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileHandle, DS_CLOSED_FILE_HANDLE);
    UtAssert_INT32_EQ(DS_AppData.FileStatus[FileIndex].FileCount, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCreateDest_Test_ClosedFileHandle */

void DS_FileCreateName_Test_Nominal(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_True(
        strncmp(DS_AppData.FileStatus[FileIndex].FileName, "path/basename.extension", DS_TOTAL_FNAME_BUFSIZE) == 0,
        "strncmp (DS_AppData.FileStatus[FileIndex].FileName, 'path/basename.extension', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_Nominal */

void DS_FileCreateName_Test_NominalWithSeparator(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path/", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_True(
        strncmp(DS_AppData.FileStatus[FileIndex].FileName, "path/basename.extension", DS_TOTAL_FNAME_BUFSIZE) == 0,
        "strncmp (DS_AppData.FileStatus[FileIndex].FileName, 'path/basename.extension', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_NominalWithSeparator */

void DS_FileCreateName_Test_NominalWithPeriod(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, ".extension", OS_MAX_PATH_LEN);

    DS_AppData.FileStatus[FileIndex].FileCount = 1;

    /* Execute the function being tested */
    DS_FileCreateName(FileIndex);

    /* Verify results */
    UtAssert_True(
        strncmp(DS_AppData.FileStatus[FileIndex].FileName, "path/basename.extension", DS_TOTAL_FNAME_BUFSIZE) == 0,
        "strncmp (DS_AppData.FileStatus[FileIndex].FileName, 'path/basename.extension', DS_TOTAL_FNAME_BUFSIZE) == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileCreateName_Test_NominalWithPeriod */

void DS_FileCreateName_Test_EmptyPath(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Basename, "basename", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;
    int32              i;
    char               EventMessage[300];

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.DestFileTblPtr->File[FileIndex].FileNameType = DS_BY_COUNT;
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Pathname, "path", OS_MAX_PATH_LEN);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Extension, "extension", OS_MAX_PATH_LEN);

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

#if DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE
void DS_FileCreateSequence_Test_ByCount(void)
{
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

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
    CFE_SB_MsgId_t     FileIndex = 0;
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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

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
    CFE_SB_MsgId_t FileIndex = 0;

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
    CFE_SB_MsgId_t FileIndex = 0;

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
    CFE_SB_MsgId_t FileIndex = 0;

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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename", DS_TOTAL_FNAME_BUFSIZE);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/", DS_PATHNAME_BUFSIZE);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE");
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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filename", DS_TOTAL_FNAME_BUFSIZE);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/", DS_PATHNAME_BUFSIZE);

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
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE");
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
    CFE_SB_MsgId_t     FileIndex = 0;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    strncpy(DS_AppData.FileStatus[FileIndex].FileName, "directory1/filenamefilenamefilenamefilenamefilenamefilename",
            DS_TOTAL_FNAME_BUFSIZE);
    strncpy(DS_AppData.DestFileTblPtr->File[FileIndex].Movename, "directory2/movename/", DS_PATHNAME_BUFSIZE);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    /*  UtAssert_True
          (Ut_CFE_EVS_EventSent(DS_MOVE_FILE_ERR_EID, CFE_EVS_ERROR, "FILE MOVE error: dir name = 'directory2/movename',
       filename = '/filenamefilenamefilenamefilenamefilenamefilename'"), "FILE MOVE error: dir name =
       'directory2/movename', filename = '/filenamefilenamefilen'");
  */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileSize == 0, "DS_AppData.FileStatus[FileIndex].FileSize == 0");
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileName[0] == 0,
                  "DS_AppData.FileStatus[FileIndex].FileName[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 1);

} /* end DS_FileCloseDest_Test_PlatformConfigMoveFiles_FilenameTooLarge */
#endif

#if (DS_MOVE_FILES == false)
void DS_FileCloseDest_Test_MoveFilesFalse(void)
{
    CFE_SB_MsgId_t FileIndex = 0;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);

    /* Execute the function being tested */
    DS_FileCloseDest(FileIndex);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE,
                  "DS_AppData.FileStatus[FileIndex].FileHandle == DS_CLOSED_FILE_HANDLE");
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
    CFE_SB_MsgId_t     FileIndex      = 0;
    uint32             ElapsedSeconds = 2;
    DS_DestFileTable_t DestFileTable;
    uint32             i;

    UT_SetHandlerFunction(UT_KEY(OS_close), &UT_OS_CLOSE_SuccessHandler, NULL);
    DS_AppData.DestFileTblPtr = &DestFileTable;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        strncpy(DS_AppData.FileStatus[i].FileName, "directory1/filename", DS_TOTAL_FNAME_BUFSIZE);
#if (DS_MOVE_FILES == true)
        strncpy(DS_AppData.DestFileTblPtr->File[i].Movename, "", DS_PATHNAME_BUFSIZE);
#endif
    }

    DS_AppData.FileStatus[FileIndex].FileHandle           = 99;
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
    CFE_SB_MsgId_t     FileIndex      = 0;
    uint32             ElapsedSeconds = 2;
    DS_DestFileTable_t DestFileTable;
    uint32             i;
    uint8              call_count_OS_close = 0;
    DS_AppData.DestFileTblPtr              = NULL;

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
    CFE_SB_MsgId_t     FileIndex      = 0;
    uint32             ElapsedSeconds = 2;
    DS_DestFileTable_t DestFileTable;

    DS_AppData.DestFileTblPtr = &DestFileTable;

    DS_AppData.FileStatus[FileIndex].FileHandle = 99;
    for (int32 i = 1; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.FileStatus[i].FileHandle = DS_CLOSED_FILE_HANDLE;
    }
    DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge  = 1;
    DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] = '\0';

    DS_SetupFileUpdateHeaderSuccess();
    DS_SetupFileCloseDestSuccess();

    /* Execute the function being tested */
    DS_FileTestAge(ElapsedSeconds);

    /* Verify results */
    UtAssert_True(DS_AppData.FileStatus[FileIndex].FileAge == 0, "DS_AppData.FileStatus[FileIndex].FileAge == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_INT32_EQ(call_count_CFE_EVS_SendEvent, 0);

} /* end DS_FileTestAge_Test_ExceedMaxAge */

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
#else
    UtTest_Add(DS_FileCloseDest_Test_MoveFilesFalse, DS_Test_Setup, DS_Test_TearDown,
               "DS_FileCloseDest_Test_MoveFilesFalse");
#endif

    UtTest_Add(DS_FileTestAge_Test_Nominal, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_Nominal");
    UtTest_Add(DS_FileTestAge_Test_ExceedMaxAge, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_ExceedMaxAge");
    UtTest_Add(DS_FileTestAge_Test_NullTable, DS_Test_Setup, DS_Test_TearDown, "DS_FileTestAge_Test_NullTable");
} /* end DS_File_Test_AddTestCases */

/************************/
/*  End of File Comment */
/************************/
