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
 *   Coverage stub replacement for stdio.h
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utstubs.h"

#include "stub_stdio.h"

struct stub_FILE
{
    int f;
};

#define stub_STDIO_MAX_SIZE 0x01000000

int stub_fclose(stub_FILE *stream)
{
    int32 Status;

    Status = UT_DEFAULT_IMPL(stub_fclose);

    return Status;
}

char *stub_fgets(char *s, int n, stub_FILE *stream)
{
    int32  Status;
    size_t CopySize;

    Status = UT_DEFAULT_IMPL_RC(stub_fgets, stub_STDIO_MAX_SIZE);

    if (Status > 0)
    {
        if (Status > n)
        {
            CopySize = n;
        }
        else
        {
            CopySize = Status;
        }

        CopySize = UT_Stub_CopyToLocal(UT_KEY(stub_fgets), s, CopySize);

        if (CopySize != 0)
        {
            Status = CopySize;
        }
        else if (Status <= n)
        {
            memset(s, 'x', Status);
        }
        else if (UT_GetStubCount(UT_KEY(stub_fgets) < 4))
        {
            memset(s, 'x', n);
            Status = n;
        }
        else
        {
            Status = 0;
        }
    }

    if (Status <= 0)
    {
        return NULL;
    }

    return s;
}

stub_FILE *stub_fopen(const char *filename, const char *modes)
{
    int32            Status;
    stub_FILE *      retval;
    static stub_FILE FOPEN_FP = {0};

    Status = UT_DEFAULT_IMPL(stub_fopen);

    if (Status == 0)
    {
        retval = &FOPEN_FP;
    }
    else
    {
        retval = NULL;
    }

    return retval;
}

int stub_fputs(const char *s, stub_FILE *stream)
{
    int32 Status;

    Status = UT_DEFAULT_IMPL(stub_fputs);

    return Status;
}

int stub_putchar(int c)
{
    int32 Status;

    Status = UT_DEFAULT_IMPL(stub_putchar);

    return Status;
}

int stub_remove(const char *filename)
{
    int32 Status;

    Status = UT_DEFAULT_IMPL(stub_remove);

    return Status;
}

int stub_rename(const char *old, const char *nw)
{
    int32 Status;

    Status = UT_DEFAULT_IMPL(stub_rename);

    return Status;
}

int stub_snprintf(char *s, size_t maxlen, const char *format, ...)
{
    int32   Status;
    int     actual = 0;
    va_list ap;

    Status = UT_DEFAULT_IMPL(stub_snprintf);

    /* need to actually _do_ the snprintf */
    if (Status >= 0)
    {
        va_start(ap, format);
        actual = vsnprintf(s, maxlen, format, ap);
        va_end(ap);
    }

    if (Status != 0)
    {
        actual = Status;
    }

    return actual;
}

int stub_vsnprintf(char *s, size_t maxlen, const char *format, stub_va_list arg)
{
    int32 Status;
    int   actual = 0;

    Status = UT_DEFAULT_IMPL(stub_vsnprintf);

    /* need to actually _do_ something here -
     * cannot do the real vsnprintf because we lost the args. */
    if (Status >= 0)
    {
        actual = snprintf(s, maxlen, "%s", format);
    }

    if (Status != 0)
    {
        actual = Status;
    }

    return actual;
}

int stub_printf(const char *format, ...)
{
    return UT_DEFAULT_IMPL(stub_printf);
}

int stub_fprintf(stub_FILE *fp, const char *format, ...)
{
    return UT_DEFAULT_IMPL(stub_fprintf);
}

static stub_FILE LOCAL_FP[3] = {{10}, {11}, {12}};

stub_FILE *stub_stdin  = &LOCAL_FP[0];
stub_FILE *stub_stdout = &LOCAL_FP[1];
stub_FILE *stub_stderr = &LOCAL_FP[2];
