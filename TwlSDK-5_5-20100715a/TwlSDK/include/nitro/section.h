/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - 
  File:     section.h

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
#ifndef NITRO_SECTION_H_
#define NITRO_SECTION_H_

#ifdef __cplusplus
extern "C" {
#endif


//--------------------------------------------------------------------------------
//       section definition for LCF
#if     defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#ifdef SDK_ARM9
#pragma define_section ITCM ".itcm" ".itcm.bss" abs32 RWX
#pragma define_section DTCM ".dtcm" ".dtcm.bss" abs32 RWX
#else
#pragma define_section WRAM ".wram" ".wram.bss" abs32 RWX
#pragma define_section MAIN ".main" ".main.bss" abs32 RWX
#endif
#pragma define_section PARENT ".parent" abs32 RWX
#pragma define_section VERSION ".version" abs32 RWX
#elif   defined(SDK_PRODG)
// nothing to do
#elif	defined(SDK_ADS)
TO BE   DEFINED
#elif	defined(SDK_GCC)
TO BE   DEFINED
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif
/* NITRO_SECTION_H_ */
#endif
