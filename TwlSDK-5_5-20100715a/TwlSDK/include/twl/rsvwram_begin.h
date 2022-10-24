/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl
  File:     rsvwram_begin.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
//
// include this instead of using C99 pragma extensions for compatibility
//
#include <twl/section.h>

#ifdef SDK_TWL

#ifdef SDK_TWLHYB

#include <twl/ltdwram_begin.h>

#else // SDK_TWLHYB

#if	defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#pragma section RSVWRAM begin
#elif	defined(SDK_ADS)
TO BE   DEFINED
#elif	defined(SDK_GCC)
TO BE   DEFINED
#endif

#endif // SDK_TWLHYB

#endif // SDK_TWL
