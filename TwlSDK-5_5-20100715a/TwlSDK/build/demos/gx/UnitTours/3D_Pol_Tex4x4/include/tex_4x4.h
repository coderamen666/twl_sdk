/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Tex4x4
  File:     tex_4x4.h

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
#ifndef TEX_4X4_H_
#define TEX_4X4_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

/* 4x4 compressed texture */
extern const u32 tex_4x4_64x64[];

/* 4x4 compressed index */
extern const u16 index_4x4index[];

/* 4x4 compressed palette */
extern const u16 pal_4x4plett[];

#endif
