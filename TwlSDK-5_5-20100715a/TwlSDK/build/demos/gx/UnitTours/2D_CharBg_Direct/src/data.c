/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_CharBg_Direct
  File:     data.c

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
 *               NITRO-SDK Sample Program
 *  Title      :
 *               Data for displaying a direct-color bitmap for an extended affine BG
 *  Explanation:
 *               Rotates and scales the 256-color direct-color bitmap for an extended affine BG and then displays it
 *             
 *
 ******************************************************************************
 */
// Texture Information
// type:          GX_TEXFMT_DIRECT
// size_width:    256
// size_height:   256

const unsigned short bitmapBG_directcolor_Texel[256 * 256] = {
#include "picture_256_256.autogen.c"
};
