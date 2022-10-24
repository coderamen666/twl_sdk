/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-1
  File:     print.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
 
#include    "print.h"
 
extern u16 gScreen[32 * 32];           // Virtual screen
 
/*---------------------------------------------------------------------------*
  Name:         ClearStringY

  Description:  Clears the virtual screen (single line).

  Arguments:    y: Line to be deleted

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearStringY(s16 y)
{
    MI_CpuClearFast((void *)(&gScreen[(y * 32)]), sizeof(u16)*32);
}


/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  Clears the virtual screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearString(void)
{
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}


/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Positions the text string on the virtual screen. The string can be up to 32 chars.

  Arguments:    x: X-coordinate where character string starts (x 8 dots)
                y: Y-coordinate where character string starts (x 8 dots)
                palette: Specify text color by palette number
                text: Text string to position. Null-terminated.
                ...: Virtual argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char temp[32 + 2];
    s32 i;

    va_start(vlist, text);
    (void)OS_VSNPrintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}
