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

#ifndef DS_TEST_UTILS_H
#define DS_TEST_UTILS_H

#include "ds_app.h"
#include "ds_msg.h"
#include "cfe_msgids.h"
#include "utstubs.h"

/*
 * Allow UT access to the global "DS_AppData" object.
 */
extern DS_AppData_t DS_AppData;

/*
 * Global context structures
 */
typedef struct
{
    uint16 EventID;
    uint16 EventType;
    char   Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_EVS_SendEvent_context_t;

typedef struct
{
    char Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_ES_WriteToSysLog_context_t;

extern CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[];
extern CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* Command buffer typedef for any handler */
typedef union
{
    CFE_SB_Buffer_t        Buf;
    DS_NoopCmd_t           NoopCmd;
    DS_ResetCountersCmd_t  ResetCountersCmd;
    DS_SetAppStateCmd_t    AppStateCmd;
    DS_SetFilterFileCmd_t  FilterFileCmd;
    DS_SetFilterTypeCmd_t  FilterTypeCmd;
    DS_SetFilterParmsCmd_t FilterParmsCmd;
    DS_SetDestTypeCmd_t    DestTypeCmd;
    DS_SetDestStateCmd_t   DestStateCmd;
    DS_SetDestPathCmd_t    DestPathCmd;
    DS_SetDestBaseCmd_t    DestBaseCmd;
    DS_SetDestExtCmd_t     DestExtCmd;
    DS_SetDestSizeCmd_t    DestSizeCmd;
    DS_SetDestAgeCmd_t     DestAgeCmd;
    DS_SetDestCountCmd_t   DestCountCmd;
    DS_CloseFileCmd_t      CloseFileCmd;
    DS_CloseAllCmd_t       CloseAllCmd;
    DS_GetFileInfoCmd_t    GetFileInfoCmd;
    DS_AddMidCmd_t         AddMidCmd;
    DS_RemoveMidCmd_t      RemoveMidCmd;
    DS_SendHkCmd_t         SendHkCmd;
} UT_CmdBuf_t;

extern UT_CmdBuf_t UT_CmdBuf;

/*
 * Macro to add a test case to the list of tests to execute
 */
#define UT_DS_TEST_ADD(test) UtTest_Add(test, DS_Test_Setup, DS_Test_TearDown, #test)

/* Unit test MID */
#define DS_UT_MID_1 CFE_SB_ValueToMsgId(1)
#define DS_UT_MID_2 CFE_SB_ValueToMsgId(2)

/* Unit test osal ID, generic w/ no type */
#define DS_UT_OBJID_1 OS_ObjectIdFromInteger(1)

/* Unit test CDS handle */
#define DS_UT_CDSHANDLE_1 CFE_ES_CDSHANDLE_C(CFE_RESOURCEID_WRAP(1))

/*
 * Setup function prior to every test
 */
void DS_Test_Setup(void);

/*
 * Teardown function after every test
 */
void DS_Test_TearDown(void);

#endif
