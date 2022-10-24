/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - capture
  File:     fanfare.32.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef FANFARE_32_H_
#define FANFARE_32_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#define FANFARE_32_FORMAT    SND_WAVE_FORMAT_PCM16
#define FANFARE_32_LOOPFLAG  0
#define FANFARE_32_RATE      32000
#define FANFARE_32_TIMER     523
#define FANFARE_32_LOOPSTART 0
#define FANFARE_32_LOOPLEN   56926

#ifdef __cplusplus
extern "C" {
#endif

const s16 fanfare_32[];

#ifdef __cplusplus
}
#endif

#endif // FANFARE_32_H_
