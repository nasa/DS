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
 *  CFS Data Storage (DS) table definitions
 */
#ifndef DS_TABLE_H
#define DS_TABLE_H

#include "cfe.h"

#include "ds_platform_cfg.h"

#include "ds_extern_typedefs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* DS hash table structures and definitions                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_HASH_TABLE_ENTRIES 256
#define DS_HASH_TABLE_MASK    0x00FF

/** \brief DS Hash Table Linked List structure */
typedef struct DS_HashTag
{
    CFE_SB_MsgId_t MessageID; /**< \brief DS filter table entry MessageID */
    uint16         Index;     /**< \brief DS filter table entry index */

    struct DS_HashTag *Next; /**< \brief Next hash table linked list element */
} DS_HashLink_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Prototypes for functions defined in ds_table.c                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Startup table initialization function
 *
 *  \par Description
 *       This function creates the Packet Filter and Destination
 *       File tables. The function then tries to load default table
 *       data for each table. If the Packet Filter Table load is
 *       successful, then the function will subscribe to all of the
 *       packets referenced by the table.
 *
 *  \par Called From:
 *       - DS startup initialization function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_DestFileEntry_t
 */
CFE_Status_t DS_TableInit(void);

/**
 *  \brief Manage destination file table loads, dumps, etc.
 *
 *  \par Description
 *       This function will provide cFE Table Services with an
 *       opportunity to make updates to the Destination File Table
 *       while in the context of the DS application. There is no
 *       special handling necessary if a new version of this table
 *       is loaded.
 *
 *  \par Called From:
 *       - DS table initialization function
 *       - DS housekeeping request command processor
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_DestFileEntry_t, #DS_TableVerifyDestFile
 */
void DS_TableManageDestFile(void);

/**
 *  \brief Manage packet filter table loads, dumps, etc.
 *
 *  \par Description
 *       This function will provide cFE Table Services with an
 *       opportunity to make updates to the Packet Filter Table
 *       while in the context of the DS application. If a new
 *       version of the filter table gets loaded, the function
 *       will first unsubscribe to any packets referenced by the
 *       previous version of the table, and then subscribe to
 *       all packets referenced by the new version of the table.
 *
 *  \par Called From:
 *       - DS table initialization function
 *       - DS housekeeping request command processor
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_TableVerifyFilter
 */
void DS_TableManageFilter(void);

/**
 *  \brief Verify destination file table data
 *
 *  \par Description
 *       This function is called by cFE Table Services to verify
 *       the contents of a candidate Destination File Table.  This
 *       will occur while cFE Table Services is processing another
 *       call from the DS application to manage the table. Thus,
 *       this verification function will be executed from within
 *       the context of the DS application.
 *
 *  \par Called From:
 *       - cFE Table Services
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] TableData Pointer to Destination File Table data
 *
 *  \sa #DS_DestFileEntry_t, #DS_TableVerifyDestFileEntry
 */
CFE_Status_t DS_TableVerifyDestFile(const void *TableData);

/**
 *  \brief Verify destination file table entry
 *
 *  \par Description
 *       This function is called from the Destination File Table
 *       verification function to verify a single table entry.
 *       This function, in turn, calls common utility functions
 *       to verify each field within the selected table entry.
 *
 *  \par Called From:
 *       - Destination File Table validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] DestFileEntry Pointer to selected Destination File Table entry
 *  \param[in] TableIndex    Index of selected Destination File Table entry
 *  \param[in] ErrorCount    Number of errors already encountered
 *
 *  \sa #DS_DestFileEntry_t, #DS_TableVerifyDestFile
 */
bool DS_TableVerifyDestFileEntry(DS_DestFileEntry_t *DestFileEntry, uint8 TableIndex, int32 ErrorCount);

/**
 *  \brief Verify packet filter table data
 *
 *  \par Description
 *       This function is called by cFE Table Services to verify
 *       the contents of a candidate Packet Filter Table.  This
 *       will occur while cFE Table Services is processing another
 *       call from the DS application to manage the table. Thus,
 *       this verification function will be executed from within
 *       the context of the DS application.
 *
 *  \par Called From:
 *       - cFE Table Services
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] TableData Pointer to Packet Filter Table data
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_TableVerifyFilterEntry
 */
CFE_Status_t DS_TableVerifyFilter(const void *TableData);

/**
 *  \brief Verify packet filter table entry
 *
 *  \par Description
 *       This function is called from the Packet Filter Table
 *       verification function to verify a single table entry.
 *       This function, in turn, calls common utility functions
 *       to verify each field within the selected table entry.
 *
 *  \par Called From:
 *       - Packet Filter Table validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] PacketEntry Pointer to a Packet Filter Table entry
 *  \param[in] TableIndex  Index of selected Packet Filter Table entry
 *  \param[in] ErrorCount  Number of errors already encountered
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_TableVerifyFilter
 */
bool DS_TableVerifyFilterEntry(DS_PacketEntry_t *PacketEntry, int32 TableIndex, int32 ErrorCount);

/**
 *  \brief Test for unused table entry
 *
 *  \par Description
 *       This function returns true if a table entry is unused.
 *       Unused is defined as containing nothing but zero's.
 *
 *  \par Called From:
 *       - Packet Filter Table validation function
 *       - Destination File Table validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] TableEntry Pointer to the table entry data
 *  \param[in] BufferSize Length of the table entry data
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_DestFileEntry_t
 */
bool DS_TableEntryUnused(const void *TableEntry, size_t BufferSize);

/**
 *  \brief Verify destination file index
 *
 *  \par Description
 *       This function verifies that the indicated packet filter
 *       table file table index is within bounds - as defined by
 *       platform configuration parameters. Note
 *       that the utility functions are also called from the ground
 *       command handlers to verify command arguments that nodify
 *       the table data.
 *
 *  \par Called From:
 *       - Command handler (set file table index)
 *       - Command handler (set filename type)
 *       - Command handler (set file enable/disable state)
 *       - Command handler (set pathname)
 *       - Command handler (set basename)
 *       - Command handler (set extension)
 *       - Command handler (set max file size limit)
 *       - Command handler (set max file age limit)
 *       - Command handler (set file sequence count)
 *       - Command handler (close file)
 *       - Packet Filter Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] FileTableIndex Destination File Table Index value
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_DestFileEntry_t
 */
bool DS_TableVerifyFileIndex(uint16 FileTableIndex);

/**
 *  \brief Verify packet filter parameters
 *
 *  \par Description
 *       This function verifies that the indicated packet filter
 *       table filter parameters are within bounds.
 *       Algorithm N must be <= Algorithm X
 *       Algorithm O must be <  Algorithm X
 *
 *  \par Called From:
 *       - Command handler (set filter parms)
 *       - Packet Filter Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] Algorithm_N Filter Algorithm N value
 *  \param[in] Algorithm_X Filter Algorithm X value
 *  \param[in] Algorithm_O Filter Algorithm O value
 *
 *  \sa #DS_TableVerifyType, #DS_TableVerifyState, #DS_DestFileEntry_t
 */
bool DS_TableVerifyParms(uint16 Algorithm_N, uint16 Algorithm_X, uint16 Algorithm_O);

/**
 *  \brief Verify packet filter type or filename type
 *
 *  \par Description
 *       This common function verifies that the indicated packet
 *       filter table filter type, or destination file table
 *       filename type is within bounds.
 *       Must be DS_BY_COUNT or DS_BY_TIME.
 *
 *  \par Called From:
 *       - Command handler (set filter type)
 *       - Command handler (set filename type)
 *       - Packet Filter Table entry validation function
 *       - Destination File Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] TimeVsCount Filter Type or Filename Type value
 *                         #DS_BY_COUNT or #DS_BY_TIME
 *
 *  \sa #DS_TableVerifyState, #DS_TableVerifySize, #DS_DestFileEntry_t
 */
bool DS_TableVerifyType(uint16 TimeVsCount);

/**
 *  \brief Verify application or destination file enable/disable state
 *
 *  \par Description
 *       This function verifies that the indicated destination file
 *       enable/disable state is within bounds.
 *       Must be DS_ENABLED or DS_DISABLED.
 *
 *  \par Called From:
 *       - Command handler (set application enable/disable state)
 *       - Command handler (set file enable/disable state)
 *       - Destination File Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] EnableState Enable/Disable State value
 *
 *  \sa #DS_TableVerifySize, #DS_TableVerifyAge, #DS_DestFileEntry_t
 */
bool DS_TableVerifyState(uint16 EnableState);

/**
 *  \brief Verify destination file max size limit
 *
 *  \par Description
 *       This function verifies that the indicated destination file
 *       max size limit is within bounds - as defined by platform
 *       configuration parameters.
 *
 *  \par Called From:
 *       - Command handler (set file max size limit)
 *       - Destination File Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MaxFileSize Maximum Size Limit value
 *
 *  \sa #DS_TableVerifyAge, #DS_TableVerifyCount, #DS_DestFileEntry_t
 */
bool DS_TableVerifySize(size_t MaxFileSize);

/**
 *  \brief Verify destination file max age limit
 *
 *  \par Description
 *       This function verifies that the indicated destination file
 *       max age limit is within bounds - as defined by platform
 *       configuration parameters.
 *
 *  \par Called From:
 *       - Command handler (set file max age limit)
 *       - Destination File Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MaxFileAge Maximum Age Limit value
 *
 *  \sa #DS_TableVerifySize, #DS_TableVerifyCount, #DS_DestFileEntry_t
 */
bool DS_TableVerifyAge(uint32 MaxFileAge);

/**
 *  \brief Verify destination file sequence count
 *
 *  \par Description
 *       This function verifies that the indicated destination file
 *       sequence count is within bounds - as defined by platform
 *       configuration parameters.
 *
 *  \par Called From:
 *       - Command handler (set file sequence count)
 *       - Destination File Table entry validation function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] SequenceCount Sequence Count value
 *
 *  \sa #DS_TableVerifySize, #DS_TableVerifyAge, #DS_DestFileEntry_t
 */
bool DS_TableVerifyCount(uint32 SequenceCount);

/**
 *  \brief Subscribe to packet filter table packets
 *
 *  \par Description
 *       A new Packet Filter Table is available for use. This
 *       function is called to subscribe to packets referenced
 *       by the new filter table.
 *
 *  \par Called From:
 *       - Packet Filter Table manage function
 *
 *  \par Assumptions, External Events, and Notes:
 *       Caller has determined that the new filter table exists.
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_TableUnsubscribe
 */
void DS_TableSubscribe(void);

/**
 *  \brief Unsubscribe to packet filter table packets
 *
 *  \par Description
 *       A new Packet Filter Table is available for use. Prior to
 *       subscribing to the packets referenced by the new filter
 *       table, this function is called to unsubscribe to packets
 *       referenced by the old filter table.
 *
 *  \par Called From:
 *       - Packet Filter Table manage function
 *
 *  \par Assumptions, External Events, and Notes:
 *       Caller has determined that the old filter table exists.
 *
 *  \sa #DS_PacketEntry_t, #DS_FilterParms_t, #DS_TableSubscribe
 */
void DS_TableUnsubscribe(void);

/**
 *  \brief Create local area within the Critical Data Store (CDS)
 *
 *  \par Description
 *       This function creates a new CDS area or gets access to a
 *       CDS area created prior to a processor reset. The CDS area
 *       will be used to store the current values for destination
 *       filename sequence counts.
 *
 *  \par Called From:
 *       - Application initialization function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_DestFileEntry_t
 */
CFE_Status_t DS_TableCreateCDS(void);

/**
 *  \brief Update CDS with current filename sequence count values
 *
 *  \par Description
 *       This function writes the current filename sequence count
 *       values to the Critical Data Store. The function is called
 *       each time the sequence count values are modified.
 *
 *  \par Called From:
 *       - Destination table data update handler
 *       - Destination file creation function
 *       - Command handler (set sequence count)
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_DestFileEntry_t
 */
void DS_TableUpdateCDS(void);

/**
 *  \brief Hash table function
 *
 *  \par Description
 *       This function converts a cFE MessageID into an index into
 *       the hash table. The indexed hash table entry contains a
 *       linked list where each link identifies a single entry in
 *       the packet filter table. Thus, the scope of searching the
 *       Packet Filter Table for a specific MessageID has been reduced
 *       from searching the total number of entries in the filter
 *       table by a factor equal to the size of the hash table.
 *
 *  \par Called From:
 *       - Hash table creation function (after load filter table)
 *       - Find messageID in filter table function
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MessageID Message ID
 *
 *  \return Index of hash table entry for input message ID
 *
 *  \sa #DS_HashLink_t, #DS_TableCreateHash, #DS_TableFindMsgID
 */
uint32 DS_TableHashFunction(CFE_SB_MsgId_t MessageID);

/**
 *  \brief Create hash table function
 *
 *  \par Description
 *       This function populates the hash table following a new
 *       load of the packet filter table. Because there may be
 *       more message ID's than hash table entries, the hash table
 *       function may translate multiple message ID's into each
 *       hash table index. Each hash table entry is a linked list
 *       containing a link for each message ID that translates to
 *       the same hash table index.
 *
 *  \par Called From:
 *       - Filter table manage updates function (after table load)
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \sa #DS_HashLink_t, #DS_TableHashFunction, #DS_TableFindMsgID
 */
void DS_TableCreateHash(void);

/**
 *  \brief Adds a message ID to the hash table
 *
 *  \par Description
 *       This function populates the hash table with a new message ID
 *
 *  \par Called From:
 *       - Creation of Hash Table
 *       - Command to add a MID
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MessageID   Message ID
 *  \param[in] FilterIndex Filter table index for message ID
 *
 *  \return Hash table index for message ID
 *
 *  \sa #DS_HashLink_t, #DS_TableHashFunction, #DS_TableFindMsgID
 */
int32 DS_TableAddMsgID(CFE_SB_MsgId_t MessageID, int32 FilterIndex);

/**
 *  \brief Search packet filter table for message ID
 *
 *  \par Description
 *       This function searches for a packet filter table entry that
 *       matches the input argument message ID.
 *
 *  \par Called From:
 *       - Data storage packet processor
 *       - Command handler (set file index)
 *       - Command handler (set filter type)
 *       - Command handler (set filter parms)
 *       - Command handler (add messageID to filter table)
 *
 *  \par Assumptions, External Events, and Notes:
 *       (none)
 *
 *  \param[in] MessageID Message ID
 *
 *  \return Filter table index for message ID
 *
 *  \sa #DS_HashLink_t, #DS_TableHashFunction, #DS_TableCreateHash
 */
int32 DS_TableFindMsgID(CFE_SB_MsgId_t MessageID);

#endif
