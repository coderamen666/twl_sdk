/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Edge_Marking
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
// A sample for edge marking:
//
// Display three rotating cubes with edge marking in three colors.
//
// HOWTO:
// 1. Enable edge marking by G3X_EdgeMarking(TRUE).
// 2. Set up the edge color table by G3X_SetEdgeColorTable(table).
//    The color table[n] is used for the polygons with polygonID = 8*n to 8*n + 7.
// 3. Render polygons.
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

VecFx10 gCubeNormal[8] = {
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3)
};

inline void vtx(int idx)
{
    G3_Vtx(gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

inline void normal(int idx)
{
    G3_Direct1(G3OP_NORMAL, gCubeNormal[idx]);
    // use G3_Normal(x, y, z) if not packed
}


static void NrmVtxQuad(int idx0, int idx1, int idx2, int idx3)
{
    normal(idx0);
    vtx(idx0);
    normal(idx1);
    vtx(idx1);
    normal(idx2);
    vtx(idx2);
    normal(idx3);
    vtx(idx3);
}


GXRgb   gEdgeColor[8] = {
    GX_RGB(31, 0, 0),
    GX_RGB(0, 31, 0),
    GX_RGB(0, 0, 31),
    GX_RGB(0, 0, 0),
    GX_RGB(0, 0, 0),
    GX_RGB(0, 0, 0),
    GX_RGB(0, 0, 0),
    GX_RGB(0, 0, 0)
};

static void drawCube(int polygonID, u16 Rotate)
{
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    // Set the Material As basic whilte 
    DEMO_Set3DDefaultMaterial(TRUE, TRUE);
    DEMO_Set3DDefaultShininessTable();

    G3_PolygonAttr(GX_LIGHTMASK_0,     // Light #0 is on
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_BACK,       // cull back
                   polygonID,          // polygon ID(0 - 63)
                   31,                 // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );


    G3_Begin(GX_BEGIN_QUADS);
    {
        NrmVtxQuad(2, 0, 4, 6);
        NrmVtxQuad(7, 5, 1, 3);
        NrmVtxQuad(6, 4, 5, 7);
        NrmVtxQuad(3, 1, 0, 2);
        NrmVtxQuad(5, 4, 0, 1);
        NrmVtxQuad(6, 7, 3, 2);
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

    G3X_EdgeMarking(TRUE);             // Enable edge marking



    //---------------------------------------------------------------------------
    // Set up the edge color table:
    //
    // Use different edge colors from each other
    //---------------------------------------------------------------------------
    G3X_SetEdgeColorTable(&gEdgeColor[0]);

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye Position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

        //---------------------------------------------------------------------------
        // Set up light colors and direction.
        // Notice that light vector is transformed by the current vector matrix
        // immediately after LightVector command is issued.
        //
        // GX_LIGHTID_0: white, downward
        //---------------------------------------------------------------------------
        G3_LightVector(GX_LIGHTID_0, FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        G3_PushMtx();
        G3_Translate(0, 0, -5 * FX32_ONE);

        //---------------------------------------------------------------------------
        // Draw cube #1 with polygonID= 0
        //---------------------------------------------------------------------------
        G3_PushMtx();
        G3_Translate(-1 * (FX32_ONE >> 1), -1 * (FX32_ONE >> 1), 0);
        drawCube(0, Rotate);
        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // Draw cube #2 with polygonID= 8
        //---------------------------------------------------------------------------
        G3_PushMtx();
        G3_Translate(1 * (FX32_ONE >> 1), -1 * (FX32_ONE >> 1), 0);
        drawCube(8, Rotate);
        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // Draw cube #3 with polygonID= 16
        //---------------------------------------------------------------------------
        G3_PushMtx();
        G3_Translate(0, 1 * (FX32_ONE >> 1), -2 * FX32_ONE);
        drawCube(16, Rotate);
        G3_PopMtx(1);

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x408C5C88);
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
