/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_CharBg_4
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
// A sample that scrolls BG #0 (text mode)
//
// USAGE:
//   UP, DOWN, LEFT, RIGHT: Scrolls a sphere
//   SELECT:                Move sphere to the initial position
//
// HOWTO:
// 1. Set up the character/palette/screen data the same as Sub_CharBg_1
// 2. Specify an offset with G2S_SetBGxOffset
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
    int     offset_H = 0, offset_V = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // Enables IRQ interrupts, initializes VRAM, and sets BG #0 for text mode
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplaySubBG0Only();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GXS_LoadBG0Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GXS_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));

    {
        int     i, j;
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                sScrnBuf[(i * 32) + j] = (u16)((i * 0x10) + j);
            }
        }
    }

    // Store the data in the main memory, and invalidate the cache
    DC_FlushRange(sScrnBuf, sizeof(sScrnBuf));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();

    // DMA Transfer to BG #0 screen
    GXS_LoadBG0Scr(sScrnBuf, 0, sizeof(sScrnBuf));

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        DEMOReadKey();

        if (DEMO_IS_PRESS(PAD_KEY_UP))
            offset_V += 2;
        if (DEMO_IS_PRESS(PAD_KEY_DOWN))
            offset_V -= 2;
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
            offset_H -= 2;
        if (DEMO_IS_PRESS(PAD_KEY_LEFT))
            offset_H += 2;
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
        {
            offset_H = 0;
            offset_V = 0;
        }

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt

        // Setting the offset for BG #0 plane
        G2S_SetBG0Offset(offset_H, offset_V);
    }
}

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
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}
