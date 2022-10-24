/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/Sub_Oam_3
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
// A sample that scales an OBJ.
//
// USAGE:
//   A: scaling down
//   B: scaling up
//
// HOWTO:
// 1. Set up the character/palette/attribute data the same as for Sub_Oam_1
// 2. Set up a matrix with G2_SetOBJAffine
// 3. Transfer the copy of object attribute memory with GXS_LoadOAM
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
    fx32    revScale = FX32_ONE;

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

    DEMOStartDisplay();
    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
        MtxFx22 mtx;

        DEMOReadKey();

        if (DEMO_IS_PRESS(PAD_BUTTON_A))        // Scaling down
            revScale += 1 << 7;
        if (DEMO_IS_PRESS(PAD_BUTTON_B))        // Scaling up
            revScale -= 1 << 7;
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
            revScale = FX32_ONE;
        if (revScale < 1)
            revScale = 1;

        G2_SetOBJAttr(&sOamBak[0],     // Pointer to the attributes
                      0,               // X
                      0,               // Y
                      0,               // Priority
                      GX_OAM_MODE_NORMAL,       // OBJ mode
                      FALSE,           // Mosaic
                      GX_OAM_EFFECT_AFFINE_DOUBLE,      // Flip/affine/no display/affine(double)
                      GX_OAM_SHAPE_64x64,       // Size and shape
                      GX_OAM_COLORMODE_256,     // OBJ character data is in 256-color format
                      0,               // Character name
                      0,               // Color param
                      0                // Affine param
            );

        mtx._00 = revScale;
        mtx._01 = 0;
        mtx._10 = 0;
        mtx._11 = revScale;

        G2_SetOBJAffine((GXOamAffine *)&sOamBak[0], &mtx);

        // Store the data in main memory and invalidate the cache
        DC_FlushRange(sOamBak, sizeof(sOamBak));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();

        OS_WaitVBlankIntr();           // Waiting for the end of V-Blank interrupt

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
