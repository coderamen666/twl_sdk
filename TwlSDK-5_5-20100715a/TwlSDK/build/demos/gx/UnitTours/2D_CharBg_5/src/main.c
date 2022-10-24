/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_CharBg_5
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
// A sample that flips all characters on the screen horizontally/vertically:
//
// USAGE:
//   A: Flips horizontally
//   B: Flips vertically
//
// HOWTO:
// 1. Set up the character/palette/screen data same as 2D_CharBG_1.
// 2. Set up horizontal/vertical flip bits of the screen data.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

static u16 sScrnBuf[SCREEN_SIZE];      // Buffer for screen data (BG #0)

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    //---------------------------------------------------------------------------
    // Initialize:
    // Enables IRQ interrupts, initializes VRAM, and sets BG #0 for text mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBG0Only();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GX_LoadBG0Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GX_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        int     flip_H = 0, flip_V = 0;
        int     i, j;

        DEMOReadKey();

        if (DEMO_IS_PRESS(PAD_BUTTON_A))
            flip_H = 1;
        if (DEMO_IS_PRESS(PAD_BUTTON_B))
            flip_V = 1;

#ifdef SDK_AUTOTEST
        flip_H = 1;
        flip_V = 1;                    // default params for testing.
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xC776DFB4);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                sScrnBuf[(i * 32) + j] = (u16)(flip_V << 11 | flip_H << 10 | ((i * 0x10) + j));
            }
        }
        // Store the data in the main memory, and invalidate the cache.
        DC_FlushRange(sScrnBuf, sizeof(sScrnBuf));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();

        OS_WaitVBlankIntr();           // Waiting for the end of the VBlank interrupt

        GX_LoadBG0Scr(sScrnBuf, 0, sizeof(sScrnBuf));
    }
}

//---------------------------------------------------------------------------
// VBlank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call 'OS_SetIrqCheckFlag' to check a VBlank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking VBlank interrupt
}
