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
 *   This file contains unit test cases for the functions contained in the file ds_table.c
 */

/*
 * Includes
 */

#include "ds_app.h"
#include "ds_appdefs.h"
#include "ds_table.h"
#include "ds_msg.h"
#include "ds_msgdefs.h"
#include "ds_msgids.h"
#include "ds_eventids.h"
#include "ds_version.h"
#include "ds_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

/*
 * Function Definitions
 */

void DS_TableInit_Test_Nominal(void)
{
    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableInit_Test_TableInfoRecovered(void)
{
    /* Set to generate both of the two error messages DS_INIT_TBL_CDS_EID  */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Register), CFE_TBL_INFO_RECOVERED_TBL);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_CDS_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_INIT_TBL_CDS_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_DEBUG);
}

void DS_TableInit_Test_RegisterDestTableError(void)
{
    /* Set to generate first instance of error message DS_INIT_TBL_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Register), 0x99);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), 0x99);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableInit_Test_RegisterFilterTableError(void)
{
    /* Set to generate second instance of error message DS_INIT_TBL_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 2, 0x99);

    /* Set to prevent unintended error messages */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), 0x99);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableInit_Test_LoadDestTableError(void)
{
    /* Fail on the first load (loading the dest table */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableInit_Test_LoadFilterTableError(void)
{
    /* Set to generate error message DS_INIT_TBL_ERR_EID on 2nd call (but not 1st) */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Load), 2, -1);

    /* Setting addresses to NULL will exercise table manage address NULL but unmatched GetAddress cases */
    DS_AppData.DestFileTblPtr = NULL;
    DS_AppData.FilterTblPtr   = NULL;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableInit(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableManageDestFile_Test_TableInfoUpdated(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    /* Reset table pointer to NULL, but cause the CFE_TBL_GetAddress handler to provide back the ut value */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &DS_AppData.DestFileTblPtr, sizeof(DS_AppData.DestFileTblPtr), true);
    DS_AppData.DestFileTblPtr = NULL;

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.DestTblLoadCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileState, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileCount, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState, DS_DEST_FILE_CNT / 2);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount, DS_DEST_FILE_CNT / 2);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState, DS_DEST_FILE_CNT - 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount, DS_DEST_FILE_CNT - 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageDestFile_Test_TableNeverLoaded(void)
{
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_ERR_NEVER_LOADED);
    DS_AppData.DestFileTblPtr = NULL;

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.DestTblErrCounter, 1);
    UtAssert_ADDRESS_EQ(DS_AppData.DestFileTblPtr, NULL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageDestFile_Test_TableInfoDumpPending(void)
{
    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_DUMP_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_DUMP_PENDING);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageDestFile_Test_TableInfoValidationPending(void)
{
    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_VALIDATION_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_VALIDATION_PENDING);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageDestFile_Test_TableInfoUpdatePending(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_UPDATE_PENDING);
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.DestTblLoadCounter, 1);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileState, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileCount, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState, DS_DEST_FILE_CNT / 2);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount, DS_DEST_FILE_CNT / 2);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState, DS_DEST_FILE_CNT - 1);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount, DS_DEST_FILE_CNT - 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageDestFile_Test_TableSuccess(void)
{
    uint32 i;

    for (i = 0; i < DS_DEST_FILE_CNT; i++)
    {
        DS_AppData.DestFileTblPtr->File[i].EnableState   = i;
        DS_AppData.DestFileTblPtr->File[i].SequenceCount = i;
    }

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    DS_TableManageDestFile();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.DestTblLoadCounter, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileState, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileCount, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileState, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount, 0);

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileState, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableInfoUpdated(void)
{
    /* Set handle back to NULL, but cause CFE_TBL_GetAddress to return valid address */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &DS_AppData.FilterTblPtr, sizeof(DS_AppData.FilterTblPtr), true);
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilterTblLoadCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableNeverLoaded(void)
{
    /* Set to satisfy condition "if (Result == CFE_TBL_ERR_NEVER_LOADED)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_ERR_NEVER_LOADED);
    DS_AppData.FilterTblPtr = NULL;

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilterTblErrCounter, 1);
    UtAssert_ADDRESS_EQ(DS_AppData.FilterTblPtr, NULL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableInfoDumpPending(void)
{
    /* Set to satisfy condition "if (Result == CFE_TBL_INFO_DUMP_PENDING)" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_DUMP_PENDING);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableInfoValidationPending(void)
{
    /* Set to satisfy condition "CFE_TBL_INFO_VALIDATION_PENDING" */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_VALIDATION_PENDING);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableInfoUpdatePending(void)
{
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_TBL_INFO_UPDATE_PENDING);
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilterTblLoadCounter, 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableManageFilter_Test_TableSuccess(void)
{
    /* Returns CFE_TBL_INFO_UPDATED to satisfy condition "if (Result == CFE_TBL_INFO_UPDATE_PENDING)", and sets
     * DS_AppData.DestFileTblPtr to the address of a local table defined globally in this file, to prevent segmentation
     * fault */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetStatus), CFE_SUCCESS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);

    /* Execute the function being tested */
    DS_TableManageFilter();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.FilterTblLoadCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFile_Test_Nominal(void)
{
    DS_DestFileTable_t DestFileTable;

    memset(&DestFileTable, 0, sizeof(DestFileTable));

    strncpy(DestFileTable.File[0].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Extension, "ext", DS_EXTENSION_BUFSIZE);

    DestFileTable.File[0].FileNameType  = DS_BY_COUNT;
    DestFileTable.File[0].EnableState   = DS_DISABLED;
    DestFileTable.File[0].MaxFileSize   = DS_FILE_MIN_SIZE_LIMIT;
    DestFileTable.File[0].MaxFileAge    = DS_FILE_MIN_AGE_LIMIT;
    DestFileTable.File[0].SequenceCount = DS_MAX_SEQUENCE_COUNT - 1;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyDestFile(&DestFileTable), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyDestFile_Test_DestFileTableVerificationError(void)
{
    DS_DestFileTable_t DestFileTable;
    uint32             i;

    memset(&DestFileTable, 0, sizeof(DestFileTable));

    memset(&DestFileTable.File[0], 1, sizeof(DestFileTable.File[0]));
    DestFileTable.File[0].FileNameType  = DS_BY_TIME;
    DestFileTable.File[0].EnableState   = DS_ENABLED;
    DestFileTable.File[0].MaxFileSize   = 2048;
    DestFileTable.File[0].MaxFileAge    = 100;
    DestFileTable.File[0].SequenceCount = 1;

    strncpy(DestFileTable.File[0].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Extension, "ext", DS_EXTENSION_BUFSIZE);

    for (i = 0; i < DS_DESCRIPTOR_BUFSIZE; i++)
    {
        DestFileTable.Descriptor[i] = '*';
    }

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyDestFile(&DestFileTable), DS_TABLE_VERIFY_ERR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FIL_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyDestFile_Test_CountBad(void)
{
    DS_DestFileTable_t DestFileTable;

    memset(&DestFileTable, 0, sizeof(DestFileTable));

    strncpy(DestFileTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    DestFileTable.File[0].FileNameType  = DS_BY_TIME;
    DestFileTable.File[0].EnableState   = DS_ENABLED;
    DestFileTable.File[0].MaxFileSize   = 2048;
    DestFileTable.File[0].MaxFileAge    = 100;
    DestFileTable.File[0].SequenceCount = 1;

    strncpy(DestFileTable.File[0].Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileTable.File[0].Extension, "1234567", DS_EXTENSION_BUFSIZE);

    memset(&DestFileTable.File[0], 1, sizeof(DestFileTable.File[0]));

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyDestFile(&DestFileTable), DS_TABLE_VERIFY_ERR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    /* this generates 1 event message we don't care about for this test */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FIL_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyDestFileEntry_Test_NominalErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFileEntry_Test_InvalidPathnameErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "***", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "basename", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidBasenameErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "***", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = 99;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = 99;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = DS_FILE_MIN_SIZE_LIMIT - 1;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = DS_FILE_MIN_AGE_LIMIT - 1;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 0;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FIL_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrNonZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = 99;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrNonZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = 99;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFileEntry_Test_InvalidSizeErrNonZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = DS_FILE_MIN_SIZE_LIMIT - 1;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFileEntry_Test_InvalidAgeErrNonZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = DS_FILE_MIN_AGE_LIMIT - 1;
    DestFileEntry.SequenceCount = 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrNonZero(void)
{
    DS_DestFileEntry_t DestFileEntry;
    uint32             TableIndex = 0;
    uint32             ErrorCount = 1;

    DestFileEntry.FileNameType  = DS_BY_TIME;
    DestFileEntry.EnableState   = DS_ENABLED;
    DestFileEntry.MaxFileSize   = 2048;
    DestFileEntry.MaxFileAge    = 100;
    DestFileEntry.SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    strncpy(DestFileEntry.Pathname, "path", DS_PATHNAME_BUFSIZE);
    strncpy(DestFileEntry.Basename, "pathname", DS_BASENAME_BUFSIZE);
    strncpy(DestFileEntry.Extension, "ext", DS_EXTENSION_BUFSIZE);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyDestFileEntry(&DestFileEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFilter_Test_Nominal(void)
{
    DS_FilterTable_t FilterTable;
    uint32           i;

    memset(&FilterTable, 0, sizeof(FilterTable));

    FilterTable.Packet[0].MessageID                = DS_UT_MID_1;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    FilterTable.Packet[0].Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    strncpy(FilterTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = CFE_SB_INVALID_MSG_ID;
    }

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyFilter(&FilterTable), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyFilter_Test_FilterTableVerificationError(void)
{
    DS_FilterTable_t FilterTable;
    uint32           i;

    memset(&FilterTable, 0, sizeof(FilterTable));

    FilterTable.Packet[0].MessageID                = DS_UT_MID_1;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    for (i = 0; i < DS_DESCRIPTOR_BUFSIZE; i++)
    {
        FilterTable.Descriptor[i] = '*';
    }

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = CFE_SB_INVALID_MSG_ID;
    }

    FilterTable.Packet[0].Filter[0].FilterType = 3;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyFilter(&FilterTable), DS_TABLE_VERIFY_ERR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FLT_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyFilter_Test_CountBad(void)
{
    DS_FilterTable_t FilterTable;
    uint32           i;

    memset(&FilterTable, 0, sizeof(FilterTable));

    FilterTable.Packet[0].MessageID                = DS_UT_MID_1;
    FilterTable.Packet[0].Filter[0].FileTableIndex = 0;
    FilterTable.Packet[0].Filter[0].Algorithm_N    = 1;
    FilterTable.Packet[0].Filter[0].Algorithm_X    = 3;
    FilterTable.Packet[0].Filter[0].Algorithm_O    = 0;
    FilterTable.Packet[0].Filter[0].FilterType     = DS_BY_TIME * 2;
    DS_AppData.FileStatus[0].FileState             = DS_ENABLED;

    strncpy(FilterTable.Descriptor, "descriptor", DS_DESCRIPTOR_BUFSIZE);

    for (i = 1; i < 256; i++)
    {
        FilterTable.Packet[i].MessageID = CFE_SB_INVALID_MSG_ID;
    }

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableVerifyFilter(&FilterTable), DS_TABLE_VERIFY_ERR);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    /* this generates 1 event message we don't care about for this test */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, DS_FLT_TBL_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);
}

void DS_TableVerifyFilterEntry_Test_Unused(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 0;
    PacketEntry.Filter[0].Algorithm_X    = 0;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFilterEntry_Test_Nominal(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 0;
    PacketEntry.Filter[0].Algorithm_X    = 0;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = DS_DEST_FILE_CNT + 1;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 99;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 0;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 99;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_FLT_TBL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = DS_DEST_FILE_CNT + 1;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 0;
    PacketEntry.Filter[0].FilterType     = 99;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero(void)
{
    DS_PacketEntry_t PacketEntry;
    uint32           TableIndex = 0;
    uint32           ErrorCount = 1;

    memset(&PacketEntry, 0, sizeof(PacketEntry));

    PacketEntry.MessageID                = DS_UT_MID_1;
    PacketEntry.Filter[0].FileTableIndex = 0;
    PacketEntry.Filter[0].Algorithm_N    = 1;
    PacketEntry.Filter[0].Algorithm_X    = 3;
    PacketEntry.Filter[0].Algorithm_O    = 99;
    PacketEntry.Filter[0].FilterType     = 1;
    DS_AppData.FileStatus[0].FileState   = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFilterEntry(&PacketEntry, TableIndex, ErrorCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableEntryUnused_Test_Nominal(void)
{
    DS_DestFileEntry_t DestFileEntry;

    memset(&DestFileEntry, DS_UNUSED, sizeof(DestFileEntry));

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableEntryUnused(&DestFileEntry, sizeof(DestFileEntry)));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableEntryUnused_Test_Fail(void)
{
    DS_DestFileEntry_t DestFileEntry;

    memset(&DestFileEntry, 99, sizeof(DestFileEntry));

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableEntryUnused(&DestFileEntry, sizeof(DestFileEntry)));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFileIndex_Test_Nominal(void)
{
    uint16 FileTableIndex = 0;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyFileIndex(FileTableIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyFileIndex_Test_Fail(void)
{
    uint16 FileTableIndex = DS_DEST_FILE_CNT;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyFileIndex(FileTableIndex));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyParms_Test_NominalOnlyXNonZero(void)
{
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 1;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero(void)
{
    uint16 Algorithm_N = 1;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero(void)
{
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 1;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyParms_Test_AllZero(void)
{
    uint16 Algorithm_N = 0;
    uint16 Algorithm_X = 0;
    uint16 Algorithm_O = 0;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyParms(Algorithm_N, Algorithm_X, Algorithm_O));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyType_Test_Nominal(void)
{
    uint16 TimeVsCount = DS_BY_TIME;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyType(TimeVsCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyType_Test_Fail(void)
{
    uint16 TimeVsCount = 99;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyType(TimeVsCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyState_Test_NominalEnabled(void)
{
    uint16 EnableState = DS_ENABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyState(EnableState));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyState_Test_NominalDisabled(void)
{
    uint16 EnableState = DS_DISABLED;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyState(EnableState));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyState_Test_Fail(void)
{
    uint16 EnableState = 99;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyState(EnableState));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifySize_Test_Nominal(void)
{
    size_t MaxFileSize = DS_FILE_MIN_SIZE_LIMIT;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifySize(MaxFileSize));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifySize_Test_Fail(void)
{
    size_t MaxFileSize = DS_FILE_MIN_SIZE_LIMIT - 1;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifySize(MaxFileSize));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyAge_Test_Nominal(void)
{
    uint32 MaxFileAge = DS_FILE_MIN_AGE_LIMIT;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyAge(MaxFileAge));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyAge_Test_Fail(void)
{
    uint32 MaxFileAge = DS_FILE_MIN_AGE_LIMIT - 1;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyAge(MaxFileAge));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyCount_Test_Nominal(void)
{
    uint32 SequenceCount = DS_MAX_SEQUENCE_COUNT;

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(DS_TableVerifyCount(SequenceCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableVerifyCount_Test_Fail(void)
{
    uint32 SequenceCount = DS_MAX_SEQUENCE_COUNT + 1;

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(DS_TableVerifyCount(SequenceCount));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableSubscribe_Test_Unused(void)
{
    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_SubscribeEx, 0);
}

void DS_TableSubscribe_Test_Cmd(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = CFE_SB_ValueToMsgId(DS_CMD_MID);

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_SubscribeEx, 0);
}

void DS_TableSubscribe_Test_SendHk(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_SubscribeEx, 0);
}

void DS_TableSubscribe_Test_Data(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = DS_UT_MID_1; /* NOT the CMD or SEND_HK MIDs */

    /* Execute the function being tested */
    DS_TableSubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_SubscribeEx, 1);
}

void DS_TableUnsubscribe_Test_Unused(void)
{
    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 0);
}

void DS_TableUnsubscribe_Test_Cmd(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = CFE_SB_ValueToMsgId(DS_CMD_MID);

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 0);
}

void DS_TableUnsubscribe_Test_SendHk(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = CFE_SB_ValueToMsgId(DS_SEND_HK_MID);

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 0);
}

void DS_TableUnsubscribe_Test_Data(void)
{
    DS_AppData.FilterTblPtr->Packet[0].MessageID = DS_UT_MID_1; /* NOT the CMD or SEND_HK MIDs */

    /* Execute the function being tested */
    DS_TableUnsubscribe();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 1);
}

void DS_TableCreateCDS_Test_NewCDSArea(void)
{
    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableCreateCDS(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableCreateCDS_Test_PreExistingCDSArea(void)
{
    /* Set to satisfy condition "if (Result == CFE_ES_CDS_ALREADY_EXISTS)", which is the main thing we're testing here
     */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), CFE_ES_CDS_ALREADY_EXISTS);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableCreateCDS(), CFE_SUCCESS);

    /* Verify results */

    UtAssert_UINT32_EQ(DS_AppData.FileStatus[0].FileCount, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT / 2].FileCount, 0);
    UtAssert_UINT32_EQ(DS_AppData.FileStatus[DS_DEST_FILE_CNT - 1].FileCount, 0);

    if (DS_CDS_ENABLE_STATE == 1)
    {
        /* only test if configured */
        UtAssert_UINT32_EQ(DS_AppData.AppEnableState, 0);
    }

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableCreateCDS_Test_RestoreFail(void)
{
    /* Set to satisfy condition "if (Result == CFE_ES_CDS_ALREADY_EXISTS)", which is the main thing we're testing here
     */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), CFE_ES_CDS_ALREADY_EXISTS);
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RestoreFromCDS), -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableCreateCDS(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_CDS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableCreateCDS_Test_Error(void)
{
    /* Set to generate error message DS_INIT_CDS_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_RegisterCDS), -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableCreateCDS(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(DS_AppData.DataStoreHandle, CFE_ES_CDS_BAD_HANDLE));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_CDS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableUpdateCDS_Test_Nominal(void)
{
    DS_AppData.DataStoreHandle = DS_UT_CDSHANDLE_1;

    /* Execute the function being tested */
    DS_TableUpdateCDS();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableUpdateCDS_Test_Error(void)
{
    DS_AppData.DataStoreHandle = DS_UT_CDSHANDLE_1;

    /* Set to generate error message DS_INIT_CDS_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_CopyToCDS), -1);

    /* Execute the function being tested */
    DS_TableUpdateCDS();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, DS_INIT_CDS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void DS_TableHashFunction_Test(void)
{
    CFE_SB_MsgId_t MessageID = DS_UT_MID_1;

    /* Execute the function being tested */
    UtAssert_UINT32_LT(DS_TableHashFunction(MessageID), DS_HASH_TABLE_ENTRIES);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableCreateHash_Test_Nominal(void)
{
    uint32 HashIndex;

    DS_AppData.FilterTblPtr->Packet[0].MessageID = DS_UT_MID_1;
    HashIndex                                    = DS_TableHashFunction(DS_AppData.FilterTblPtr->Packet[0].MessageID);

    /* Execute the function being tested */
    DS_TableCreateHash();

    /* Verify results */
    UtAssert_UINT32_EQ(DS_AppData.HashLinks[0].Index, 0);
    UtAssert_BOOL_TRUE(CFE_SB_MsgId_Equal(DS_AppData.HashLinks[0].MessageID, DS_UT_MID_1));
    UtAssert_ADDRESS_EQ(DS_AppData.HashTable[HashIndex], &DS_AppData.HashLinks[0]);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableFindMsgID_Test(void)
{
    CFE_SB_MsgId_t MessageID = DS_UT_MID_1;
    DS_HashLink_t  HashLink;
    uint32         HashIndex;

    HashIndex                                                 = DS_TableHashFunction(MessageID);
    DS_AppData.HashTable[HashIndex]                           = &HashLink;
    HashLink.Index                                            = 1;
    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = MessageID;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableFindMsgID(MessageID), HashLink.Index);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableFindMsgID_Test_NullTable(void)
{
    CFE_SB_MsgId_t MessageID = DS_UT_MID_1;

    for (int i = 0; i < DS_HASH_TABLE_ENTRIES; i++)
    {
        DS_AppData.HashTable[i] = NULL;
    }

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableFindMsgID(MessageID), DS_INDEX_NONE);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void DS_TableFindMsgID_Test_Mismatch(void)
{
    CFE_SB_MsgId_t MessageID = DS_UT_MID_1;
    DS_HashLink_t  HashLink;
    uint32         HashIndex;

    HashIndex                       = DS_TableHashFunction(MessageID);
    DS_AppData.HashTable[HashIndex] = &HashLink;

    HashLink.Index = 1;
    HashLink.Next  = NULL;

    DS_AppData.FilterTblPtr->Packet[HashLink.Index].MessageID = DS_UT_MID_2;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(DS_TableFindMsgID(MessageID), DS_INDEX_NONE);

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void UtTest_Setup(void)
{
    UT_DS_TEST_ADD(DS_TableInit_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableInit_Test_TableInfoRecovered);
    UT_DS_TEST_ADD(DS_TableInit_Test_RegisterDestTableError);
    UT_DS_TEST_ADD(DS_TableInit_Test_RegisterFilterTableError);
    UT_DS_TEST_ADD(DS_TableInit_Test_LoadDestTableError);
    UT_DS_TEST_ADD(DS_TableInit_Test_LoadFilterTableError);

    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableInfoUpdated);
    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableNeverLoaded);
    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableInfoDumpPending);
    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableInfoValidationPending);
    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableInfoUpdatePending);
    UT_DS_TEST_ADD(DS_TableManageDestFile_Test_TableSuccess);

    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableInfoUpdated);
    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableNeverLoaded);
    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableInfoDumpPending);
    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableInfoValidationPending);
    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableInfoUpdatePending);
    UT_DS_TEST_ADD(DS_TableManageFilter_Test_TableSuccess);

    UT_DS_TEST_ADD(DS_TableVerifyDestFile_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyDestFile_Test_CountBad);

    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_NominalErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidSizeErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidAgeErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrZero);

    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidFilenameTypeErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidFileEnableStateErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidSizeErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidAgeErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyDestFileEntry_Test_InvalidSequenceCountErrNonZero);

    UT_DS_TEST_ADD(DS_TableVerifyFilter_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyFilter_Test_FilterTableVerificationError);
    UT_DS_TEST_ADD(DS_TableVerifyFilter_Test_CountBad);

    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_Unused);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrZero);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFileTableIndexErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFilterTypeErrNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyFilterEntry_Test_InvalidFilterParmsErrNonZero);

    UT_DS_TEST_ADD(DS_TableEntryUnused_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableEntryUnused_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifyFileIndex_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyFileIndex_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifyParms_Test_NominalOnlyXNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyParms_Test_NGreaterThanXOnlyNNonZero);
    UT_DS_TEST_ADD(DS_TableVerifyParms_Test_OGreaterThanXOnlyONonZero);
    UT_DS_TEST_ADD(DS_TableVerifyParms_Test_AllZero);

    UT_DS_TEST_ADD(DS_TableVerifyType_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyType_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifyState_Test_NominalEnabled);
    UT_DS_TEST_ADD(DS_TableVerifyState_Test_NominalDisabled);
    UT_DS_TEST_ADD(DS_TableVerifyState_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifySize_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifySize_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifyAge_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyAge_Test_Fail);

    UT_DS_TEST_ADD(DS_TableVerifyCount_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableVerifyCount_Test_Fail);

    UT_DS_TEST_ADD(DS_TableSubscribe_Test_Unused);
    UT_DS_TEST_ADD(DS_TableSubscribe_Test_Cmd);
    UT_DS_TEST_ADD(DS_TableSubscribe_Test_SendHk);
    UT_DS_TEST_ADD(DS_TableSubscribe_Test_Data);

    UT_DS_TEST_ADD(DS_TableUnsubscribe_Test_Unused);
    UT_DS_TEST_ADD(DS_TableUnsubscribe_Test_Cmd);
    UT_DS_TEST_ADD(DS_TableUnsubscribe_Test_SendHk);
    UT_DS_TEST_ADD(DS_TableUnsubscribe_Test_Data);

    UT_DS_TEST_ADD(DS_TableCreateCDS_Test_NewCDSArea);
    UT_DS_TEST_ADD(DS_TableCreateCDS_Test_PreExistingCDSArea);
    UT_DS_TEST_ADD(DS_TableCreateCDS_Test_RestoreFail);
    UT_DS_TEST_ADD(DS_TableCreateCDS_Test_Error);

    UT_DS_TEST_ADD(DS_TableUpdateCDS_Test_Nominal);
    UT_DS_TEST_ADD(DS_TableUpdateCDS_Test_Error);

    UT_DS_TEST_ADD(DS_TableHashFunction_Test);

    UT_DS_TEST_ADD(DS_TableCreateHash_Test_Nominal);

    UT_DS_TEST_ADD(DS_TableFindMsgID_Test);
    UT_DS_TEST_ADD(DS_TableFindMsgID_Test_NullTable);

    UT_DS_TEST_ADD(DS_TableFindMsgID_Test_Mismatch);
}
