/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_CharBg_6
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
// A sample that apply mosaic effect on BG #0:
//
// USAGE:
//   UP, DOWN:    Control the mosaic size in a vertical direction
//   LEFT, RIGHT: Control the mosaic size in a horizontal direction
//
// HOWTO:
// 1. Set up the character/palette/screen data the same as Sub_CharBg_1
// 2. Enable mosaic on BGs with G2S_BGxMosaic(TRUE)
// 3. Specify the mosaic size with G2S_SetBGMosaicSize
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
    int     mosaic_H = 0, mosaic_V = 0;

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

    G2S_BG0Mosaic(TRUE);               // enables mosaic on BG #0

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
    // Store the data in main memory, and invalidate the cache
    DC_FlushRange(sScrnBuf, sizeof(sScrnBuf));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();

    // DMA Transfer to BG0 screen
    GXS_LoadBG0Scr(sScrnBuf, 0, sizeof(sScrnBuf));

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        DEMOReadKey();

        if (DEMO_IS_PRESS(PAD_KEY_DOWN))
            mosaic_V++;
        if (DEMO_IS_PRESS(PAD_KEY_UP))
            mosaic_V--;
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
            mosaic_H++;
        if (DEMO_IS_PRESS(PAD_KEY_LEFT))
            mosaic_H--;
        if (mosaic_H > 15)
            mosaic_H = 15;
        if (mosaic_H < 0)
            mosaic_H = 0;
        if (mosaic_V > 15)
            mosaic_V = 15;
        if (mosaic_V < 0)
            mosaic_V = 0;
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
        {
            mosaic_H = 0;
            mosaic_V = 0;
        }

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt

        //---------------------------------------------------------------------------
        // Specify the horizontal/vertical size of mosaics on BGs
        //---------------------------------------------------------------------------
        G2S_SetBGMosaicSize(mosaic_H, mosaic_V);
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
