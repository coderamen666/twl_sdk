/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_OneTri
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9663 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"

//---------------------------------------------------------------------------
// A sample for drawing a triangle:
//
// This sample simply draws a triangle.
// It does not even set camera.
// Also, it does not use DEMOInit(), which sets up 3D hardware.
//---------------------------------------------------------------------------
static void drawTriangle()
{
    G3_PolygonAttr(GX_LIGHTMASK_NONE,  // no lights
                   GX_POLYGONMODE_MODULATE,     // modulation mode
                   GX_CULL_NONE,       // cull none
                   0,                  // polygon ID(0 - 63)
                   31,                 // alpha(0 - 31)
                   0                   // OR of GXPolygonAttrMisc's value
        );

    G3_Begin(GX_BEGIN_TRIANGLES);
    {
        G3_Color(GX_RGB(31, 16, 16));
        G3_Vtx(0, FX16_ONE, 0);
        G3_Color(GX_RGB(16, 31, 16));
        G3_Vtx(-FX16_ONE, -FX16_ONE, 0);
        G3_Color(GX_RGB(16, 16, 31));
        G3_Vtx(FX16_ONE, -FX16_ONE, 0);
    }
    G3_End();
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    //---------------------------------------------------------------------------
    // Initialization:
    // They set up interrupt, VRAM banks only.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    // This sample set the basic 3D settings, DEMOInit() not called.

    //---------------------------------------------------------------------------
    // Initialize 3D hardware
    //---------------------------------------------------------------------------
    G3X_Init();

    //---------------------------------------------------------------------------
    // Initialize the matrix stacks and the matrix stack pointers
    //---------------------------------------------------------------------------
    G3X_InitMtxStack();

    //---------------------------------------------------------------------------
    // After the geometry engine or the rendering one is enabled,
    // you must issue SwapBuffers geometry command once.
    //---------------------------------------------------------------------------
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

    //---------------------------------------------------------------------------
    // Set up graphics mode.
    // GX_DISPMODE_GRAPHICS must be specified if you display 2D or 3D graphics.
    // GX_BG0_AS_3D must be specified if you use BG0 as 3D graphics.
    //---------------------------------------------------------------------------
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_0,    // BG mode is 0
                       GX_BG0_AS_3D);  // Set BG0 as 3D

    GX_DispOff();

    //---------------------------------------------------------------------------
    // Set BG0 visible
    //---------------------------------------------------------------------------
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    //---------------------------------------------------------------------------
    // Set priority of BG0 plane
    //---------------------------------------------------------------------------
    G2_SetBG0Priority(0);

    //---------------------------------------------------------------------------
    // Shading mode is toon(toon shading is not used in this sample)
    //---------------------------------------------------------------------------
    G3X_SetShading(GX_SHADING_TOON);

    //---------------------------------------------------------------------------
    // No alpha tests
    //---------------------------------------------------------------------------
    G3X_AlphaTest(FALSE, 0);

    //---------------------------------------------------------------------------
    // Enable alpha blending
    //---------------------------------------------------------------------------
    G3X_AlphaBlend(TRUE);

    //---------------------------------------------------------------------------
    // Enable anti aliasing
    //---------------------------------------------------------------------------
    G3X_AntiAlias(TRUE);

    //---------------------------------------------------------------------------
    // No edge marking
    //---------------------------------------------------------------------------
    G3X_EdgeMarking(FALSE);

    //---------------------------------------------------------------------------
    // No fog
    //---------------------------------------------------------------------------
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);

    //---------------------------------------------------------------------------
    // Set up clear color, depth, and polygon ID.
    //---------------------------------------------------------------------------
    G3X_SetClearColor(0, 31, 0x7fff, 63, FALSE);

    //---------------------------------------------------------------------------
    // Viewport
    //---------------------------------------------------------------------------
    G3_ViewPort(0, 0, 255, 191);

    //---------------------------------------------------------------------------
    // Start display
    //---------------------------------------------------------------------------
    OS_WaitVBlankIntr();               // Waiting the end of VBlank interrupt
    GX_DispOn();

    while (1)
    {
        //---------------------------------------------------------------------------
        // Reset the states of 3D graphics
        //---------------------------------------------------------------------------
        G3X_Reset();

        //---------------------------------------------------------------------------
        // In this sample, the camera matrix and the projection matrix are set as
        // an identity matrices for simplicity.
        //---------------------------------------------------------------------------
        G3_MtxMode(GX_MTXMODE_PROJECTION);
        G3_Identity();
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
        G3_Identity();

        drawTriangle();

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xEC77D136);
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
