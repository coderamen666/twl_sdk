/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     nitro_win32.h

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
#ifndef NITRO_WIN32_H_
#define NITRO_WIN32_H_

//
//  NITRO HEADER for Win32 (VC,BCB or gcc)
//
#ifndef SDK_FROM_TOOL
#define SDK_FROM_TOOL
#endif

#ifndef SDK_WIN32
#define SDK_WIN32
#endif

#ifndef SDK_HAS_LONG_LONG_INT
#define SDK_HAS_NO_LONG_LONG_INT_
#endif

// Default SDK_ARM9
#ifndef SDK_ARM7
#define SDK_ARM9
#endif

#ifdef  _MSC_VER
#pragma warning(disable:4201)          // warning for nameless structure definition
#endif

#include <nitro/gx.h>

#ifndef SDK_NITRO
#define SDK_NITRO
#include <nitro/memorymap.h>
#undef SDK_NITRO
#else
#include <nitro/memorymap.h>
#endif

#include <nitro/types.h>
#include <nitro/fx/fx.h>
#include <nitro/fx/fx_const.h>

#include <nitro/os/ARM9/argument.h>

#ifdef  _MSC_VER
#pragma warning(default:4201)
#endif

#endif //NITRO_WIN32_H_
