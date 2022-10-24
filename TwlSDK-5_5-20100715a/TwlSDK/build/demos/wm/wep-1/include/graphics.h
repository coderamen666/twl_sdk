/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wep-1
  File:     graphics.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#define PSEUDO2D_ONE 64

extern void initGraphics(void);
extern void setupPseudo2DCamera(void);
extern void drawPseudo2DTexQuad(int sx, int sy, int width, int height, int texw, int texh);
extern void drawPseudo2DColorQuad(int sx, int sy, int width, int height, GXRgb color);

#endif
