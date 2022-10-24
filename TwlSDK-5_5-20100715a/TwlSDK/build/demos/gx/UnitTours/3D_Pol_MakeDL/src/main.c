/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_MakeDL
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that makes display lists dynamically:
//
// HOWTO:
// 1. Call G3_BeginMakeDL to initialize GXDLInfo
// 2. Generate a display list by G3B/G3C_xxxxxx
// 3. Call G3_EndMakeDL to finish making the display list
// 4. Don't forget to flush the cache for the display list
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

GXRgb   gCubeColor[8] = {
    GX_RGB(31, 31, 31),
    GX_RGB(31, 31, 0),
    GX_RGB(31, 0, 31),
    GX_RGB(31, 0, 0),
    GX_RGB(0, 31, 31),
    GX_RGB(0, 31, 0),
    GX_RGB(0, 0, 31),
    GX_RGB(0, 0, 0)
};

#define LENGTH_DL (1024)
GXDLInfo gGXDLInfoB;
GXDLInfo gGXDLInfoC;
u32     gDL_B[LENGTH_DL];
u32     gSzDL_B;
u32     gDL_C[LENGTH_DL];
u32     gSzDL_C;

static void VtxB(GXDLInfo *info, int idx)
{
    G3B_Vtx(info, gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

static void VtxC(GXDLInfo *info, int idx)
{
    G3C_Vtx(info, gCubeGeometry[idx * 3], gCubeGeometry[idx * 3 + 1], gCubeGeometry[idx * 3 + 2]);
}

static void ColorB(GXDLInfo *info, int idx)
{
    G3B_Color(info, gCubeColor[idx]);
}

static void ColorC(GXDLInfo *info, int idx)
{
    G3C_Color(info, gCubeColor[idx]);
}

static void QuadB(GXDLInfo *info, int idx0, int idx1, int idx2, int idx3)
{
    VtxB(info, idx0);
    VtxB(info, idx1);
    VtxB(info, idx2);
    VtxB(info, idx3);
}

static void ColVtxQuadC(GXDLInfo *info, int idx0, int idx1, int idx2, int idx3)
{
    ColorC(info, idx0);
    VtxC(info, idx0);
    ColorC(info, idx1);
    VtxC(info, idx1);
    ColorC(info, idx2);
    VtxC(info, idx2);
    ColorC(info, idx3);
    VtxC(info, idx3);
}

//---------------------------------------------------------------------------
// Make a display list with G3B_xxxxxx, which makes a display list
// without packing commands
//---------------------------------------------------------------------------
static void makeLeftCube(void)
{
    //---------------------------------------------------------------------------
    // Call G3_BeginMakeDL before making a display list
    // This sets up GXDLInfo
    //---------------------------------------------------------------------------
    G3_BeginMakeDL(&gGXDLInfoB,        // GXDLInfo
                   gDL_B,              // buffer
                   LENGTH_DL * sizeof(u32)      // Buffer length (in bytes)
        );
    {
        G3B_MaterialColorDiffAmb(&gGXDLInfoB, GX_RGB(31, 31, 31),       // diffuse
                                 GX_RGB(16, 16, 16),    // ambient
                                 FALSE // use diffuse as vtx color if TRUE
            );

        G3B_MaterialColorSpecEmi(&gGXDLInfoB, GX_RGB(16, 16, 16),       // specular
                                 GX_RGB(0, 0, 0),       // emission
                                 FALSE // use shininess table if TRUE
            );

        G3B_PolygonAttr(&gGXDLInfoB, GX_LIGHTMASK_NONE, // disable lights
                        GX_POLYGONMODE_MODULATE,        // modulation mode
                        GX_CULL_BACK,  // cull back
                        0,             // polygon ID(0 - 63)
                        31,            // alpha(0 - 31)
                        0              // OR of GXPolygonAttrMisc's value
            );

        //---------------------------------------------------------------------------
        // Draw a cube:
        // Specify different colors for the planes
        //---------------------------------------------------------------------------

        G3B_Begin(&gGXDLInfoB, GX_BEGIN_QUADS);

        {
            ColorB(&gGXDLInfoB, 3);
            QuadB(&gGXDLInfoB, 2, 0, 4, 6);

            ColorB(&gGXDLInfoB, 4);
            QuadB(&gGXDLInfoB, 7, 5, 1, 3);

            ColorB(&gGXDLInfoB, 5);
            QuadB(&gGXDLInfoB, 6, 4, 5, 7);

            ColorB(&gGXDLInfoB, 2);
            QuadB(&gGXDLInfoB, 3, 1, 0, 2);

            ColorB(&gGXDLInfoB, 6);
            QuadB(&gGXDLInfoB, 5, 4, 0, 1);

            ColorB(&gGXDLInfoB, 1);
            QuadB(&gGXDLInfoB, 6, 7, 3, 2);
        }

        G3B_End(&gGXDLInfoB);
    }

    //---------------------------------------------------------------------------
    // Call G3_EndMakeDL after making a display list
    // This returns the size of the display list in bytes
    //---------------------------------------------------------------------------
    gSzDL_B = G3_EndMakeDL(&gGXDLInfoB);

    // Don't forget to flush the cache
    DC_FlushRange(G3_GetDLStart(&gGXDLInfoB), gSzDL_B);
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
}

//---------------------------------------------------------------------------
// Make a display list with G3C_xxxxxx, which makes a display list
// with packing commands
//---------------------------------------------------------------------------
static void makeRightCube(void)
{
    G3_BeginMakeDL(&gGXDLInfoC,        // GXDLInfo
                   gDL_C,              // buffer
                   LENGTH_DL * sizeof(u32)      // Buffer length (in bytes)
        );
    {

        G3C_MaterialColorDiffAmb(&gGXDLInfoC, GX_RGB(31, 31, 31),       // diffuse
                                 GX_RGB(16, 16, 16),    // ambient
                                 FALSE // use diffuse as vtx color if TRUE
            );

        G3C_MaterialColorSpecEmi(&gGXDLInfoC, GX_RGB(16, 16, 16),       // specular
                                 GX_RGB(0, 0, 0),       // emission
                                 FALSE // use shininess table if TRUE
            );

        G3C_PolygonAttr(&gGXDLInfoC, GX_LIGHTMASK_NONE, // disable lights
                        GX_POLYGONMODE_MODULATE,        // modulation mode
                        GX_CULL_BACK,  // cull back
                        0,             // polygon ID(0 - 63)
                        31,            // alpha(0 - 31)
                        0              // OR of GXPolygonAttrMisc's value
            );

        //---------------------------------------------------------------------------
        // Draw a cube:
        // Specify different colors for the vertices.
        //---------------------------------------------------------------------------
        G3C_Begin(&gGXDLInfoC, GX_BEGIN_QUADS);
        {
            ColVtxQuadC(&gGXDLInfoC, 2, 0, 4, 6);
            ColVtxQuadC(&gGXDLInfoC, 7, 5, 1, 3);
            ColVtxQuadC(&gGXDLInfoC, 6, 4, 5, 7);
            ColVtxQuadC(&gGXDLInfoC, 3, 1, 0, 2);
            ColVtxQuadC(&gGXDLInfoC, 5, 4, 0, 1);
            ColVtxQuadC(&gGXDLInfoC, 6, 7, 3, 2);
        }
        G3C_End(&gGXDLInfoC);
    }

    //---------------------------------------------------------------------------
    // Call G3_EndMakeDL after making a display list
    // This returns the size of the display list in bytes
    //---------------------------------------------------------------------------
    gSzDL_C = G3_EndMakeDL(&gGXDLInfoC);

    // Don't forget to flush the cache
    DC_FlushRange(G3_GetDLStart(&gGXDLInfoC), gSzDL_C);
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
}

static void drawLeftCube(u16 Rotate)
{
    G3_PushMtx();

    // Rotate and translate
    G3_Translate(-3 << (FX32_SHIFT - 1), 0, 0);
    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    // Sending a display list
    MI_SendGXCommand(3, gDL_B, gSzDL_B);

    G3_PopMtx(1);
}

static void drawRightCube(u16 Rotate)
{
    G3_PushMtx();

    // Rotate and translate
    G3_Translate(3 << (FX32_SHIFT - 1), 0, 0);

    {
        fx16    s = FX_SinIdx(Rotate);
        fx16    c = FX_CosIdx(Rotate);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);
    }

    // Sending a display list
    MI_SendGXCommand(3, gDL_C, gSzDL_C);

    G3_PopMtx(1);
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     Rotate = 0;                // For rotating cubes (0-65535)

    //---------------------------------------------------------------------------
    // Initialize:
    // Enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    makeLeftCube();
    makeRightCube();

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up a camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 5 };       // Eye Position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);
        }

        G3_PushMtx();

        drawLeftCube(Rotate);
        drawRightCube(Rotate);

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xA51F0EEF);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
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
