/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     code32.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

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
#if	defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#pragma thumb off
#elif	defined(SDK_ADS)
TO BE   DEFINED
#elif	defined(SDK_GCC)
TO BE   DEFINED
#endif
// Prepare to switch the definition of FX_Mul
#ifdef FX_Mul
#undef FX_Mul
#endif
// Prepare to switch the definition of FX_MulFx64c
#ifdef FX_Mul32x64c
#undef FX_Mul32x64c
#endif
// Prepare to switch the definition of MATH_CountLeadingZeros
#ifdef MATH_CountLeadingZeros
#undef MATH_CountLeadingZeros
#endif
// Because CPU is in ARM mode, inline versions are used.
#define FX_Mul(v1, v2)                 FX_MulInline(v1, v2)
#define FX_Mul32x64c(v32, v64c)        FX_Mul32x64cInline(v32, v64c)
#define MATH_CountLeadingZeros(x)      MATH_CountLeadingZerosInline(x)
