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
 * Auto-Generated stub implementations for functions defined in ds_file header
 */

#include "ds_file.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileCloseDest()
 * ----------------------------------------------------
 */
void DS_FileCloseDest(int32 FileIndex)
{
    UT_GenStub_AddParam(DS_FileCloseDest, int32, FileIndex);

    UT_GenStub_Execute(DS_FileCloseDest, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileCreateDest()
 * ----------------------------------------------------
 */
void DS_FileCreateDest(uint32 FileIndex)
{
    UT_GenStub_AddParam(DS_FileCreateDest, uint32, FileIndex);

    UT_GenStub_Execute(DS_FileCreateDest, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileCreateName()
 * ----------------------------------------------------
 */
void DS_FileCreateName(uint32 FileIndex)
{
    UT_GenStub_AddParam(DS_FileCreateName, uint32, FileIndex);

    UT_GenStub_Execute(DS_FileCreateName, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileCreateSequence()
 * ----------------------------------------------------
 */
void DS_FileCreateSequence(char *Buffer, uint32 Type, uint32 Count)
{
    UT_GenStub_AddParam(DS_FileCreateSequence, char *, Buffer);
    UT_GenStub_AddParam(DS_FileCreateSequence, uint32, Type);
    UT_GenStub_AddParam(DS_FileCreateSequence, uint32, Count);

    UT_GenStub_Execute(DS_FileCreateSequence, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileSetupWrite()
 * ----------------------------------------------------
 */
void DS_FileSetupWrite(int32 FileIndex, const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(DS_FileSetupWrite, int32, FileIndex);
    UT_GenStub_AddParam(DS_FileSetupWrite, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(DS_FileSetupWrite, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileStorePacket()
 * ----------------------------------------------------
 */
void DS_FileStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(DS_FileStorePacket, CFE_SB_MsgId_t, MessageID);
    UT_GenStub_AddParam(DS_FileStorePacket, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(DS_FileStorePacket, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileTestAge()
 * ----------------------------------------------------
 */
void DS_FileTestAge(uint32 ElapsedSeconds)
{
    UT_GenStub_AddParam(DS_FileTestAge, uint32, ElapsedSeconds);

    UT_GenStub_Execute(DS_FileTestAge, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileTransmit()
 * ----------------------------------------------------
 */
void DS_FileTransmit(DS_AppFileStatus_t *FileStatus)
{
    UT_GenStub_AddParam(DS_FileTransmit, DS_AppFileStatus_t *, FileStatus);

    UT_GenStub_Execute(DS_FileTransmit, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileUpdateHeader()
 * ----------------------------------------------------
 */
void DS_FileUpdateHeader(int32 FileIndex)
{
    UT_GenStub_AddParam(DS_FileUpdateHeader, int32, FileIndex);

    UT_GenStub_Execute(DS_FileUpdateHeader, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileWriteData()
 * ----------------------------------------------------
 */
void DS_FileWriteData(int32 FileIndex, const void *FileData, size_t DataLength)
{
    UT_GenStub_AddParam(DS_FileWriteData, int32, FileIndex);
    UT_GenStub_AddParam(DS_FileWriteData, const void *, FileData);
    UT_GenStub_AddParam(DS_FileWriteData, size_t, DataLength);

    UT_GenStub_Execute(DS_FileWriteData, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileWriteError()
 * ----------------------------------------------------
 */
void DS_FileWriteError(uint32 FileIndex, size_t DataLength, int32 WriteResult)
{
    UT_GenStub_AddParam(DS_FileWriteError, uint32, FileIndex);
    UT_GenStub_AddParam(DS_FileWriteError, size_t, DataLength);
    UT_GenStub_AddParam(DS_FileWriteError, int32, WriteResult);

    UT_GenStub_Execute(DS_FileWriteError, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_FileWriteHeader()
 * ----------------------------------------------------
 */
void DS_FileWriteHeader(int32 FileIndex)
{
    UT_GenStub_AddParam(DS_FileWriteHeader, int32, FileIndex);

    UT_GenStub_Execute(DS_FileWriteHeader, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_IsPacketFiltered()
 * ----------------------------------------------------
 */
bool DS_IsPacketFiltered(CFE_MSG_Message_t *MessagePtr, uint16 FilterType, uint16 Algorithm_N, uint16 Algorithm_X,
                         uint16 Algorithm_O)
{
    UT_GenStub_SetupReturnBuffer(DS_IsPacketFiltered, bool);

    UT_GenStub_AddParam(DS_IsPacketFiltered, CFE_MSG_Message_t *, MessagePtr);
    UT_GenStub_AddParam(DS_IsPacketFiltered, uint16, FilterType);
    UT_GenStub_AddParam(DS_IsPacketFiltered, uint16, Algorithm_N);
    UT_GenStub_AddParam(DS_IsPacketFiltered, uint16, Algorithm_X);
    UT_GenStub_AddParam(DS_IsPacketFiltered, uint16, Algorithm_O);

    UT_GenStub_Execute(DS_IsPacketFiltered, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_IsPacketFiltered, bool);
}
