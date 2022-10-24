/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - STD
  File:     std_unicode_array.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#if defined(SDK_ARM9) || !defined(SDK_NITRO)
// Because of memory restrictions, it is not possible to use the Unicode conversion feature with ARM7 in NITRO mode

#if defined(SDK_ARM9)
#define STD_UNICODE_STATIC_IMPLEMENTATION
#endif

#if defined(STD_UNICODE_STATIC_IMPLEMENTATION)
// The array pointers are weak symbols; in HYBRID mode, the code table is sent to ltdmain.
// As weak symbols, however, they are not overwritten externally if they are placed in the same file. The array pointers have therefore been separated into std_unicode_array.c.
// 
// 

#include "sjis2unicode.h"
#include "unicode2sjis.h"

SDK_WEAK_SYMBOL const u8    *STD_Unicode2SjisArray  = unicode2sjis_array;
SDK_WEAK_SYMBOL const u16   *STD_Sjis2UnicodeArray  = sjis2unicode_array;
#endif

#endif // defined(SDK_ARM9) || !defined(SDK_NITRO)
