 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - stubs - isd
  File:     stubs_isd.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
      Function prototype for IS NITRO Debugger Console Functions
 *---------------------------------------------------------------------------*/
void    ISDPrintf(const char *, ...);
void    ISDPrint(const char *);
void    _ISDbgLib_Initialize(void);
void    _ISDbgLib_AllocateEmualtor(void);
void    _ISDbgLib_FreeEmulator(void);
void    _ISDbgLib_RegistOverlayInfo(void);
void    _ISDbgLib_UnregistOverlayInfo(void);
void    ISDPrintSetBlockingMode(int);
int     ISDPrintGetBlockingMode(void);

/*---------------------------------------------------------------------------*
      Dummy Function for IS NITRO Debugger Console Functions
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void ISDPrintf(const char *, ...)
{
}

SDK_WEAK_SYMBOL void ISDPrint(const char *)
{
}

SDK_WEAK_SYMBOL void _ISDbgLib_Initialize(void)
{
}

SDK_WEAK_SYMBOL void _ISDbgLib_AllocateEmualtor(void)
{
}

SDK_WEAK_SYMBOL void _ISDbgLib_FreeEmulator(void)
{
}

SDK_WEAK_SYMBOL void _ISDbgLib_RegistOverlayInfo(void)
{
}

SDK_WEAK_SYMBOL void _ISDbgLib_UnregistOverlayInfo(void)
{
}

SDK_WEAK_SYMBOL void ISDPrintSetBlockingMode(int)
{
}

SDK_WEAK_SYMBOL int ISDPrintGetBlockingMode(void)
{
    return 0;
}
