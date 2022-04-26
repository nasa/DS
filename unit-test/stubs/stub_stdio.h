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
 * \file
 *   Coverage stub replacement for stdio.h
 */

#ifndef STUB_STDIO_H
#define STUB_STDIO_H

#include "stub_basetypes.h"
#include "stub_stdarg.h"

/* ----------------------------------------- */
/* constants normally defined in stdio.h */
/* ----------------------------------------- */

/* ----------------------------------------- */
/* types normally defined in stdio.h */
/* ----------------------------------------- */
typedef struct stub_FILE stub_FILE;

/* ----------------------------------------- */
/* prototypes normally declared in stdio.h */
/* ----------------------------------------- */

extern int        stub_fclose(stub_FILE *stream);
extern char *     stub_fgets(char *s, int n, stub_FILE *stream);
extern stub_FILE *stub_fopen(const char *filename, const char *modes);
extern int        stub_fputs(const char *s, stub_FILE *stream);
extern int        stub_remove(const char *filename);
extern int        stub_rename(const char *old, const char *nw);
extern int        stub_snprintf(char *s, size_t maxlen, const char *format, ...);
extern int        stub_vsnprintf(char *s, size_t maxlen, const char *format, stub_va_list arg);
extern int        stub_printf(const char *format, ...);
extern int        stub_fprintf(stub_FILE *fp, const char *format, ...);
extern int        stub_putchar(int c);

extern stub_FILE *stub_stdin;
extern stub_FILE *stub_stdout;
extern stub_FILE *stub_stderr;

#endif
