/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     dispfunc.c

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

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include "dispfunc.h"


/*****************************************************************************/
/* Constants */

/* Constant array for each type of rendering */
extern const u32 sampleCharData[8 * 0xe0];
extern const u16 samplePlttData[16][16];


/*****************************************************************************/
/* Variables */

/* Virtual BG screen for V-Blank transfer */
static u16 g_screen[2][32 * 32] ATTRIBUTE_ALIGN(32);

/* Current drawing target */
static u16 *draw_target;

/* Newest line number in message log */
static int cur_line = 0;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         BgiGetTargetScreen

  Description:  Gets pointer to the main/sub screen's BG screen.

  Arguments:    is_lcd_main: If main screen, TRUE; if sub-screen, FALSE.

  Returns:      Pointer to BG screen.
 *---------------------------------------------------------------------------*/
static  inline u16 *BgiGetTargetScreen(BOOL is_lcd_main)
{
    return g_screen[!is_lcd_main];
}

/*---------------------------------------------------------------------------*
  Name:         BgiSetTargetScreen

  Description:  Switches between the main/sub-screen BG screen for the display of character strings.

  Arguments:    is_lcd_main: If main screen, TRUE; if sub-screen, FALSE.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static  inline void BgiSetTargetScreen(BOOL is_lcd_main)
{
    draw_target = BgiGetTargetScreen(is_lcd_main);
}

/*---------------------------------------------------------------------------*
  Name:         BgInit

  Description:  Initializes BG for display of character strings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgInit(void)
{
    /* Initialize internal variables */
    cur_line = 0;
    MI_CpuClearFast(g_screen, sizeof(g_screen));
    DC_StoreRange(g_screen, sizeof(g_screen));
    DC_WaitWriteBufferEmpty();
    draw_target = BgiGetTargetScreen(TRUE);

    /*
     * Configure the main LCD.
     * BG0: Use to display ASCII characters.
     *   -256x256x16 text BG
     *   -Screen base 30 (0F000-0F800)
     *   -Character base 0 (00000-04000)
     */
    GX_SetBankForBG(GX_VRAM_BG_64_E);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_SetBG0Offset(0, 0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_LoadBG0Scr(g_screen[0], 0, sizeof(g_screen[0]));
    GX_LoadBG0Char(sampleCharData, 0, sizeof(sampleCharData));
    GX_LoadBGPltt(samplePlttData, 0, sizeof(samplePlttData));
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    /*
     * Configure the sub-LCD.
     * BG0: Use to display ASCII characters.
     *   -256x256x16 text BG
     *   -Screen base 31 (0F800-10000)
     *   -Character base 4 (10000-14000)
     */
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_SetBG0Offset(0, 0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_LoadBG0Scr(g_screen[1], 0, sizeof(g_screen[1]));
    GXS_LoadBG0Char(sampleCharData, 0, sizeof(sampleCharData));
    GXS_LoadBGPltt(samplePlttData, 0, sizeof(samplePlttData));
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);
}

/*---------------------------------------------------------------------------*
  Name:         BgClear

  Description:  Initializes BG screen of the main screen so that all chars are set to 0

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgClear(void)
{
    MI_CpuClearFast(g_screen[0], sizeof(g_screen[0]));
}

/*---------------------------------------------------------------------------*
  Name:         BgUpdate

  Description:  Reflect BG screen in real memory

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgUpdate(void)
{
    /* Reflect BG in real memory */
    DC_FlushRange(g_screen, sizeof(g_screen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(g_screen[0], 0, sizeof(g_screen[0]));
    GXS_LoadBG0Scr(g_screen[1], 0, sizeof(g_screen[1]));
}

/*---------------------------------------------------------------------------*
  Name:         BgPutString

  Description:  Displays ASCII characters horizontally from BG's specified grid.

  Arguments:    x: Upper left X grid to display (8 pixel units)
                y: Upper left Y grid to display (8 pixel units)
                pal: Palette number (0-15)
                str: ASCII character string to display
                len: Length of character string to display
                     (If str contains a NULL in a location less than len, length up to that position will be used)
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPutString(int x, int y, int pal, const char *str, int len)
{
    u16    *const dst = draw_target;
    x += y * 32;
    str -= x, len += x;
    for (; str[x] && (x < len); ++x)
    {
        dst[x & ((32 * 32) - 1)] = (u16)((pal << 12) | (u8)str[x]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         BgPrintf

  Description:  Displays formatted ASCII character string horizontally from BG's specified grid.

  Arguments:    x: Upper left X grid to display (8 pixel units)
                y: Upper left Y grid to display (8 pixel units)
                pal: Palette number (0-15)
                str: ASCII character string to display
                     (The supported format is that of OS_VSNPrintf)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPrintf(int x, int y, int pal, const char *str, ...)
{
    char    tmp[32 + 1];
    va_list vlist;
    va_start(vlist, str);
    (void)OS_VSNPrintf(tmp, sizeof(tmp), str, vlist);
    va_end(vlist);
    BgPutString(x, y, pal, tmp, sizeof(tmp));
}

/*---------------------------------------------------------------------------*
  Name:         BgSetMessage

  Description:  Displays character string at the (4, 22) position of both main and sub screens.

  Arguments:    pal: Palette number (0-15)
                str: ASCII character string to display
                     (The supported format is that of OS_VSNPrintf)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgSetMessage(int pal, const char *str, ...)
{
    char    tmp[32 + 1];
    va_list vlist;
    va_start(vlist, str);
    (void)OS_VSNPrintf(tmp, sizeof(tmp), str, vlist);
    va_end(vlist);

    /* Display character string at the (4, 22) position of main screen */
    BgPutString(4, 22, pal, "                            ", 28);
    BgPutString(4, 22, pal, tmp, 28);
    /* Display another character string at newest line of sub screen */
    {
        /* Scroll as necessary */
        const int BG_LINES = 24;
        if (cur_line == BG_LINES)
        {
            u16    *const sub_screen = BgiGetTargetScreen(FALSE);
            MI_CpuCopy16(&sub_screen[32 * 1], sub_screen, sizeof(u16) * 32 * (BG_LINES - 1));
            MI_CpuClear16(&sub_screen[32 * (BG_LINES - 1)], sizeof(u16) * 32 * 1);
            --cur_line;
        }
        /* Switch the output destination temporarily */
        BgiSetTargetScreen(FALSE);
        BgPutString(0, cur_line, pal, tmp, sizeof(tmp));
        BgiSetTargetScreen(TRUE);
        if (cur_line < BG_LINES)
            ++cur_line;
    }
}
