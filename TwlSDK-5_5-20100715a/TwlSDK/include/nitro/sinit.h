/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     sinit.h

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
#if	defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
//
// Setup NitroStaticInit
//
static void NitroStaticInit(void);
#pragma define_section SINIT ".sinit" abs32 RWX
#pragma section        SINIT begin
SDK_FORCE_EXPORT static void *NitroStaticInit_[] = { NitroStaticInit };
#pragma section        SINIT end

#elif	defined(SDK_ADS)

TO BE   DEFINED
#elif	defined(SDK_GCC)

TO BE   DEFINED
#endif
