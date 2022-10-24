/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - demolib
  File:     print.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "g2d_demolib.h"
#include "g2d_demolib/print.h"
#include "g2d_demolib/fontData.h"

#define SCREEN_HEIGHT       192     // screen height
#define SCREEN_WIDTH        256     // screen width

#define SCREEN_CHARA_SIZE   8       // BG character size

#define SCR_PLTT_SHIFT      12      // shift width for palette number in screen data
#define FONT_COLOR          3       // font color = blue

// character unit screen size
#define SCREEN_HEIGHT_CHARA (SCREEN_HEIGHT / SCREEN_CHARA_SIZE)
#define SCREEN_WIDTH_CHARA  (SCREEN_WIDTH / SCREEN_CHARA_SIZE)

// screen buffer
static u16 sScreenBuf[SCREEN_HEIGHT_CHARA][SCREEN_WIDTH_CHARA];


/*---------------------------------------------------------------------------*
  Name:         G2DDemo_PrintInit

  Description:  Sets up BG1 of main screen for character display.
                Depends on Init3DStuff_ process of system.c.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_PrintInit(void)
{
    // BG1 setup
    GX_SetBankForBGExtPltt(GX_VRAM_BGEXTPLTT_01_F);
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ );
    G2_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,
        GX_BG_COLORMODE_256,
        GX_BG_SCRBASE_0x0000,
        GX_BG_CHARBASE_0x04000,
        GX_BG_EXTPLTT_01);
    G2_SetBG1Priority(3);

    // Resource load
    GX_LoadBG1Char(fontCharData, 0, sizeof(fontCharData));
    
    GX_BeginLoadBGExtPltt();
    GX_LoadBGExtPltt(fontPlttData, 0x2000, sizeof(fontPlttData));
    GX_EndLoadBGExtPltt();

    // Clear internal buffer
    G2DDemo_PrintClear();
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_PrintClear

  Description:  Clears internal screen buffer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_PrintClear(void)
{
    MI_CpuClearFast( sScreenBuf, sizeof(sScreenBuf) );
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_PrintOut

  Description:  Displays a string using BG1 of the main screen.
                Displays up to 24 lines with 32 characters per line.

  Arguments:    x:      Specifies the column displaying the first character. Starts from 0.
                y:      Specifies the line to display. Starts from 0.
                string: The output character string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_PrintOut(int x, int y, const char* string)
{
    const char* pos;
    SDK_NULL_ASSERT( string );
    SDK_ASSERT( 0 <= x && x < SCREEN_WIDTH_CHARA );
    SDK_ASSERT( 0 <= y && y < SCREEN_HEIGHT_CHARA );

    for( pos = string; *pos != '\0'; ++pos )
    {
        sScreenBuf[y][x] = (u16)((u16)(FONT_COLOR << SCR_PLTT_SHIFT) | (u16)(*pos));
        x++;

        if( x >= SCREEN_WIDTH )
        {
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_PrintOutf

  Description:  Displays a string using BG1 of the main screen.
                Displays up to 24 lines with 32 characters per line.

  Arguments:    x:      Specifies the column displaying the first character. Starts from 0.
                y:      Specifies the line to display. Starts from 0.
                format: Format character string of the printf format.
                ...:    Parameters corresponding to format.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_PrintOutf(int x, int y, const char* format, ...)
{
    va_list vlist;
    char buf[SCREEN_WIDTH_CHARA + 1];
	SDK_NULL_ASSERT( format );
    
    va_start( vlist, format );
    (void)vsnprintf( buf, sizeof(buf), format, vlist );
    va_end( vlist );
    
    G2DDemo_PrintOut(x, y, buf);
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_PrintApplyToHW

  Description:  Transfers internal screen buffer to hardware.
                Usually called for each VBlank.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_PrintApplyToHW(void)
{
    DC_FlushRange( sScreenBuf, sizeof(sScreenBuf) );
    GX_LoadBG1Scr( sScreenBuf, 0, sizeof(sScreenBuf) );
}

