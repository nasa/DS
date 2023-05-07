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
 *  The CFS Data Storage (DS) Application event id header file
 */
#ifndef DS_EVENTS_H
#define DS_EVENTS_H

/**
 * \defgroup cfsdsevents CFS Data Store Event IDs
 * \{
 */

/**
 *  \brief DS Initialization Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event message is issued if the Data Storage (DS) application successfully
 *  completes its initialization.
 */
#define DS_INIT_EID 1

/**
 *  \brief DS Initialization Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the application receives an error while
 *  performing cFE initialization.  The value displayed is the return code
 *  from the function that failed.
 */
#define DS_INIT_ERR_EID 2

/**
 *  \brief DS App Termination Event ID
 *
 *  \par Type: CRITICAL
 *
 *  \par Cause:
 *
 *  This event message is issued when the application performs a voluntary
 *  termination.  The termination may be caused by a cFE command requesting
 *  the application be terminated - or, the cause may result from the DS
 *  application having received an error when invoking a cFE or O/S function
 *  necessary for normal operation.
 */
#define DS_EXIT_ERR_EID 3

/**
 *  \brief DS Critical Data Store Initialization Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  The DS application stores the current values for Destination Filename
 *  sequence counts in the Critical Data Store (CDS).  This ensures that DS
 *  will not overwrite old data storage files following a processor reset.
 *  This event indicates an error at startup as DS is initializing access
 *  to the Critical Data Store.  Subsequent CDS errors are ignored by DS.
 */
#define DS_INIT_CDS_ERR_EID 6

/**
 *  \brief DS Critical Data Store Table Restore Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  If configured to make DS tables critical, the DS application will try and
 *  recover the table data in use prior to a processor reset.  These startup
 *  initialization events indicate that the table data has been restored from
 *  the Critical Data Store (CDS).
 */
#define DS_INIT_TBL_CDS_EID 7

/**
 *  \brief DS Initialize Table Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  At startup, the DS application must create both a packet filter table and
 *  a destination file definition table.  After creating the tables, DS must
 *  then load default table data for each of the files.  If DS is unable to
 *  create either table then the application will terminate.  If DS is able
 *  to create both tables and is then unable to load both tables then DS will
 *  operate in a reduced function mode - thus allowing table load error
 *  recovery via the cFE Table Services command interface.
 */
#define DS_INIT_TBL_ERR_EID 8

/**
 *  \brief DS Destination Table Validation Results Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event describes the results of the Destination File Table validation
 *  function.  The cFE Table Services Manager will call this function autonomously
 *  when the default table is loaded at startup and also whenever a table validate
 *  command (that targets this table) is processed.
 */
#define DS_FIL_TBL_EID 10

/**
 *  \brief DS Destination Table Validation Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  These are the errors that the DS application might encounter when verifying
 *  the contents of a Destination File Table.  Note that only the first error
 *  detected for each table file will result in an event being generated.  The
 *  remaining entries are still tested to establish the result counters but no
 *  further error events will be generated during this call to the verify function.
 */
#define DS_FIL_TBL_ERR_EID 11

/**
 *  \brief DS Filter Table Validation Results Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event describes the results of the Packet Filter Table validation
 *  function.  The cFE Table Services Manager will call this function autonomously
 *  when the default table is loaded at startup and also whenever a table validate
 *  command (that targets this table) is processed.
 */
#define DS_FLT_TBL_EID 12

/**
 *  \brief DS Filter Table Validation Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  These are the errors that the DS application might encounter when verifying
 *  the contents of a Packet Filter Table.  Note that only the first error
 *  detected for each table file will result in an event being generated.  The
 *  remaining entries are still tested to establish the result counters but no
 *  further error events will be generated during this call to the verify function.
 */
#define DS_FLT_TBL_ERR_EID 13

/**
 *  \brief DS File Name Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This error occurs when an attempt to create a destination filename fails.
 *  The expected cause of this error is for the sum of the lengths of each
 *  filename element to be invalid, even though each individual element
 *  length is valid.  Note that each of the individual element strings has
 *  been previously verified.
 *
 *  Note that the filename create error handler will also disable the
 *  destination to prevent the immediate re-occurrence of the error as more
 *  packets are written to this destination.
 */
#define DS_FILE_NAME_ERR_EID 14

/**
 *  \brief DS File Create Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This error occurs when an attempt to create a destination file fails.
 *  The most common cause of this error is insufficient free space available
 *  on the target file system (disk).  However, it is possible to specify
 *  a filename that is legal but still invalid (i.e. the path string has
 *  legal chars but does not exist).
 *
 *  Note that the file create error handler will also disable the destination
 *  to prevent the immediate re-occurrence of the error as more packets are
 *  written to this destination.
 */
#define DS_CREATE_FILE_ERR_EID 15

/**
 *  \brief DS File Write Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This error occurs when an attempt to write to a destination file fails.
 *  The most common cause of this error is insufficient free space available
 *  on the target file system (disk).  Other causes for this error imply
 *  some sort of operational or file system failure and will require careful
 *  analysis.
 *
 *  Note that the file write error handler will also close the file and
 *  disable the destination to prevent the immediate re-occurrence of the
 *  error as more packets are written to this destination.
 */
#define DS_WRITE_FILE_ERR_EID 16

/**
 *  \brief DS Invalid Command Code Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This error occurs when a DS application command is received that has
 *  an undefined command code (CC).  Ground systems generally prevent
 *  these errors so the cause is more likely to result from sending "raw"
 *  ground commands or on-board stored commands.
 */
#define DS_CMD_CODE_ERR_EID 21

/**
 *  \brief DS Housekeeping Request Invalid Length Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a HK request command.
 *  The cause of the failure is an invalid command packet length.
 */
#define DS_HK_REQUEST_ERR_EID 22

/**
 *  \brief DS No-op Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of an application NOOP
 *  command.  The command is used primarily as an indicator that the DS
 *  application can receive commands and generate telemetry.
 */
#define DS_NOOP_CMD_EID 31

/**
 *  \brief DS No-op Command Invalid Length Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a NOOP command.
 *  The cause of the failure is an invalid command packet length.
 */
#define DS_NOOP_CMD_ERR_EID 32

/**
 *  \brief DS Reset Counters Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a RESET housekeeping
 *  counters command.  The command is used primarily to clear counters
 *  that have already been examined.
 */
#define DS_RESET_CMD_EID 33

/**
 *  \brief DS Reset Counters Command Invalid Length Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a RESET command.
 *  The cause of the failure is an invalid command packet length.
 */
#define DS_RESET_CMD_ERR_EID 34

/**
 *  \brief DS Application Enable/Disable State Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set
 *  the application enable/disable state for processing data storage
 *  packets.  The DS application is always enabled to receive and
 *  process commands.
 */
#define DS_ENADIS_CMD_EID 35

/**
 *  \brief DS Application Enable/Disable State Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  application enable/disable state.  The cause of the failure may
 *  be an invalid command packet length or an invalid enable/disable
 *  state value.
 */
#define DS_ENADIS_CMD_ERR_EID 36

/**
 *  \brief DS Set Filter File Index Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  The Packet Filter Table consists of an array of packet structures,
 *  with each element in the array representing one packet message ID.
 *
 *  Each packet structure contains an array of filter structures, and
 *  each filter structure contains a destination file selection.  Thus,
 *  each packet has multiple filters and each filter selects a file.
 *
 *  File selections are expressed as an index into the Destination File
 *  Table.
 *
 *  This event signals the successful execution of a command to set the
 *  destination file index for a single filter entry in the Packet Filter
 *  Table.
 */
#define DS_FILE_CMD_EID 37

/**
 *  \brief DS Set Filter File Index Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  Destination File Table index for a Packet Filter Table entry.
 *  The cause of the failure may be an invalid command packet length,
 *  an invalid Message ID or an invalid filter parameters index.
 *
 *  The failure may also result from not having a Packet Filter Table
 *  loaded at the time the command was invoked or because the message
 *  ID was not found in the Packet Filter Table.
 */
#define DS_FILE_CMD_ERR_EID 38

/**
 *  \brief DS Set Filter Type Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  The Packet Filter Table consists of an array of packet structures,
 *  with each element in the array representing one packet message ID.
 *
 *  Each packet structure contains an array of filter structures, and
 *  each filter structure contains a filter type selection.  Thus, each
 *  packet has multiple filters and each filter selects a filter type.
 *
 *  This event signals the successful execution of a command to set the
 *  filter type for a single filter entry in the Packet Filter Table.
 */
#define DS_FTYPE_CMD_EID 39

/**
 *  \brief DS Set Filter Type Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  filter type for a Packet Filter Table entry.  The cause of the
 *  failure may be an invalid command packet length, an invalid message
 *  ID, an invalid Filter Parameters index or an invalid filter type.
 *
 *  The failure may also result from not having a Packet Filter Table
 *  loaded at the time the command was invoked or because the Message
 *  ID was not found in the Packet Filter Table.
 */
#define DS_FTYPE_CMD_ERR_EID 40

/**
 *  \brief DS Set Filter Parameters Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  The Packet Filter Table consists of an array of packet structures,
 *  with each element in the array representing one packet message ID.
 *
 *  Each packet structure contains an array of filter structures, and
 *  each filter structure contains a set of filter parameters.  Thus,
 *  each packet has multiple filters and each filter has unique filter
 *  parameters.
 *
 *  This event signals the successful execution of a command to set the
 *  filter parameters for a single filter entry in the Packet Filter Table.
 */
#define DS_PARMS_CMD_EID 41

/**
 *  \brief DS Set Filter Parameters Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  filter parameters for a Packet Filter Table entry.  The cause of
 *  the failure may be an invalid command packet length, an invalid
 *  message ID, an invalid filter parameters index or an invalid filter
 *  parameters value (N,X,O).
 *
 *  The failure may also result from not having a Packet Filter Table
 *  loaded at the time the command was invoked or because the Message
 *  ID was not found in the Packet Filter Table.
 */
#define DS_PARMS_CMD_ERR_EID 42

/**
 *  \brief DS Set Filename Type Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  filename type for a Destination File Table entry.
 */
#define DS_NTYPE_CMD_EID 43

/**
 *  \brief DS Set Filename Type Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  filename type for the selected Destination File Table entry.  The
 *  cause of the failure may be an invalid command packet length, an
 *  invalid file table index or an invalid filename type.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_NTYPE_CMD_ERR_EID 44

/**
 *  \brief DS Set File Enable/Disable State Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  enable/disable state for a Destination File Table entry.
 */
#define DS_STATE_CMD_EID 45

/**
 *  \brief DS Set File Enable/Disable State Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  enable/disable state for the selected Destination File Table entry.
 *  The cause of the failure may be an invalid command packet length,
 *  an invalid file table index or an invalid state selection.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_STATE_CMD_ERR_EID 46

/**
 *  \brief DS Set Filename Path Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  path portion of the filename for a Destination File Table entry.
 */
#define DS_PATH_CMD_EID 47

/**
 *  \brief DS Set Filename Path Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  filename path text for a Destination File Table entry.  The cause
 *  of the failure may be an invalid command packet length, an invalid
 *  file table index or invalid filename path text.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_PATH_CMD_ERR_EID 48

/**
 *  \brief DS Set File Basename Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  basename portion of the filename for a Destination File Table entry.
 */
#define DS_BASE_CMD_EID 49

/**
 *  \brief DS Set File Basename Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  base filename text for a Destination File Table entry.  The cause
 *  of the failure may be an invalid command packet length, an invalid
 *  file table index or invalid base filename text.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_BASE_CMD_ERR_EID 50

/**
 *  \brief DS Set Filename Extension Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  extension portion of the filename for a Destination File Table entry.
 */
#define DS_EXT_CMD_EID 51

/**
 *  \brief DS Set Filename Extension Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  filename extension text for a Destination File Table entry.  The
 *  cause of the failure may be an invalid command packet length, an
 *  invalid file table index or invalid filename extension text.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_EXT_CMD_ERR_EID 52

/**
 *  \brief DS Set Max File Size Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  file size limit for a Destination File Table entry.
 */
#define DS_SIZE_CMD_EID 53

/**
 *  \brief DS Set Max File Size Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  file size limit for a Destination File Table entry.  The cause
 *  of the failure may be an invalid command packet length, an invalid
 *  file table index or an invalid file size limit.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_SIZE_CMD_ERR_EID 54

/**
 *  \brief DS Set Max File Age Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  file age limit for a Destination File Table entry.
 */
#define DS_AGE_CMD_EID 55

/**
 *  \brief DS Set Max File Age Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  file age limit for a Destination File Table entry.  The cause
 *  of the failure may be an invalid command packet length, an invalid
 *  file table index or an invalid file age limit.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_AGE_CMD_ERR_EID 56

/**
 *  \brief DS Set Sequence Count Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to set the
 *  sequence count for a Destination File Table entry.
 */
#define DS_SEQ_CMD_EID 57

/**
 *  \brief DS Set Sequence Count Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to set the
 *  sequence count for a Destination File Table entry.  The cause
 *  of the failure may be an invalid command packet length, an invalid
 *  file table index or an invalid sequence count.
 *
 *  The failure may also result from not having a Destination File Table
 *  loaded at the time the command was invoked.
 */
#define DS_SEQ_CMD_ERR_EID 58

/**
 *  \brief DS Close Destination File Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to close
 *  the selected Destination File.
 *
 *  This command will succeed if the file table index is valid,
 *  regardless of whether there was an open file.
 */
#define DS_CLOSE_CMD_EID 59

/**
 *  \brief DS Close Destination File Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to close the
 *  selected Destination File.  The cause of the failure may be an
 *  invalid command packet length or an invalid file table index.
 */
#define DS_CLOSE_CMD_ERR_EID 60

/**
 *  \brief DS File Move Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed attempt to move a Data Storage file
 *  from the temporary working directory it was created in, to the
 *  directory specified in the file definition table.  The cause of the
 *  failure may be an invalid combined directory and filename length or
 *  the system file move function may have returned an error result.
 */
#define DS_MOVE_FILE_ERR_EID 61

/**
 *  \brief DS Get File Info Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to get
 *  the file info diagnostic packet.
 *
 *  This command will succeed even if the selected file does not exist.
 */
#define DS_GET_FILE_INFO_CMD_EID 62

/**
 *  \brief DS Get File Info Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to get the
 *  file info diagnostic packet.  The cause of the failure is an
 *  invalid command packet length.
 */
#define DS_GET_FILE_INFO_CMD_ERR_EID 63

/**
 *  \brief DS Add Message ID to Filter Table Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to add
 *  a new message ID to the Packet Filter Table.
 *
 *  The Packet Filter Table must be loaded and have an unused entry
 *  available for adding the new message ID.  The new message ID must
 *  not be zero and must not already exist in the table.
 */
#define DS_ADD_MID_CMD_EID 64

/**
 *  \brief DS Add Message ID to Filter Table Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to add a new
 *  message ID to the Packet Filter Table.  The cause of the failure
 *  may be an invalid command packet length or an invalid message ID.
 *
 *  The failure may also result from not having a Packet Filter Table
 *  loaded at the time the command was invoked.  The loaded table
 *  must have an unused entry available for the new message ID and
 *  must not already contain the new message ID.
 */
#define DS_ADD_MID_CMD_ERR_EID 65

/**
 *  \brief DS Close All Destination Files Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to close
 *  all open Destination Files.
 *
 *  This command will succeed regardless of whether there was an open file.
 */
#define DS_CLOSE_ALL_CMD_EID 66

/**
 *  \brief DS Close All Destination Files Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to close
 *  all open Destination Files.  The cause of the failure can only
 *  be an invalid command packet length.
 */
#define DS_CLOSE_ALL_CMD_ERR_EID 67

/**
 *  \brief DS File Create Invalid Name Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event is generated when DS_FileCreateName is invoked with an
 *  empty path name.
 */
#define DS_FILE_CREATE_EMPTY_PATH_ERR_EID 68

/**
 * \brief DS Filter Table Name Invalid Event ID
 *
 * \par Type: ERROR
 *
 * \par Cause:
 *
 * This event is issued when an invalid filter table name is passed to
 * the CFE_TBL_GetInfo.
 */
#define DS_APPHK_FILTER_TBL_ERR_EID 69

/**
 * \brief DS Filter Table Name Create Failed Event ID
 *
 * \par Type: ERROR
 *
 * \par Cause:
 *
 * This event is issued when the filter table name is not successfully
 * created (via snprintf) in the DS_AppSendHkCmd function.
 */
#define DS_APPHK_FILTER_TBL_PRINT_ERR_EID 70

/**
 *  \brief DS Remove Message ID from Filter Table Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event signals the successful execution of a command to remove
 *  a message ID from the Packet Filter Table.
 *
 *  The Packet Filter Table must be loaded and have a used entry
 *  for removing the message ID.  The message ID must not be zero
 *  and must already exist in the table.
 */
#define DS_REMOVE_MID_CMD_EID 71

/**
 *  \brief DS Remove Message ID from Filter Table Command Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event signals the failed execution of a command to remove a
 *  message ID from the Packet Filter Table.  The cause of the failure
 *  may be an invalid command packet length or an invalid message ID.
 *
 *  The failure may also result from not having a Packet Filter Table
 *  loaded at the time the command was invoked.  The loaded table
 *  must have an entry with the indicated message ID.
 */
#define DS_REMOVE_MID_CMD_ERR_EID 72

/**@}*/

#endif
