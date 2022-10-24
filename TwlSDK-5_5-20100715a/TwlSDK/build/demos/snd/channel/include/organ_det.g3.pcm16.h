/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - channel
  File:     organ_det.g3.pcm16.h

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
#ifndef ORGAN_DET_G3_PCM16_H_
#define ORGAN_DET_G3_PCM16_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#define ORGAN_DET_G3_PCM16_FORMAT    SND_WAVE_FORMAT_PCM16
#define ORGAN_DET_G3_PCM16_LOOPFLAG  1
#define ORGAN_DET_G3_PCM16_RATE      32000
#define ORGAN_DET_G3_PCM16_TIMER     523
#define ORGAN_DET_G3_PCM16_LOOPSTART 1995
#define ORGAN_DET_G3_PCM16_LOOPLEN   11959

#ifdef __cplusplus
extern "C" {
#endif

const s16 organ_det_g3_pcm16[];

#ifdef __cplusplus
}
#endif

#endif // ORGAN_DET_G3_PCM16_H_
