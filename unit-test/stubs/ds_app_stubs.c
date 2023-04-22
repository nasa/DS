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
 * Auto-Generated stub implementations for functions defined in ds_app header
 */

#include "ds_app.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppInitialize()
 * ----------------------------------------------------
 */
CFE_Status_t DS_AppInitialize(void)
{
    UT_GenStub_SetupReturnBuffer(DS_AppInitialize, CFE_Status_t);

    UT_GenStub_Execute(DS_AppInitialize, Basic, NULL);

    return UT_GenStub_GetReturnValue(DS_AppInitialize, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppMain()
 * ----------------------------------------------------
 */
void DS_AppMain(void)
{

    UT_GenStub_Execute(DS_AppMain, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppSendHkCmd()
 * ----------------------------------------------------
 */
void DS_AppSendHkCmd(void)
{

    UT_GenStub_Execute(DS_AppSendHkCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for DS_AppStorePacket()
 * ----------------------------------------------------
 */
void DS_AppStorePacket(CFE_SB_MsgId_t MessageID, const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(DS_AppStorePacket, CFE_SB_MsgId_t, MessageID);
    UT_GenStub_AddParam(DS_AppStorePacket, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(DS_AppStorePacket, Basic, NULL);
}
