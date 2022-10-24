/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - demolib - include - g3d_demolib
  File:     graphics.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_G3DDEMO_GRAPHICS_H_
#define NNS_G3DDEMO_GRAPHICS_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
	G3DDemoGround structure

 *---------------------------------------------------------------------------*/
typedef struct
{
	u16			groundEnable;
	GXRgb		wireColor;
	GXRgb		backColor;
	u16			backAlpha;
	fx32		scale;
}
G3DDemoGround;

/*---------------------------------------------------------------------------*
	Function Prototypes

 *---------------------------------------------------------------------------*/

void G3DDemo_InitDisplay(void);
void G3DDemo_InitGround(G3DDemoGround* stage, fx32 scale);
void G3DDemo_DrawGround(G3DDemoGround* stage);

#ifdef __cplusplus
}/* extern "C" */
#endif

// NNS_G3DDEMO_GRAPHICS_H_
#endif
