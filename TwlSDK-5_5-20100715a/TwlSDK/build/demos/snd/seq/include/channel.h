/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - seq
  File:     channel.h

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
#ifndef CHANNEL_H_
#define CHANNEL_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void    ChInit(void);
void    ChSetupWave(const u8 *wave_buf);
void    ChSetEvent(const u8 *midi_data);
void    ChAllNoteOff(void);

#ifdef __cplusplus
}
#endif

#endif // CHANNEL_H_
