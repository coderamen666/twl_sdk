/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Window_HDMA
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
//---------------------------------------------------------------------------
//  A sample that transforms a window with HDMA
//
//  Background image is displayed only in transformed window
//
//  HOWTO:
//  1.  Set HDMA copy function in V-Blank period
//
//---------------------------------------------------------------------------
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"


//---------------------------------------------------------------------------
//  DMA No. for window transforming HDMA
//---------------------------------------------------------------------------
#define WINDOW_TRANSFORM_HDMA_NO    3

//---------------------------------------------------------------------------
//  Array for window position per line
//---------------------------------------------------------------------------
u16     s_WindowSizePerLine[192];
BOOL    preparedGraphic;

//---------------------------------------------------------------------------
// V-Blank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call OS_SetIrqCheckFlag to check a V-Blank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    // Don't use DMA #3 before "Load graphic data" function has finished
    if (preparedGraphic)
    {
        // Stop HDMA
        MI_StopDma(WINDOW_TRANSFORM_HDMA_NO);
        // Set Window horizontal position (first line)
        G2_SetWnd0Position(s_WindowSizePerLine[0] >> 8, 0, s_WindowSizePerLine[0] & 0x0ff, 192);
        // Start HDMA which copies window position array to "Window size register"
        MI_HBlankDmaCopy16(WINDOW_TRANSFORM_HDMA_NO,
                           &(s_WindowSizePerLine[1]), (void *)REG_WIN0H_ADDR, 2);
    }

    // Set flag that checks for V-Blank interrupt
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//---------------------------------------------------------------------------
//  Create window horizontal position table per line
//---------------------------------------------------------------------------
static void CreateWindowStyle(void)
{
    int     i;
    int     size;
    u16     left_pos;
    u16     right_pos;

    (void)OS_DisableIrq();
    for (i = 0; i < 96; i++)
    {
        size = (256 * i) / 96;
        left_pos = (u16)(128 - (size / 2));
        right_pos = (u16)(128 + (size / 2));
        s_WindowSizePerLine[i] = (u16)((left_pos << 8) | right_pos);
    }
    for (i = 96; i < 192; i++)
    {
        s_WindowSizePerLine[i] = s_WindowSizePerLine[191 - i];
    }
    (void)OS_EnableIrq();

    // Store the data in main memory and invalidate the cache
    DC_FlushRange((void *)s_WindowSizePerLine, sizeof(s_WindowSizePerLine));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
}

//---------------------------------------------------------------------------
//  Main
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    // Initialize
    preparedGraphic = FALSE;
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBG0Only();

    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);

    // Load 2D graphic data
    GX_LoadBG0Char(d_natsunoumi_schDT, 0, sizeof(d_natsunoumi_schDT));
    GX_LoadBGPltt(d_natsunoumi_sclDT, 0, sizeof(d_natsunoumi_sclDT));
    GX_LoadBG0Scr(d_natsunoumi_sscDT, 0, sizeof(d_natsunoumi_sscDT));

    // Window settings
    GX_SetVisibleWnd(GX_WNDMASK_W0);   // Turn on WINDOW-0
    // Set inside of window visible BG0
    G2_SetWnd0InsidePlane(GX_WND_PLANEMASK_BG0, FALSE);
    // Set outside of window non-visible
    G2_SetWndOutsidePlane(GX_WND_PLANEMASK_NONE, FALSE);

    // Create window horizontal position table
    CreateWindowStyle();

    // Turn on flag that specifies whether graphics data has been loaded
    preparedGraphic = TRUE;

    DEMOStartDisplay();

    // Main loop
    while (1)
    {
        // Wait for V-Blank
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x7490C85D);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
