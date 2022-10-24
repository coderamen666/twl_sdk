/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Shininess
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that uses the shininess table:
//
// There are nine spheres displayed. Each of them displayed with different shininess adjustment
// by the shininess table.
// 
// HOWTO:
// 1. Set up the shininess table by G3_Shininess().
// 2. Prepare to use the shininess table by G3_MaterialColorSpecEmi(..., TRUE).
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "commandList.h"

#define NUM_SHININESS_TYPES     9
static u32 shininess_table[NUM_SHININESS_TYPES][32];


static void drawSphere(BOOL shininess)
{
    //---------------------------------------------------------------------------
    // GX_RGB(0, 0, 0) set except specular color for comparison
    //---------------------------------------------------------------------------
    G3_MaterialColorDiffAmb(GX_RGB(23, 0, 0),   // diffuse
                            GX_RGB(8, 0, 0),    // ambient
                            FALSE      // use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(GX_RGB(31, 31, 31), // specular
                            GX_RGB(0, 0, 0),    // emission
                            shininess  // use shininess table if TRUE
        );


    G3_PolygonAttr(GX_LIGHTMASK_0,     // lighting
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_BACK,       // cull back
                   0,                  // polygon ID(0 - 63)
                   31,                 // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );

    // Transfer by DMA
    G3_Begin(GX_BEGIN_TRIANGLES);
    MI_SendGXCommand(3, sphere1, sphere_size);
    G3_End();

}

// Power fixed point format( 0.8 ) number 
static inline u8 powFixedPoint8_(u8 base, u8 exponent)
{
    u16     result = (u16)base;
    const u16 LIGHTING_L_DOT_H_SHIFT = 8;

    if (exponent == 0)
        return base;

    while (exponent - 1 > 0)
    {
        result *= (u16)base;
        result = (u16)(result >> LIGHTING_L_DOT_H_SHIFT);
        exponent--;
    }

    return (u8)result;
}

#include <nitro/code32.h>              // avoid byte access problems
// Initialize the shininess table.
static void InitShininessTables_()
{
    u8      i, j;
    u8     *pShininess = NULL;
    const u8 tableLength = 32 * sizeof(u32);

    for (i = 0; i < NUM_SHININESS_TYPES; i++)
    {
        pShininess = (u8 *)&shininess_table[i][0];

        for (j = 0; j < tableLength - 1; j++)
        {
            pShininess[j] = powFixedPoint8_((u8)(j * 2 + 1), i);
        }

        // set the max value specially 
        pShininess[tableLength - 1] = 0xFF;
    }
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u8      count = 0;
    u16     Rotate = 0;                // for rotating cubes(0-65535)

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    // Initialize shininess tables 
    InitShininessTables_();

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        count++;
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up the camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

        //---------------------------------------------------------------------------
        // Set up light colors and direction.
        // Notice that light vector is transformed by the current vector matrix
        // immediately after LightVector command is issued.
        //---------------------------------------------------------------------------
        {
            const fx32 XY_R = FX_Mul(FX32_SQRT1_3, FX32_SQRT2);

            G3_LightVector(GX_LIGHTID_0,
                           (fx16)FX_Mul(XY_R, FX_SinIdx(Rotate)),
                           (fx16)FX_Mul(XY_R, FX_CosIdx(Rotate)), (fx16)(FX32_SQRT1_3 * -1));

            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
        }

        G3_PushMtx();

        //---------------------------------------------------------------------------
        // Draw spheres , changing its shininess table.
        //---------------------------------------------------------------------------
        G3_Translate(0, 0, -15 * FX32_ONE);
        {
            u8      i = 0;
            const fx32 step = FX32_ONE * 5;
            const u8 num_row = 3;
            const u8 num_cor = NUM_SHININESS_TYPES / num_row;
            for (i = 0; i < NUM_SHININESS_TYPES; i++)
            {
                int     ix = i % num_row;
                int     iy = i / num_row;
                //---------------------------------------------------------------------------
                // Set up the shininess table
                //---------------------------------------------------------------------------
                G3_Shininess(&shininess_table[i][0]);


                G3_PushMtx();
                G3_Translate(step * ix - step * (num_row / 2), -step * iy + step * (num_cor / 2),
                             0);
                drawSphere(TRUE);
                G3_PopMtx(1);
            }
        }

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x445FBD00);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt
    }
}

#include <nitro/codereset.h>

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
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}
