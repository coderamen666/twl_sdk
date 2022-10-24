/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wmPadRead-child
  File:     data.h

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
#ifndef __DATA_H
#define __DATA_H

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

extern const u32 sampleCharData[8 * 0x100];
extern const u16 samplePlttData[16][16];

#endif // __DATA_H
