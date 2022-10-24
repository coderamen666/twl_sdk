/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     dispfunc.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#if !defined(NITROSDK_DEMOS_MB_MULTIBOOT_POWERSAVE_DISPFUNC_H_)
#define NITROSDK_DEMOS_MB_MULTIBOOT_POWERSAVE_DISPFUNC_H_


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif


/*****************************************************************************/
/* Constants */

/* Palette color for text */
enum
{
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    PURPLE,
    LIGHT_GREEN,
    HI_YELLOW,
    HI_CYAN,
    HI_WHITE,
    HI_BLUE,
    HI_PURPLE,
    HI_LIGHT_GREEN,
    BROWN,
    HI_BROWN,
    COLOR_NUM
};


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         BgInit

  Description:  Initializes BG for display of strings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgInit(void);

/*---------------------------------------------------------------------------*
  Name:         BgClear

  Description:  Initializes BG screen of the main screen so that all chars are set to 0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgClear(void);

/*---------------------------------------------------------------------------*
  Name:         BgUpdate

  Description:  Reflects BG screen in real memory.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgUpdate(void);

/*---------------------------------------------------------------------------*
  Name:         BgPutString

  Description:  Displays ASCII characters horizontally from BG's specified grid.

  Arguments:    x                Upper-left x grid to display. (8 pixel units)
                x                Upper-left y grid to display. (8 pixel units)
                pal              Palette number. (0-15)
                str              ASCII string to display.
                len              Length of string to display.
                                 If str contains a null character in a location less than len, length up to that position is used.
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgPutString(int x, int y, int pal, const char *str, int len);

/*---------------------------------------------------------------------------*
  Name:         BgPrintf

  Description:  Displays formatted ASCII string horizontally from BG's specified grid.

  Arguments:    x                Upper left x grid to display. (8 pixel units)
                x                Upper left y grid to display. (8 pixel units)
                pal              Palette number. (0-15)
                str              ASCII string to display.
                                 The supported format is that of OS_VSNPrintf().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgPrintf(int x, int y, int pal, const char *str, ...);

/*---------------------------------------------------------------------------*
  Name:         BgPutChar

  Description:  Displays a string on the BG specification grid.

  Arguments:    x                Upper left x grid to display. (8 pixel units)
                x                Upper left y grid to display. (8 pixel units)
                pal              Palette number. (0-15)
                c                BG character number to be displayed.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static  inline void BgPutChar(int x, int y, int pal, int c)
{
    const char tmp = (char)c;
    BgPutString(x, y, pal, &tmp, 1);
}

/*---------------------------------------------------------------------------*
  Name:         BgSetMessage

  Description:  Displays string at the (4, 22) position of both main and sub screens.

  Arguments:    pal              Palette number. (0-15)
                str              ASCII string to display.
                                 The supported format is that of OS_VSNPrintf().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    BgSetMessage(int pal, const char *str, ...);


#endif /* NITROSDK_DEMOS_MB_MULTIBOOT_POWERSAVE_DISPFUNC_H_ */
