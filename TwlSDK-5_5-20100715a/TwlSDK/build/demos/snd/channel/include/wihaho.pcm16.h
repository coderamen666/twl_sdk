/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - channel
  File:     wihaho.pcm16.h

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
#ifndef WIHAHO_PCM16_H_
#define WIHAHO_PCM16_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#define WIHAHO_PCM16_FORMAT    SND_WAVE_FORMAT_PCM16
#define WIHAHO_PCM16_LOOPFLAG  0
#define WIHAHO_PCM16_RATE      44100
#define WIHAHO_PCM16_TIMER     379
#define WIHAHO_PCM16_LOOPSTART 0
#define WIHAHO_PCM16_LOOPLEN   27016

#ifdef __cplusplus
extern "C" {
#endif

const s16 wihaho_pcm16[];

#ifdef __cplusplus
}
#endif

#endif // WIHAHO_PCM16_H_
