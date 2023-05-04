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
 *  CFS Data Storage (DS) file functions
 */

#include "cfe.h"
#include "cfe_fs.h"

#include "ds_platform_cfg.h"
#include "ds_verify.h"

#include "ds_appdefs.h"
#include "ds_msgids.h"

#include "ds_msg.h"
#include "ds_app.h"
#include "ds_file.h"
#include "ds_table.h"
#include "ds_events.h"

#include <stdio.h>

#define DS_PKT_SEQUENCE_BASED_FILTER_TYPE 1
#define DS_PKT_TIME_BASED_FILTER_TYPE     2

#define DS_16_MSB_SUBSECS_SHIFT 16
#define DS_11_LSB_SECONDS_MASK  0x07FF
#define DS_11_LSB_SECONDS_SHIFT 4
#define DS_4_MSB_SUBSECS_MASK   0xF000
#define DS_4_MSB_SUBSECS_SHIFT  12

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Apply common filter algorithm to Software Bus packet            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool DS_IsPacketFiltered(CFE_MSG_Message_t *MessagePtr, uint16 FilterType, uint16 Algorithm_N, uint16 Algorithm_X,
                         uint16 Algorithm_O)
{
    /*
    ** Algorithm_N = the filter will pass this many packets
    ** Algorithm_X = out of every group of this many packets
    ** Algorithm_O = starting at this offset within the group
    */
    bool                    PacketIsFiltered = false;
    CFE_TIME_SysTime_t      PacketTime;
    uint16                  PacketValue;
    uint16                  Seconds;
    uint16                  Subsecs;
    CFE_MSG_SequenceCount_t SeqCnt = 0;

    memset(&PacketTime, 0, sizeof(PacketTime));

    /*
    ** Verify input values (all errors = packet is filtered)...
    */
    if (Algorithm_X == 0)
    {
        /*
        ** Group size of zero will result in divide by zero...
        */
        PacketIsFiltered = true;
    }
    else if (Algorithm_N == 0)
    {
        /*
        ** Pass count of zero will result in zero packets...
        */
        PacketIsFiltered = true;
    }
    else if (Algorithm_N > Algorithm_X)
    {
        /*
        ** Pass count cannot exceed group size...
        */
        PacketIsFiltered = true;
    }
    else if (Algorithm_O >= Algorithm_X)
    {
        /*
        ** Group offset must be less than group size...
        */
        PacketIsFiltered = true;
    }
    else if ((FilterType != DS_PKT_TIME_BASED_FILTER_TYPE) && (FilterType != DS_PKT_SEQUENCE_BASED_FILTER_TYPE))
    {
        /*
        ** Invalid - unknown filter type...
        */
        PacketIsFiltered = true;
    }
    else
    {
        if (FilterType == DS_PKT_SEQUENCE_BASED_FILTER_TYPE)
        {
            /*
            ** Create packet filter value from packet sequence count...
            */
            CFE_MSG_GetSequenceCount(MessagePtr, &SeqCnt);
            PacketValue = (uint16)SeqCnt;
        }
        else
        {
            /*
            ** Create packet filter value from packet timestamp...
            */
            CFE_MSG_GetMsgTime(MessagePtr, &PacketTime);

            /*
            ** Get the least significant 11 bits of timestamp seconds...
            */
            Seconds = (uint16)PacketTime.Seconds;
            Seconds = Seconds & DS_11_LSB_SECONDS_MASK;

            /*
            ** Get the most significant 4 bits of timestamp subsecs...
            */
            Subsecs = (uint16)(PacketTime.Subseconds >> DS_16_MSB_SUBSECS_SHIFT);
            Subsecs = Subsecs & DS_4_MSB_SUBSECS_MASK;

            /*
            ** Shift seconds and subsecs to allow merge...
            */
            Seconds = Seconds << DS_11_LSB_SECONDS_SHIFT;
            Subsecs = Subsecs >> DS_4_MSB_SUBSECS_SHIFT;

            /*
            ** Merge seconds and subsecs to create packet filter value...
            */
            PacketValue = Seconds | Subsecs;
        }

        /*
        ** Apply the filter algorithm (common for both filter types)...
        */
        if (PacketValue < Algorithm_O)
        {
            /*
            ** Value is less than offset of passed range...
            */
            PacketIsFiltered = true;
        }
        else if (((PacketValue - Algorithm_O) % Algorithm_X) < Algorithm_N)
        {
            /*
            ** This packet was passed by the filter algorithm...
            */
            PacketIsFiltered = false;
        }
        else
        {
            /*
            ** This packet was filtered by the filter algorithm...
            */
            PacketIsFiltered = true;
        }
    }

    return PacketIsFiltered;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Store packet in file(s)                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{
    DS_PacketEntry_t *PacketEntry  = NULL;
    DS_FilterParms_t *FilterParms  = NULL;
    bool              PassedFilter = false;
    bool              FilterResult = false;
    int32             FilterIndex  = 0;
    int32             FileIndex    = 0;
    int32             i            = 0;

    /*
    ** Convert packet MessageID to packet filter table index...
    */
    FilterIndex = DS_TableFindMsgID(MessageID);

    /*
    ** Ignore packets not listed in the packet filter table...
    */
    if (FilterIndex == DS_INDEX_NONE)
    {
        DS_AppData.IgnoredPktCounter++;
    }
    else
    {
        PacketEntry  = &DS_AppData.FilterTblPtr->Packet[FilterIndex];
        PassedFilter = false;

        /*
        ** Each packet has multiple filters for multiple files...
        */
        for (i = 0; i < DS_FILTERS_PER_PACKET; i++)
        {
            FilterParms = &PacketEntry->Filter[i];

            /*
            ** Ignore unused and invalid filters...
            */
            if ((FilterParms->Algorithm_N != DS_UNUSED) && (FilterParms->FileTableIndex < DS_DEST_FILE_CNT))
            {
                FileIndex = FilterParms->FileTableIndex;
                /*
                ** Ignore disabled destination files...
                */
                if (DS_AppData.FileStatus[FileIndex].FileState == DS_ENABLED)
                {
                    /*
                    ** Apply filter algorithm to the packet...
                    */
                    FilterResult = DS_IsPacketFiltered((CFE_MSG_Message_t *)BufPtr, FilterParms->FilterType,
                                                       FilterParms->Algorithm_N, FilterParms->Algorithm_X,
                                                       FilterParms->Algorithm_O);
                    if (FilterResult == false)
                    {
                        /*
                        ** Write unfiltered packets to destination file...
                        */
                        DS_FileSetupWrite(FileIndex, BufPtr);
                        PassedFilter = true;
                    }
                }
            }
        }

        /*
        ** Count packet as passed if any filters passed...
        */
        if (PassedFilter)
        {
            DS_AppData.PassedPktCounter++;
        }
        else
        {
            DS_AppData.FilteredPktCounter++;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Prepare to write packet data to file                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileSetupWrite(int32 FileIndex, const CFE_SB_Buffer_t *BufPtr)
{
    DS_DestFileEntry_t *DestFile     = &DS_AppData.DestFileTblPtr->File[FileIndex];
    DS_AppFileStatus_t *FileStatus   = &DS_AppData.FileStatus[FileIndex];
    bool                OpenNewFile  = false;
    size_t              PacketLength = 0;

    /*
    ** Create local pointers for array indexed data...
    */
    CFE_MSG_GetSize(&BufPtr->Msg, &PacketLength);

    if (!OS_ObjectIdDefined(FileStatus->FileHandle))
    {
        /*
        ** 1st packet since destination enabled or file closed...
        */
        OpenNewFile = true;
    }
    else
    {
        /*
        ** Test size of existing destination file...
        */
        if ((FileStatus->FileSize + PacketLength) > DestFile->MaxFileSize)
        {
            /*
            ** This packet would cause file to exceed max size limit...
            */
            DS_FileUpdateHeader(FileIndex);
            DS_FileCloseDest(FileIndex);
            OpenNewFile = true;
        }
        else
        {
            /*
            ** File size is OK - write packet data to file...
            */
            DS_FileWriteData(FileIndex, BufPtr, PacketLength);
        }
    }

    if (OpenNewFile)
    {
        /*
        ** Either the file did not exist or we closed it because
        **   of the size limit test above...
        */
        DS_FileCreateDest(FileIndex);

        if (OS_ObjectIdDefined(FileStatus->FileHandle))
        {
            /*
            ** By writing the first packet without first performing a size
            **   limit test, we avoid issues resulting from having the max
            **   file size set less than the size of one packet...
            */
            DS_FileWriteData(FileIndex, BufPtr, PacketLength);
        }
    }

    /*
    ** If the write did not occur due to I/O error (create or write)
    **   then current state = file closed and destination disabled...
    */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write data to destination file                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileWriteData(int32 FileIndex, const void *FileData, uint32 DataLength)
{
    DS_AppFileStatus_t *FileStatus = &DS_AppData.FileStatus[FileIndex];
    int32               Result;

    /*
    ** Let cFE manage the file I/O...
    */
    Result = OS_write(FileStatus->FileHandle, FileData, DataLength);
    if (Result == DataLength)
    {
        /*
        ** Success - update file size and data rate counters...
        */
        DS_AppData.FileWriteCounter++;

        FileStatus->FileSize += DataLength;
        FileStatus->FileGrowth += DataLength;
    }
    else
    {
        /*
        ** Error - send event, close file and disable destination...
        */
        DS_FileWriteError(FileIndex, DataLength, Result);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Write header to destination file                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileWriteHeader(int32 FileIndex)
{
#if (DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE)

    DS_DestFileEntry_t *DestFile   = &DS_AppData.DestFileTblPtr->File[FileIndex];
    DS_AppFileStatus_t *FileStatus = &DS_AppData.FileStatus[FileIndex];
    CFE_FS_Header_t     CFE_FS_Header;
    DS_FileHeader_t     DS_FileHeader;
    int32               Result;

    /*
    ** Initialize selected parts of the cFE file header...
    */
    CFE_FS_InitHeader(&CFE_FS_Header, DS_FILE_HDR_DESCRIPTION, DS_FILE_HDR_SUBTYPE);

    /*
    ** Let cFE finish the init and write the primary header...
    */
    Result = CFE_FS_WriteHeader(FileStatus->FileHandle, &CFE_FS_Header);

    if (Result == sizeof(CFE_FS_Header_t))
    {
        /*
        ** Success - update file size and data rate counters...
        */
        DS_AppData.FileWriteCounter++;

        FileStatus->FileSize += sizeof(CFE_FS_Header_t);
        FileStatus->FileGrowth += sizeof(CFE_FS_Header_t);

        /*
        ** Initialize the DS file header...
        */
        memset(&DS_FileHeader, 0, sizeof(DS_FileHeader));
        DS_FileHeader.FileTableIndex = FileIndex;
        DS_FileHeader.FileNameType   = DestFile->FileNameType;
        strncpy(DS_FileHeader.FileName, FileStatus->FileName, sizeof(DS_FileHeader.FileName));

        /*
        ** Manually write the secondary header...
        */
        Result = OS_write(FileStatus->FileHandle, &DS_FileHeader, sizeof(DS_FileHeader_t));

        if (Result == sizeof(DS_FileHeader_t))
        {
            /*
            ** Success - update file size and data rate counters...
            */
            DS_AppData.FileWriteCounter++;

            FileStatus->FileSize += sizeof(DS_FileHeader_t);
            FileStatus->FileGrowth += sizeof(DS_FileHeader_t);
        }
        else
        {
            /*
            ** Error - send event, close file and disable destination...
            */
            DS_FileWriteError(FileIndex, sizeof(DS_FileHeader_t), Result);
        }
    }
    else
    {
        /*
        ** Error - send event, close file and disable destination...
        */
        DS_FileWriteError(FileIndex, sizeof(CFE_FS_Header_t), Result);
    }
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* File write error handler                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileWriteError(uint32 FileIndex, uint32 DataLength, int32 WriteResult)
{
    DS_AppFileStatus_t *FileStatus = &DS_AppData.FileStatus[FileIndex];

    /*
    ** Send event, close file and disable destination...
    */
    DS_AppData.FileWriteErrCounter++;

    CFE_EVS_SendEvent(DS_WRITE_FILE_ERR_EID, CFE_EVS_EventType_ERROR,
                      "FILE WRITE error: result = %d, length = %d, dest = %d, name = '%s'", (int)WriteResult,
                      (int)DataLength, (int)FileIndex, FileStatus->FileName);

    DS_FileCloseDest(FileIndex);

    FileStatus->FileState = DS_DISABLED;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create destination file                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateDest(uint32 FileIndex)
{
    DS_DestFileEntry_t *DestFile   = &DS_AppData.DestFileTblPtr->File[FileIndex];
    DS_AppFileStatus_t *FileStatus = &DS_AppData.FileStatus[FileIndex];
    int32               Result;
    osal_id_t           LocalFileHandle = OS_OBJECT_ID_UNDEFINED;

    /*
    ** Create filename from "path + base + sequence count + extension"...
    */
    DS_FileCreateName(FileIndex);

    if (FileStatus->FileName[0] != 0)
    {
        /*
        ** Success - create a new destination file...
        */
        Result = OS_OpenCreate(&LocalFileHandle, FileStatus->FileName, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE,
                               OS_READ_WRITE);

        if (Result != OS_SUCCESS)
        {
            /*
            ** Error - send event, disable destination and reset filename...
            */
            DS_AppData.FileWriteErrCounter++;

            CFE_EVS_SendEvent(DS_CREATE_FILE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FILE CREATE error: result = %d, dest = %d, name = '%s'", (int)Result, (int)FileIndex,
                              FileStatus->FileName);

            memset(FileStatus->FileName, 0, sizeof(FileStatus->FileName));

            /*
            ** Something needs to get fixed before we try again...
            */
            FileStatus->FileState = DS_DISABLED;
        }
        else
        {
            /*
            ** Success - store the file handle...
            */
            DS_AppData.FileWriteCounter++;

            FileStatus->FileHandle = LocalFileHandle;

            /*
            ** Initialize and write config specific file header...
            */
            DS_FileWriteHeader(FileIndex);

            /*
            ** Update sequence count if have one and write successful...
            */
            if (OS_ObjectIdDefined(FileStatus->FileHandle) && (DestFile->FileNameType == DS_BY_COUNT))
            {
                FileStatus->FileCount++;
                if (FileStatus->FileCount > DS_MAX_SEQUENCE_COUNT)
                {
                    FileStatus->FileCount = DestFile->SequenceCount;
                }

                /*
                ** Update Critical Data Store (CDS)...
                */
                DS_TableUpdateCDS();
            }
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Create destination filename                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateName(uint32 FileIndex)
{
    DS_DestFileEntry_t *DestFile    = &DS_AppData.DestFileTblPtr->File[FileIndex];
    DS_AppFileStatus_t *FileStatus  = &DS_AppData.FileStatus[FileIndex];
    int32               TotalLength = 0;

    char Workname[2 * DS_TOTAL_FNAME_BUFSIZE];
    char Sequence[DS_TOTAL_FNAME_BUFSIZE];

    /* Copy in path */
    CFE_SB_MessageStringGet(Workname, DestFile->Pathname, NULL, sizeof(Workname), sizeof(DestFile->Pathname));
    TotalLength = strlen(Workname);

    if (TotalLength > 0)
    {
        /* Add separator if needed */
        if (Workname[TotalLength - 1] != DS_PATH_SEPARATOR)
        {
            /* There's always space since Workname is twice the size of Pathname */
            Workname[TotalLength++] = DS_PATH_SEPARATOR;
        }

        /* Add base name */
        CFE_SB_MessageStringGet(&Workname[TotalLength], DestFile->Basename, NULL, sizeof(Workname) - TotalLength,
                                sizeof(DestFile->Basename));
        TotalLength = strlen(Workname);

        /* Create the sequence portion of the filename */
        DS_FileCreateSequence(Sequence, DestFile->FileNameType, FileStatus->FileCount);

        /* Sequence is always null terminated so can use strncat */
        strncat(&Workname[TotalLength], Sequence, sizeof(Workname) - TotalLength - 1);
        TotalLength = strlen(Workname);

        /* Only add extension if not empty */
        if (DestFile->Extension[0] != '\0')
        {
            /* Add a "." character (if needed) before appending the extension */
            if (DestFile->Extension[0] != '.')
            {
                strncat(Workname, ".", sizeof(Workname) - strlen(Workname) - 1);
                TotalLength++;
            }

            /* Append the extension portion to the path/base+sequence portion */
            CFE_SB_MessageStringGet(&Workname[TotalLength], DestFile->Extension, NULL, sizeof(Workname) - TotalLength,
                                    sizeof(DestFile->Extension));
        }

        /* Confirm working name fits */
        if (strlen(Workname) < DS_TOTAL_FNAME_BUFSIZE)
        {
            /* Success - copy workname to filename buffer */
            strcpy(FileStatus->FileName, Workname);
        }
        else
        {
            /* Error - send event and disable destination */
            CFE_EVS_SendEvent(DS_FILE_NAME_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FILE NAME error: dest = %d, path = '%s', base = '%s', seq = '%s', ext = '%s'",
                              (int)FileIndex, DestFile->Pathname, DestFile->Basename, Sequence, DestFile->Extension);
            DS_AppData.FileStatus[FileIndex].FileState = DS_DISABLED;
        }
    }
    else
    {
        /* Send event and disable for invalid path */
        CFE_EVS_SendEvent(DS_FILE_CREATE_EMPTY_PATH_ERR_EID, CFE_EVS_EventType_ERROR,
                          "FILE NAME error: Path empty. dest = %d, path = '%s'", (int)FileIndex, DestFile->Pathname);
        DS_AppData.FileStatus[FileIndex].FileState = DS_DISABLED;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set text from count or time                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCreateSequence(char *Buffer, uint32 Type, uint32 Count)
{
    CFE_TIME_SysTime_t TimeToPrint;

    uint32 SequenceCount = 0;
    uint32 NumericDigit  = 0;

    int32 BufferIndex = 0;

    /*
    ** Build the sequence portion of the filename (time or count)...
    */
    if (Type == DS_BY_COUNT)
    {
        /*
        ** Get copy of sequence count that can be modified...
        */
        SequenceCount = Count;

        /*
        ** Extract each digit (least significant digit first)...
        */
        for (BufferIndex = DS_SEQUENCE_DIGITS - 1; BufferIndex >= 0; BufferIndex--)
        {
            /*
            ** Extract this digit and prepare for next digit...
            */
            NumericDigit  = SequenceCount % 10;
            SequenceCount = SequenceCount / 10;

            /*
            ** Store this digit as ASCII in sequence string buffer...
            */
            Buffer[BufferIndex] = '0' + NumericDigit;
        }

        /*
        ** Add string terminator...
        */
        Buffer[DS_SEQUENCE_DIGITS] = '\0';
    }
    else if (Type == DS_BY_TIME)
    {
        /*
        ** Filename is based on seconds from current time...
        */
        TimeToPrint = CFE_TIME_GetTime();

        /*
        ** Convert time value to cFE format text string...
        */
        CFE_TIME_Print(Buffer, TimeToPrint);

/*
** cFE time string has format: "YYYY-DDD-HH:MM:SS.sssss"...
*/
#define CFE_YYYY_INDEX 0
#define CFE_DDD_INDEX  5
#define CFE_HH_INDEX   9
#define CFE_MM_INDEX   12
#define CFE_SS_INDEX   15
#define CFE_ssss_INDEX 18

/*
** DS time string has format: "YYYYDDDHHMMSS"...
*/
#define DS_YYYY_INDEX 0
#define DS_DDD_INDEX  4
#define DS_HH_INDEX   7
#define DS_MM_INDEX   9
#define DS_SS_INDEX   11
#define DS_TERM_INDEX 13

        /*
        ** Convert cFE time string to DS time string by moving
        **  the cFE chars to the left to remove extra stuff...
        */

        /*
        ** Step 1: Leave "year" (YYYY) alone - it is already OK...
        */

        /*
        ** Step 2: Move "day of year" (DDD) next to (YYYY)...
        */
        Buffer[DS_DDD_INDEX + 0] = Buffer[CFE_DDD_INDEX + 0];
        Buffer[DS_DDD_INDEX + 1] = Buffer[CFE_DDD_INDEX + 1];
        Buffer[DS_DDD_INDEX + 2] = Buffer[CFE_DDD_INDEX + 2];

        /*
        ** Step 3: Move "hour of day" (HH) next to (DDD)...
        */
        Buffer[DS_HH_INDEX + 0] = Buffer[CFE_HH_INDEX + 0];
        Buffer[DS_HH_INDEX + 1] = Buffer[CFE_HH_INDEX + 1];

        /*
        ** Step 4: Move "minutes" (MM) next to (HH)...
        */
        Buffer[DS_MM_INDEX + 0] = Buffer[CFE_MM_INDEX + 0];
        Buffer[DS_MM_INDEX + 1] = Buffer[CFE_MM_INDEX + 1];

        /*
        ** Step 5: Move "seconds" (SS) next to (MM)...
        */
        Buffer[DS_SS_INDEX + 0] = Buffer[CFE_SS_INDEX + 0];
        Buffer[DS_SS_INDEX + 1] = Buffer[CFE_SS_INDEX + 1];

        /*
        ** Step 6: Skip "subsecs" (ssss) - not in DS format...
        */

        /*
        ** Step 7: Add string terminator...
        */
        Buffer[DS_TERM_INDEX] = '\0';
    }
    else
    {
        /*
        ** Bad filename type, init buffer as empty...
        */
        Buffer[0] = '\0';
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Update destination file header                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileUpdateHeader(int32 FileIndex)
{
#if (DS_FILE_HEADER_TYPE == DS_FILE_HEADER_CFE)
    /*
    ** Update CFE specific header fields...
    */
    DS_AppFileStatus_t *FileStatus  = &DS_AppData.FileStatus[FileIndex];
    CFE_TIME_SysTime_t  CurrentTime = CFE_TIME_GetTime();
    int32               Result;

    Result = OS_lseek(FileStatus->FileHandle, sizeof(CFE_FS_Header_t), OS_SEEK_SET);

    if (Result == sizeof(CFE_FS_Header_t))
    {
        /* update file close time */
        Result = OS_write(FileStatus->FileHandle, &CurrentTime, sizeof(CFE_TIME_SysTime_t));

        if (Result == sizeof(CFE_TIME_SysTime_t))
        {
            DS_AppData.FileUpdateCounter++;
        }
        else
        {
            DS_AppData.FileUpdateErrCounter++;
        }
    }
    else
    {
        DS_AppData.FileUpdateErrCounter++;
    }
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Close destination file                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileCloseDest(int32 FileIndex)
{
    DS_AppFileStatus_t *FileStatus = &DS_AppData.FileStatus[FileIndex];

#if (DS_MOVE_FILES == true)
    /*
    ** Move file from working directory to downlink directory...
    */
    int32 OS_result;
    int32 PathLength;
    char *FileName;
    char  PathName[DS_TOTAL_FNAME_BUFSIZE];

    /*
    ** First, close the file...
    */
    OS_close(FileStatus->FileHandle);

    /*
    ** Move file only if table has a downlink directory name...
    */
    if (DS_AppData.DestFileTblPtr->File[FileIndex].Movename[0] != '\0')
    {
        /*
        ** Make sure directory name does not end with slash character...
        */
        CFE_SB_MessageStringGet(PathName, DS_AppData.DestFileTblPtr->File[FileIndex].Movename, NULL, sizeof(PathName),
                                sizeof(DS_AppData.DestFileTblPtr->File[FileIndex].Movename));
        PathLength = strlen(PathName);
        if (PathName[PathLength - 1] == '/')
        {
            PathName[PathLength - 1] = '\0';
            PathLength--;
        }

        /*
        ** Get a pointer to slash character before the filename...
        */
        FileName = strrchr(FileStatus->FileName, '/');

        if (FileName != NULL)
        {
            /*
            ** Verify that directory name plus filename is not too large...
            */
            if ((PathLength + strlen(FileName)) < DS_TOTAL_FNAME_BUFSIZE)
            {
                /*
                ** Append the filename (with slash) to the directory name...
                */
                strcat(PathName, FileName);

                /*
                ** Use OS function to move/rename the file...
                */
                OS_result = OS_mv(FileStatus->FileName, PathName);

                if (OS_result != OS_SUCCESS)
                {
                    /*
                    ** Error - send event but leave destination enabled...
                    */
                    CFE_EVS_SendEvent(DS_MOVE_FILE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "FILE MOVE error: src = '%s', tgt = '%s', result = %d", FileStatus->FileName,
                                      PathName, OS_result);
                }
            }
            else
            {
                /*
                ** Error - send event but leave destination enabled...
                */
                CFE_EVS_SendEvent(DS_MOVE_FILE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "FILE MOVE error: dir name = '%s', filename = '%s'", PathName, FileName);
            }
        }
        else
        {
            /*
            ** Error - send event but leave destination enabled...
            */
            CFE_EVS_SendEvent(DS_MOVE_FILE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "FILE MOVE error: dir name = '%s', filename = 'NULL'", PathName);
        }

        /* Update the path name for reporting */
        strncpy(FileStatus->FileName, PathName, sizeof(FileStatus->FileName));
    }
#else
    /*
    ** Close the file...
    */
    OS_close(FileStatus->FileHandle);
#endif

    /*
    ** Transmit file information telemetry...
    */
    DS_FileTransmit(FileStatus);

    /*
    ** Reset status for this destination file...
    */
    FileStatus->FileHandle = OS_OBJECT_ID_UNDEFINED;
    FileStatus->FileAge    = 0;
    FileStatus->FileSize   = 0;

    /*
    ** Remove previous filename from status data...
    */
    memset(FileStatus->FileName, 0, sizeof(FileStatus->FileName));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* File age processor                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DS_FileTestAge(uint32 ElapsedSeconds)
{
    uint32 FileIndex = 0;

    /*
    ** Called from HK request command handler (elapsed = platform config)
    */
    if (DS_AppData.DestFileTblPtr != (DS_DestFileTable_t *)NULL)
    {
        /*
        ** Cannot test file age without destination file table...
        */
        for (FileIndex = 0; FileIndex < DS_DEST_FILE_CNT; FileIndex++)
        {
            /*
            ** Update age of open files...
            */
            if (OS_ObjectIdDefined(DS_AppData.FileStatus[FileIndex].FileHandle))
            {
                DS_AppData.FileStatus[FileIndex].FileAge += ElapsedSeconds;

                if (DS_AppData.FileStatus[FileIndex].FileAge >= DS_AppData.DestFileTblPtr->File[FileIndex].MaxFileAge)
                {
                    /*
                    ** Close files that exceed maximum file age...
                    */
                    DS_FileUpdateHeader(FileIndex);
                    DS_FileCloseDest(FileIndex);
                }
            }
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Transmit file info                                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DS_FileTransmit(DS_AppFileStatus_t *FileStatus)
{
    DS_FileCompletePktBuf_t *PktBuf;
    DS_FileInfo_t *          FileInfo;

    /*
    ** Get a Message block of memory and initialize it
    */
    PktBuf = (DS_FileCompletePktBuf_t *)CFE_SB_AllocateMessageBuffer(sizeof(*PktBuf));

    /*
    ** Process destination file info data...
    */
    if (PktBuf != NULL)
    {
        CFE_MSG_Init(CFE_MSG_PTR(PktBuf->Pkt.TelemetryHeader), CFE_SB_ValueToMsgId(DS_COMP_TLM_MID), sizeof(*PktBuf));

        FileInfo = &PktBuf->Pkt.Payload;

        /*
        ** Set file age and size...
        */
        FileInfo->FileAge  = FileStatus->FileAge;
        FileInfo->FileSize = FileStatus->FileSize;
        /*
        ** Set file growth rate (computed when process last HK request)...
        */
        FileInfo->FileRate = FileStatus->FileRate;
        /*
        ** Set current filename sequence count...
        */
        FileInfo->SequenceCount = FileStatus->FileCount;
        /*
        ** Set file enable/disable state...
        */
        FileInfo->EnableState = FileStatus->FileState;
        /*
        ** Set file closed state...
        */
        FileInfo->OpenState = DS_CLOSED;
        /*
        ** Set current open filename...
        */
        strncpy(FileInfo->FileName, FileStatus->FileName, sizeof(FileInfo->FileName));

        /*
        ** Timestamp and send file info telemetry...
        */
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(PktBuf->Pkt.TelemetryHeader));
        CFE_SB_TransmitBuffer(&PktBuf->SBBuf, true);
    }
}
