/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PAD
  File:     pad.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_PAD_H_
#define NITRO_PAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/pad/common/pad.h>

#ifdef	SDK_ARM9
#else
#include <nitro/pad/ARM7/xyButton.h>
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PAD_H_ */
#endif
