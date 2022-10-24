/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_PolAttr_1Dot
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

/*
 ******************************************************************************
 *
 *  Project    :
 *               NITRO-SDK sample program
 *  Title      :
 *               Setting GX_POLYGON_ATTR_MISC_DISP_1DOT in G3_PolygonAttr()
 *               (Uses depth values to suppress the display of single-pixel polygons)
 *  Explanation:
 *               Uses depth values to suppress the display of single-pixel polygons.
 ******************************************************************************
 */
#ifndef TEX_32768_H_
#define TEX_32768_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

/* 32768-color texture */
extern const u16 tex_32768_64x64[];

#endif
