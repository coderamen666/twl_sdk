/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     snd.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-02-06#$
  $Rev: 4051 $
  $Author: terui $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SND_H_
#define TWL_SND_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/snd.h>
#include    <twl/snd/common/sndex_common.h>

#ifdef  SDK_ARM9
#include    <twl/snd/ARM9/sndex.h>
#else   // SDK_ARM7
#include    <twl/snd/ARM7/sndex_api.h>
#endif

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   // extern "C"
#endif
#endif  // TWL_SND_H_
