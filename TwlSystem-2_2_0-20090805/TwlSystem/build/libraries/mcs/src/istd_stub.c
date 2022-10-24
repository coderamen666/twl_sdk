/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - mcs
  File:     istd_stub.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1488 $
 *---------------------------------------------------------------------------*/

#if ! defined(NNS_FINALROM)

#include "istd_stubi.h"

__declspec(weak)
void
ISTDHIOInit()
{
}

__declspec(weak)
u32
ISTDHIOGetDevMask()
{
    return 0;
}

__declspec(weak)
BOOL
ISTDHIOOpen(u32 /* fDevMask */)
{
    return FALSE;
}

__declspec(weak)
BOOL
ISTDHIOClose()
{
    return FALSE;
}

__declspec(weak)
BOOL
ISTDSIOSend(u16 /* chn */, const void * /* pSrc */, u32 /* nSize */)
{
    return FALSE;
}

__declspec(weak)
void
ISTDSIOSetRecvCallback(ISTDSIORecvCbFunc /* cbRecv */, void * /* pUser */)
{
}

/* #if ! defined(NNS_FINALROM) */
#endif
