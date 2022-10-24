/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Fog
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample for fog blending
//
// A cube is rotating on an axis(a quad).
// It is fogged when it gets far from the eye.
//
// HOWTO:
// 1. Enable and set up attributes by G3X_SetFog.
// 2. Specify the color of fog by G3X_SetFogColor.
// 3. Set up the fog table by G3X_SetFogTable.
// 4. GX_POLYGON_ATTR_MISC_FOG must be specified when G3_PolygonAttr is called.
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

    //---------------------------------------------------------------------------
    // Set up fog
    //---------------------------------------------------------------------------
    {
        u32     fog_table[8];
        int     i;

        //---------------------------------------------------------------------------
        // Enable fog and set up attributes
        //---------------------------------------------------------------------------
        G3X_SetFog(TRUE,               // Fog enabled if TRUE
                   GX_FOGBLEND_COLOR_ALPHA,     // Fog mode(blending to pixel's color and alpha)
                   GX_FOGSLOPE_0x2000, // Fog slope(the small number makes fog steep)
                   0x5800              // Fog offset(use table[0] until depth=0x5800)
            );

        //---------------------------------------------------------------------------
        // Set up fog color
        //---------------------------------------------------------------------------
        G3X_SetFogColor(GX_RGB(0, 0, 0), 0);

        //---------------------------------------------------------------------------
        // Set up the fog table:
        // The value must be from 0 to 127.
        // The bigger number means a denser fog.
        //---------------------------------------------------------------------------
        for (i = 0; i < 8; i++)
        {
            fog_table[i] =
                (u32)(((i * 16) << 0) | ((i * 16 + 4) << 8) | ((i * 16 + 8) << 16) | ((i * 16 +
                                                                                       12) << 24));
        }
        G3X_SetFogTable(&fog_table[0]);
    }

    //---------------------------------------------------------------------------
    // Use Z-Buffering mode:
    // This sample uses Z-Buffering mode.
    // Notice that the depth values are different between Z-Buffering mode and W-Buffering mode.
    //---------------------------------------------------------------------------
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 8 };       // Eye Position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

        G3_PushMtx();

        //---------------------------------------------------------------------------
        // Rotate on the Y axis
        //---------------------------------------------------------------------------
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotY(s, c);
            G3_Translate(0, 0, 5 * FX32_ONE);
            G3_RotX(s, c);
            G3_RotZ(s, c);
        }

        G3_MaterialColorDiffAmb(GX_RGB(31, 0, 0),       // diffuse
                                GX_RGB(16, 16, 16),     // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_PolygonAttr(GX_LIGHTMASK_NONE,       // disable lights
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_BACK,   // cull back
                       0,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       GX_POLYGON_ATTR_MISC_FOG //!!! you must specify GX_POLYGON_ATTR_MISC_FOG.
            );

        //---------------------------------------------------------------------------
        // Draw a cube with fog blending
        //---------------------------------------------------------------------------
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

        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // A quad without fog blending
        //---------------------------------------------------------------------------
        G3_PushMtx();

        G3_MaterialColorDiffAmb(GX_RGB(0, 0, 31),       // diffuse
                                GX_RGB(16, 16, 16),     // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_PolygonAttr(GX_LIGHTMASK_NONE,       // disable lights
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_BACK,   // cull back
                       0,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        //---------------------------------------------------------------------------
        // Draw a quad without fog blending
        //---------------------------------------------------------------------------
        G3_Begin(GX_BEGIN_QUADS);

        G3_Vtx(FX32_ONE >> 2, -FX32_ONE << 2, 0);
        G3_Vtx(FX32_ONE >> 2, FX32_ONE << 2, 0);
        G3_Vtx(-FX32_ONE >> 2, FX32_ONE << 2, 0);
        G3_Vtx(-FX32_ONE >> 2, -FX32_ONE << 2, 0);

        G3_End();

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xA39F3A2E);
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
