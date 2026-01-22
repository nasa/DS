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
 * \file
 *   Coverage stub replacement for stdio.h
 */

#ifndef OVERRIDE_STDIO_H
#define OVERRIDE_STDIO_H

#include "stub_stdio.h"

/* ----------------------------------------- */
/* mappings for declarations in stdio.h */
/* ----------------------------------------- */

#define FILE         stub_FILE
#define fclose       stub_fclose
#define fgets        stub_fgets
#define fopen        stub_fopen
#define fputs        stub_fputs
#define remove       stub_remove
#define rename       stub_rename
#define snprintf     stub_snprintf
#define vsnprintf    stub_vsnprintf
#define printf(...)  stub_printf(__VA_ARGS__)
#define fprintf(...) stub_fprintf(__VA_ARGS__)
#define putchar      stub_putchar

#define stdin  stub_stdin
#define stdout stub_stdout
#define stderr stub_stderr

#endif
