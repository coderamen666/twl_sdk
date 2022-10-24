/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - mcs
  File:     print.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1316 $
 *---------------------------------------------------------------------------*/

#if ! defined(NNS_FINALROM)


#include <nitro.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <nnsys/misc.h>
#include <nnsys/mcs/baseCommon.h>
#include <nnsys/mcs/base.h>
#include <nnsys/mcs/print.h>
#include <nnsys/mcs/config.h>

#include "basei.h"
#include "printi.h"


/* ========================================================================
    Constants
   ======================================================================== */

static const int        SEND_RETRY_MAX = 10;


/* ========================================================================
    Static variables
   ======================================================================== */

static NNSMcsRecvCBInfo sCBInfo;
static void*            spBuffer    = NULL;
static u32              sBufferSize = 0;


/* ========================================================================
    Static functions
   ======================================================================== */

static NNS_MCS_INLINE BOOL
IsInitialized(void)
{
    return NULL != spBuffer;
}

static void
CallbackRecvDummy(
    const void* /*pRecv*/,
    u32         /*dwRecvSize*/,
    u32         /*dwUserData*/,
    u32         /*offset*/,
    u32         /*totalSize*/
)
{
    // Does nothing
}

static NNS_MCS_INLINE BOOL
CheckConnect()
{
    if (NNS_McsIsServerConnect())
    {
        return TRUE;
    }
    else
    {
        OS_Printf("NNS Mcs Print: not server connect\n");
        return FALSE;
    }
}

/*
    Sends data
        Retries several times until successful.
  */
static BOOL
WriteStream(
    const void* buf,
    u32         length
)
{
    int retryCount;

    for (retryCount = 0; retryCount < SEND_RETRY_MAX; ++retryCount)
    {
        u32 writableLength;
        if (! NNS_McsGetStreamWritableLength(&writableLength))
        {
            return FALSE;
        }

        if (length <= writableLength)
        {
            return NNS_McsWriteStream(NNSi_MCS_PRINT_CHANNEL, buf, length);
        }
        else
        {
            // OS_Printf("NNS Mcs Print: buffer short - retry\n");
            NNSi_McsSleep(16);
        }
    }

    OS_Printf("NNS Mcs Print: send time out\n");
    return FALSE;
}


/* ========================================================================
    External Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_McsInitPrint

  Description:  Initializes the file I/O API.

  Arguments:    pBuffer:  Pointer to the work buffer for output
                buffer:   Buffer size (in bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_McsInitPrint(
    void*   pBuffer,
    u32     size
)
{
    NNS_ASSERT(pBuffer != NULL);
    NNS_ASSERT(size > 1);

    if (IsInitialized())
    {
        return;
    }

    spBuffer    = pBuffer;      // Also means that initialization is complete
    sBufferSize = size;

    NNS_McsRegisterRecvCallback(&sCBInfo, NNSi_MCS_PRINT_CHANNEL, CallbackRecvDummy, 0);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_McsFinalizePrint

  Description:  Performs shutdown.
                Frees the work memory specified by the NNS_McsInitPrint function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_McsFinalizePrint()
{
    if (! IsInitialized())
    {
        return;
    }

    NNS_McsUnregisterRecvResource(NNSi_MCS_PRINT_CHANNEL);

    spBuffer    = NULL;
    sBufferSize = 0;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_McsPutString

  Description:  Outputs string to the MCS server's console.

  Arguments:    string:  String to output

  Returns:      TRUE if the function succeeds, FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL
NNS_McsPutString(
    const char* string
)
{
    NNS_ASSERT(IsInitialized());
    NNS_ASSERT(string);

    if (! CheckConnect())
    {
        return FALSE;
    }

    return WriteStream(string, strlen(string));
}

/*---------------------------------------------------------------------------*
  Name:         NNS_McsPrintf

  Description:  Outputs formatted string to MCS server's console.

  Arguments:    format:  Format-specified string
                ...   :  Arguments that can be omitted

  Returns:      TRUE if the function succeeds, FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL
NNS_McsPrintf(
    const char* format,
    ...
)
{
    int writeChars;
    va_list args;

    NNS_ASSERT(IsInitialized());
    NNS_ASSERT(format);

    if (! CheckConnect())
    {
        return FALSE;
    }

    va_start(args, format);

    writeChars = vsnprintf(spBuffer, sBufferSize, format, args);

    va_end(args);

    // With vsnprintf, if buffer cannot hold all, then the portion that it can hold is set there.
    // Also, 0 is set at the end.
    if (writeChars > 1)
    {
        return WriteStream(spBuffer, strlen(spBuffer));
    }

    return FALSE;
}

/* #if ! defined(NNS_FINALROM) */
#endif
