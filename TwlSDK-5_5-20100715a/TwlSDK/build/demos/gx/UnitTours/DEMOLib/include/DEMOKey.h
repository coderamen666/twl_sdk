/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOKey.h

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
#ifndef DEMO_KEY_H_
#define DEMO_KEY_H_
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u16     trigger;
    u16     press;
}
DEMOKeyWork;

extern DEMOKeyWork gKeyWork;
void    DEMOReadKey(void);

#define DEMO_IS_TRIG(key)  (gKeyWork.trigger & (key))
#define DEMO_IS_PRESS(key) (gKeyWork.press & (key))

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif /* DEMO_KEY_H_ */
