/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Light
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
// A sample for lighting:
//
// Draw two cubes with a light changing in colors and direction.
// The left one has a normal vector for each plane,
// and the right one has a normal vector for each vertex.
//
// HOWTO:
// 1. Specify the direction of light by G3_LightVector.
// 2. Specify the color of light by G3_LightColor.
// 3. Specify the light to be enabled by G3_PolygonAttr.
//
// Note that there are four lights available.
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

VecFx10 gCubeNormal1[8] = {
    GX_VECFX10(0, 0, FX32_ONE - 1),
    GX_VECFX10(0, FX32_ONE - 1, 0),
    GX_VECFX10(FX32_ONE - 1, 0, 0),
    GX_VECFX10(0, 0, -FX32_ONE + 1),
    GX_VECFX10(0, -FX32_ONE + 1, 0),
    GX_VECFX10(-FX32_ONE + 1, 0, 0)
};

VecFx10 gCubeNormal2[8] = {
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3)
};

s16    *gVertexArray = NULL;
VecFx10 *gNormalArray = NULL;

inline void setVtxArray(s16 *array)
{
    gVertexArray = array;
}

inline void setNormalArray(VecFx10 *array)
{
    gNormalArray = array;
}

inline void vtx(int idx)
{
    G3_Vtx(gVertexArray[idx * 3], gVertexArray[idx * 3 + 1], gVertexArray[idx * 3 + 2]);
}

inline void normal(int idx)
{
    G3_Direct1(G3OP_NORMAL, gNormalArray[idx]);
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

static void NrmQuad(int nrm, int idx0, int idx1, int idx2, int idx3)
{
    normal(nrm);
    vtx(idx0);
    vtx(idx1);
    vtx(idx2);
    vtx(idx3);
}

static void drawLeftCube(u16 Rotate, u8 count)
{
    G3_PushMtx();

    // Rotate and translate the cube
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_Translate(-3 << (FX32_SHIFT - 1), 0, 0);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    // Set the material color( diffuse, ambient , specular ) as basic white 
    DEMO_Set3DDefaultMaterial(FALSE, TRUE);
    DEMO_Set3DDefaultShininessTable();

    G3_PolygonAttr((GXLightMask)(count >> 4),   // lighting is varying
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_BACK,       // cull back
                   0,                  // polygon ID(0 - 63)
                   31,                 // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );

    setVtxArray(gCubeGeometry);
    setNormalArray(gCubeNormal1);

    G3_Begin(GX_BEGIN_QUADS);

    {
        NrmQuad(0, 2, 0, 4, 6);
        NrmQuad(3, 7, 5, 1, 3);
        NrmQuad(5, 6, 4, 5, 7);
        NrmQuad(2, 3, 1, 0, 2);
        NrmQuad(1, 5, 4, 0, 1);
        NrmQuad(4, 6, 7, 3, 2);
    }

    G3_End();

    G3_PopMtx(1);
}

static void drawRightCube(u16 Rotate, u8 count)
{
    G3_PushMtx();

    // Rotate and translate the cube
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_Translate(3 << (FX32_SHIFT - 1), 0, 0);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    // Set the material color( diffuse, ambient , specular ) as basic white 
    DEMO_Set3DDefaultMaterial(FALSE, TRUE);
    DEMO_Set3DDefaultShininessTable();

    G3_PolygonAttr((GXLightMask)(count >> 4),   // lighting is varying
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_BACK,       // cull back
                   0,                  // polygon ID(0 - 63)
                   31,                 // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );

    setVtxArray(gCubeGeometry);
    setNormalArray(gCubeNormal2);

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

    G3_PopMtx(1);
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

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        count++;
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 5 };       // Eye position
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
        // GX_LIGHTID_1: red, leftward
        // GX_LIGHTID_2: green, upward
        // GX_LIGHTID_3: blue, rightward
        //---------------------------------------------------------------------------
        G3_LightVector(GX_LIGHTID_0, 0, -FX16_ONE + 1, 0);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        G3_LightVector(GX_LIGHTID_1, -FX16_ONE + 1, 0, 0);
        G3_LightColor(GX_LIGHTID_1, GX_RGB(31, 0, 0));

        G3_LightVector(GX_LIGHTID_2, 0, FX16_ONE - 1, 0);
        G3_LightColor(GX_LIGHTID_2, GX_RGB(0, 31, 0));

        G3_LightVector(GX_LIGHTID_3, FX16_ONE - 1, 0, 0);
        G3_LightColor(GX_LIGHTID_3, GX_RGB(0, 0, 31));

        G3_PushMtx();

        drawLeftCube(Rotate, count);
        drawRightCube(Rotate, count);

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xEF41160C);
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
