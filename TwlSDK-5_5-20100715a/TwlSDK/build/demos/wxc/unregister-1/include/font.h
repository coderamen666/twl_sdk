/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - unregister-1
  File:     font.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef	FONT_H_
#define	FONT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<nitro/types.h>

extern const u32 d_CharData[8 * 256];
extern const u32 d_PaletteData[8 * 16];

enum
{
    FONT_BLACK = 0,
    FONT_RED,
    FONT_GREEN,
    FONT_BLUE,
    FONT_YELLOW,
    FONT_PUTPLE,
    FONT_LIGHT_BLUE,
    FONT_DARK_RED,
    FONT_DARK_GREEN,
    FONT_DARK_BLUE,
    FONT_DARK_YELLOW,
    FONT_DARK_PURPLE,
    FONT_DARK_LIGHT_BLUE,
    FONT_GRAY,
    FONT_DARK_GRAY,
    FONT_WHITE
};

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
