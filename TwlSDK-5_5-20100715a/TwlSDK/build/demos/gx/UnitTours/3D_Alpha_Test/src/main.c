/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Alpha_Test
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
// A sample for alpha testing:
// Two cubes are send to command FIFO.
// The alpha parameters are 31(opaque) and 16(translucent) respectively.
// Since the threshold of alpha test is set to 24 by G3X_AlphaTest(TRUE, 24),
// The only one cube is displayed.
//
// HOWTO:
// 1. Call G3X_AlphaTest(TRUE, alpha), which enables alpha tests and specifies
//    the threshold.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"

s16     gCubeGeometry[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, FX16_ONE, -FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, -FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, -FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, -FX16_ONE
};


static void vtx(int idx)
{
    G3_Vtx(gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

static void quad(int idx0, int idx1, int idx2, int idx3)
{
    vtx(idx0);
    vtx(idx1);
    vtx(idx2);
    vtx(idx3);
}


static void drawCube(int alpha, u16 Rotate)
{
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }
    G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // diffuse
                            GX_RGB(16, 16, 16), // ambient
                            TRUE       // use diffuse as vtx color if TRUE
        );

    G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // specular
                            GX_RGB(0, 0, 0),    // emission
                            FALSE      // use shininess table if TRUE
        );

    G3_PolygonAttr(GX_LIGHTMASK_NONE,  // no lights
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_BACK,       // cull back
                   0,                  // polygon ID(0 - 63)
                   alpha,              // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );


    G3_Begin(GX_BEGIN_QUADS);
    {
        quad(2, 0, 4, 6);
        quad(7, 5, 1, 3);
        quad(6, 4, 5, 7);
        quad(3, 1, 0, 2);
        quad(5, 4, 0, 1);
        quad(6, 7, 3, 2);
    }
    G3_End();
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     Rotate = 0;                // for rotating cubes(0-65535)

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    // not displayed if alpha is equal or less than 24
    G3X_AlphaTest(TRUE, 24);

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up the camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye Position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

        G3_PushMtx();
        G3_Translate(0, 0, -5 * FX32_ONE);

        //---------------------------------------------------------------------------
        // Send Cube #1(visible)
        //---------------------------------------------------------------------------
        G3_PushMtx();
        G3_Translate(-3 * (FX32_ONE >> 1), 0, 0);
        drawCube(31, Rotate);
        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // Send Cube #2(not visible)
        //---------------------------------------------------------------------------
        G3_PushMtx();
        G3_Translate(3 * (FX32_ONE >> 1), 0, 0);
        drawCube(24, Rotate);
        G3_PopMtx(1);

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x7DBE54EC);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt
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
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}
