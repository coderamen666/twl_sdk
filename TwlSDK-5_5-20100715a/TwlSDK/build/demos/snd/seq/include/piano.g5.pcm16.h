/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - seq
  File:     piano.g5.pcm16.h

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
#ifndef PIANO_G5_PCM16_H_
#define PIANO_G5_PCM16_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#define PIANO_G5_PCM16_FORMAT    SND_WAVE_FORMAT_PCM16
#define PIANO_G5_PCM16_LOOPFLAG  1
#define PIANO_G5_PCM16_RATE      32009
#define PIANO_G5_PCM16_TIMER     523
#define PIANO_G5_PCM16_LOOPSTART 9007
#define PIANO_G5_PCM16_LOOPLEN   1632

#ifdef __cplusplus
extern "C" {
#endif

const s16 piano_g5_pcm16[];

#ifdef __cplusplus
}
#endif

#endif // PIANO_G5_PCM16_H_
