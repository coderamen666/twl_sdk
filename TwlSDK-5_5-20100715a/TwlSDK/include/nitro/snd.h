/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - include
  File:     snd.h

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SND_H_
#define NITRO_SND_H_

/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))

#include <nitro/snd/common/main.h>
#include <nitro/snd/common/alarm.h>
#include <nitro/snd/common/command.h>
#include <nitro/snd/common/global.h>

#ifdef SDK_ARM9

#include <nitro/snd/ARM9/interface.h>

#endif // SDK_ARM9

#endif // SDK_FROM_TOOL

#include <nitro/snd/common/work.h>
#include <nitro/snd/common/channel.h>
#include <nitro/snd/common/capture.h>
#include <nitro/snd/common/exchannel.h>
#include <nitro/snd/common/data.h>
#include <nitro/snd/common/bank.h>
#include <nitro/snd/common/mml.h>
#include <nitro/snd/common/seq.h>
#include <nitro/snd/common/midiplayer.h>
#include <nitro/snd/common/util.h>

#endif /* NITRO_SND_H_ */
