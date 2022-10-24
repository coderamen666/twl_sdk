/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_Oam_5
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that mosaics an OBJ.
//
// USAGE:
//   UP, DOWN:    Control the size of mosaic in vertical direction
//   LEFT, RIGHT: Control the size of mosaic in horizontal direction
//
// HOWTO:
// 1. Set up the character/palette/attribute data the same as for Sub_Oam_1
// 2. Specify the size of mosaic with G2S_SetOBJMosaicSize
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

static GXOamAttr sOamBak[128];         // Buffer for OAM

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     mosaic_H = 0, mosaic_V = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and make the OBJ visible.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplaySubOBJOnly();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GXS_LoadOBJ(d_64_256_obj_schDT, 0, sizeof(d_64_256_obj_schDT));
    GXS_LoadOBJPltt(d_64_256_obj_sclDT, 0, sizeof(d_64_256_obj_sclDT));

    MI_DmaFill32(3, sOamBak, 192, sizeof(sOamBak));     // Clear OAM buffer

    G2_SetOBJAttr(&sOamBak[0],         // Pointer to the attributes
                  0,                   // X
                  0,                   // Y
                  0,                   // Priority
                  GX_OAM_MODE_NORMAL,  // OBJ mode
                  TRUE,                // Mosaic
                  GX_OAM_EFFECT_NONE,  // Flip/affine/no display/affine(double)
                  GX_OAM_SHAPE_64x64,  // Size and shape
                  GX_OAM_COLORMODE_256, // OBJ character data is in 256-color format
                  0,                   // Character name
                  0,                   // Color param
                  0                    // Affine param
        );

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
        // Specify the horizontal/vertical size of mosaic on the OBJs
        //---------------------------------------------------------------------------
        G2S_SetOBJMosaicSize(mosaic_H, mosaic_V);

        // Store the data in main memory and invalidate the cache
        DC_FlushRange(sOamBak, sizeof(sOamBak));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();

        GXS_LoadOAM(sOamBak, 0, sizeof(sOamBak));
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
