/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_High_Light
  File:     data.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef _DATA_H_
#define _DATA_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

/* Chair model */
extern const u32 isu1[];
extern const u32 isu2[];

/* Chair size */
extern const u32 isu1_size;
extern const u32 isu2_size;

/* Chair texture */
extern const u16 tex_isu1[];
extern const u16 tex_isu2[];

/* Chair palette */
extern const u16 pal_isu1[];
extern const u16 pal_isu2[];

/* Chair texture and palette size */
extern const u32 tex_isu1_size;
extern const u32 tex_isu2_size;

extern const u32 pal_isu1_size;
extern const u32 pal_isu2_size;

#endif /* _DATA_H_ */
