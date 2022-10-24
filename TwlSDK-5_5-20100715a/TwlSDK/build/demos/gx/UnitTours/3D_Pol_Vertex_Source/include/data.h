/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Vertex_Source
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
 *               Sets GX_TEXGEN_VERTEX with G3_TexImageParam().
 *               (Shows fixed viewpoint textures)
 *  Explanation:
 *               Uses a setting of GX_TEXGEN_VERTEX with G3_TexImageParam() to show fixed viewpoint textures.
 *             
 *
 ******************************************************************************
 */
#ifndef TEX_4PLETT_H_
#define TEX_4PLETT_H_

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

/* 4-color palette texture */
extern const u16 tex_4plett64x64[];

/* 4-color palette */
extern const u16 pal_4plett[];

#endif
