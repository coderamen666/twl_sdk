/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_CharBg_3
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
// A sample that use BG #2 as affine mode (scaling)
//
// USAGE:
//   UP, DOWN, LEFT, RIGHT: Change the center position of scaling
//   A,B:                   Scaling BG #2
//
// HOWTO:
// 1. Set up the character/palette/screen data the same as Sub_CharBg_1
// 2. Set up a matrix and points with G2S_SetBGxAffine
//---------------------------------------------------------------------------

#include <nitro/code32.h>              // Avoid byte access problems
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

#define STANDARD_SIZE        (1 << 8)

static u8 sScrnBuf[SCREEN_SIZE];       // Buffer for screen data (BG #2)

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    s16     x_0 = 0, y_0 = 0;
    s16     x_1 = 0, y_1 = 0;

    fx32    scale = 1 << FX32_SHIFT;

    //---------------------------------------------------------------------------
    // Initialize:
    // Enables IRQ interrupts, initializes VRAM, and sets BG #2 for affine mode
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplaySubBG2Only();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GXS_LoadBG2Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GXS_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));

    {
        int     i, j;
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                sScrnBuf[(i * 16) + j] = (u8)((i * 0x10) + j);
            }
        }
    }
    // Store the data in main memory, and invalidate the cache
    DC_FlushRange(sScrnBuf, sizeof(sScrnBuf));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();

    // DMA Transfer to BG #2 screen
    GXS_LoadBG2Scr(sScrnBuf, 0, sizeof(sScrnBuf));

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        MtxFx22 mtx;
        fx32    rScale;

        DEMOReadKey();
        if (DEMO_IS_PRESS(PAD_KEY_UP))
            y_0 -= 2;
        if (DEMO_IS_PRESS(PAD_KEY_DOWN))
            y_0 += 2;
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
            x_0 += 2;
        if (DEMO_IS_PRESS(PAD_KEY_LEFT))
            x_0 -= 2;
        if (DEMO_IS_PRESS(PAD_BUTTON_A))
            scale -= 2 << (FX32_SHIFT - 8);
        if (DEMO_IS_PRESS(PAD_BUTTON_B))
            scale += 2 << (FX32_SHIFT - 8);
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
        {
            x_0 = 32;
            y_0 = 32;
            scale = 1 << FX32_SHIFT;
        }

        scale = MATH_CLAMP(scale, 0x100, 0x3000);

        rScale = FX_Inv(scale);
        mtx._00 = rScale;
        mtx._01 = 0;
        mtx._10 = 0;
        mtx._11 = rScale;

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt

        //---------------------------------------------------------------------------
        // Set up affine transformation for BG #2
        //---------------------------------------------------------------------------
        G2S_SetBG2Affine(&mtx,         // A matrix for rotation and scaling
                         x_0, y_0,     // The center of rotation
                         0, 0          // The reference point before rotation and scaling applied
            );
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
