/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - demolib
  File:     print.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "g3d_demolib/print.h"
#include "g3d_demolib/font.h"

#define SCREEN_HEIGHT       192     // screen height
#define SCREEN_WIDTH        256     // screen width

#define SCREEN_CHARA_SIZE   8       // BG character size

#define SCR_PLTT_SHIFT      12      // shift width for palette number in screen data
#define FONT_COLOR          4       // font color = blue

// character unit screen size
#define SCREEN_HEIGHT_CHARA (SCREEN_HEIGHT / SCREEN_CHARA_SIZE)
#define SCREEN_WIDTH_CHARA  (SCREEN_WIDTH / SCREEN_CHARA_SIZE)

// screen buffer
static u16 sScreenBuf[SCREEN_HEIGHT_CHARA][SCREEN_WIDTH_CHARA];


/*---------------------------------------------------------------------------*
  Name:         G3DDemo_InitConsole

  Description:  Sets up BG1 of main screen for character display.
                Depends on Init3DStuff_ process of system.c.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_InitConsole(void)
{
    // Resource load
    GX_LoadBG1Char(fontCharData, 0, sizeof(fontCharData));
	GX_LoadBGPltt (fontPlttData, 0, 0x00200);

    // Clear internal buffer
    G3DDemo_ClearConsole();
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_ClearConsole

  Description:  Clears internal screen buffer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_ClearConsole(void)
{
    MI_CpuClearFast( sScreenBuf, sizeof(sScreenBuf) );
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_Print

  Description:  Displays a string using BG1 of the main screen.
                Displays up to 24 lines with 32 characters per line.

  Arguments:    x:      Specifies the column displaying the first character. Starts from 0.
                y:      Specifies the line to display. Starts from 0.
                string: The output character string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_Print(int x, int y, int color, const char* string)
{
    const char* pos;
    SDK_NULL_ASSERT( string );
    SDK_ASSERT( 0 <= x && x < SCREEN_WIDTH_CHARA );
    SDK_ASSERT( 0 <= y && y < SCREEN_HEIGHT_CHARA );

    for( pos = string; *pos != '\0'; ++pos )
    {
        sScreenBuf[y][x] = (u16)((u16)(color << SCR_PLTT_SHIFT) | (u16)(*pos));
        x++;

        if( x >= SCREEN_WIDTH )
        {
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_Printf

  Description:  Displays a string using BG1 of the main screen.
                Displays up to 24 lines with 32 characters per line.

  Arguments:    x:      Specifies the column displaying the first character. Starts from 0.
                y:      Specifies the line to display. Starts from 0.
                format: Format character string of the printf format.
                ...:    Parameters corresponding to format.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_Printf(int x, int y, int color, const char* format, ...)
{
    va_list vlist;
    char buf[SCREEN_WIDTH_CHARA + 1];
	SDK_NULL_ASSERT( format );
    
    va_start( vlist, format );
    (void)vsnprintf( buf, sizeof(buf), format, vlist );
    va_end( vlist );
    
    G3DDemo_Print(x, y, color, buf);
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_PrintApplyToHW

  Description:  Transfers internal screen buffer to hardware.
                Usually called for each VBlank.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_PrintApplyToHW(void)
{
    DC_FlushRange( sScreenBuf, sizeof(sScreenBuf) );
    GX_LoadBG1Scr( sScreenBuf, 0, sizeof(sScreenBuf) );
}

