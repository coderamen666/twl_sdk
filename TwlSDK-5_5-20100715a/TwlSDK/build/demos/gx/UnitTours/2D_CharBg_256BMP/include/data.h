/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_CharBg_256BMP
  File:     data.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*
 ******************************************************************************
 *
 *  Project    :
 *               NITRO-SDK sample program
 *  Title      :
 *               Displays an affine extended 256 color bit map background.
 *  Explanation:
 *         Performs affine conversions (rotate / scale) on a 256-color bitmap background and displays the result.
 *            
 *
 ******************************************************************************
 */
#ifndef BG_DATA_H_
#define BG_DATA_H_

/*-------------------------- Data -------------------------------*/
extern const u16 bitmapBG_256color_Palette[256];
extern const u8 bitmapBG_256color_Texel[256 * 256 / 1];

#define SCREEN_SIZE	(256 * 256 / 1)
#define PALETTE_SIZE	(256 * 2)
#define CHAR_SIZE	(256 * 256 / 1)

#endif
