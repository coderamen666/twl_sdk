/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_BmpBg_Vram
  File:     image.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 9005 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef IMAGE_H_
#define IMAGE_H_


/*************************************************************************************/
/*  Data format for VRAM Display mode                                                */
/*                                                                                   */
/*   d15  d14  d13  d11  d12 d10   d9   d8   d7   d6  d5    d4   d3   d2   d1  d0    */
/* --------------------------------------------------------------------------------- */
/* |    |        BLUE            |       GREEN            |        RED             | */
/* |    | B5 | B4 | B3 | B2 | B1 | G5 | G4 | G3 | G2 | G1 | R5 | R4 | R3 | R2 | R1 | */
/* --------------------------------------------------------------------------------- */
/*************************************************************************************/

extern const unsigned short IMAGE_VRAM256x192[4][256 * 192];

#endif
