/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOInit.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMOInit.h"
#include "DEMOKey.h"

void DEMOInitCommon(void)
{
    OS_Init();
    FX_Init();

//    GX_SetPower(GX_POWER_ALL);
    GX_Init();

    GX_DispOff();
    GXS_DispOff();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    DEMOReadKey();                     // Dummy read to save previous key input

    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
}

void DEMOInitVRAM(void)
{
    //---------------------------------------------------------------------------
    // All VRAM banks to LCDC
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    //---------------------------------------------------------------------------
    // Clear all LCDC space
    //---------------------------------------------------------------------------
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    //---------------------------------------------------------------------------
    // Disable the banks on LCDC
    //---------------------------------------------------------------------------
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);   // Clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);     // Clear the standard palette

    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);     // Clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);       // Clear the standard palette
}


void DEMOInitDisplayBG0Only(void)
{

    GX_SetBankForBG(GX_VRAM_BG_128_A); // VRAM-A for BGs

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_0,    // BGMODE is 0
                       GX_BG0_AS_2D);  // BG #0 is for 2D

    GX_SetVisiblePlane(GX_PLANEMASK_BG0);       // Display only BG #0

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,        // 256pix x 256pix text
                     GX_BG_COLORMODE_256,       // use 256 color mode
                     GX_BG_SCRBASE_0x0000,      // screen base offset + 0x0000 is the address for BG #0 screen
                     GX_BG_CHARBASE_0x04000,    // character base offset + 0x04000 is the address for BG #0 characters
                     GX_BG_EXTPLTT_01  // use BGExtPltt slot #0 if BGExtPltt is enabled
        );

    G2_SetBG0Priority(0);

    G2_BG0Mosaic(FALSE);
}

void DEMOInitDisplaySubBG0Only(void)
{

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);   // VRAM-C for BGs

    GXS_SetGraphicsMode(GX_BGMODE_0);  // BGMODE is 0

    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);      // Display only BG #0

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,       // 256pix x 256pix text
                      GX_BG_COLORMODE_256,      // use 256 color mode
                      GX_BG_SCRBASE_0x0000,     // screen base offset + 0x0000 is the address for BG #0 screen
                      GX_BG_CHARBASE_0x04000,   // character base offset + 0x04000 is the address for BG #0 characters
                      GX_BG_EXTPLTT_01 // use BGExtPltt slot #0 if BGExtPltt is enabled
        );

    G2S_SetBG0Priority(0);

    G2S_BG0Mosaic(FALSE);
}



void DEMOInitDisplayBG2Only(void)
{
    GX_SetBankForBG(GX_VRAM_BG_128_A); // VRAM-A for BGs

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_2,    // BGMODE is 2
                       GX_BG0_AS_2D    // BG #0 is for 2D
        );

    GX_SetVisiblePlane(GX_PLANEMASK_BG2);       // Display only BG #2

    G2_SetBG2ControlAffine(GX_BG_SCRSIZE_AFFINE_128x128,        // 128px x 128px affine
                           GX_BG_AREAOVER_XLU,  // no repeat(transparent)
                           GX_BG_SCRBASE_0x0000,        // screen base offset + 0x0000 is the address for BG #0 screen
                           GX_BG_CHARBASE_0x04000       // character base offset + 0x04000 is the address for BG #0 characters
        );

    G2_SetBG2Priority(0);

    G2_BG2Mosaic(FALSE);
}


void DEMOInitDisplaySubBG2Only(void)
{
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);   // VRAM-C for BGs

    GXS_SetGraphicsMode(GX_BGMODE_2);  // BGMODE is 2

    GXS_SetVisiblePlane(GX_PLANEMASK_BG2);      // Display only BG #2

    G2S_SetBG2ControlAffine(GX_BG_SCRSIZE_AFFINE_128x128,       // 128px x 128px affine
                            GX_BG_AREAOVER_XLU, // no repeat(transparent)
                            GX_BG_SCRBASE_0x0000,       // screen base offset + 0x0000 is the address for BG #0 screen
                            GX_BG_CHARBASE_0x04000      // character base offset + 0x04000 is the address for BG #0 characters
        );

    G2S_SetBG2Priority(0);

    G2S_BG2Mosaic(FALSE);
}



void DEMOInitDisplayOBJOnly(void)
{
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);        // VRAM-A for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_0,    // BGMODE is 0
                       GX_BG0_AS_2D);  // BG #0 is for 2D

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Display only OBJ

    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);      // 2D mapping mode
}


void DEMOInitDisplaySubOBJOnly(void)
{
    GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D); // VRAM-D for OBJ

    GXS_SetGraphicsMode(GX_BGMODE_0);  // BGMODE is 0

    GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);      // Display only OBJ

    GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);     // 2D mapping mode
}

void DEMOInitDisplay3D(void)
{
    G3X_Init();                        // initialize the 3D graphics states
//    G3X_InitTable();                                       // initialize tables

    G3X_InitMtxStack();                // initialize the matrix stack

    GX_SetBankForTex(GX_VRAM_TEX_0_A); // VRAM-A for texture images

    GX_SetBankForBG(GX_VRAM_BG_128_D); // VRAM-D for BGs

    GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0123_E);       // VRAM-E for texture palettes

    GX_SetBankForOBJ(GX_VRAM_OBJ_32_FG);        // VRAM-FG for OBJ

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_4,    // BGMODE is 4
                       GX_BG0_AS_3D);  // BG #0 is for 3D

    GX_SetVisiblePlane(GX_PLANEMASK_BG0 |       // BG #0 for 3D
                       GX_PLANEMASK_BG1 |       // BG #1 for text
                       GX_PLANEMASK_OBJ // OBJ
        );

    G2_SetBG0Priority(1);

    G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256,        // 256pix x 256pix text
                     GX_BG_COLORMODE_16,        // use 16 color mode
                     GX_BG_SCRBASE_0x3800,      // screen base offset + 0x3800 is the address for BG #1 screen
                     GX_BG_CHARBASE_0x00000,    // character base offset + 0x00000 is the address for BG #1 characters
                     GX_BG_EXTPLTT_01  // use BGExtPltt slot #1 if BGExtPltt is enabled
        );

    G2_SetBG1Priority(0);

    G2_BG1Mosaic(FALSE);

    G3X_SetShading(GX_SHADING_TOON);   // shading mode is toon

    G3X_AntiAlias(TRUE);               // enable antialiasing(without additional computing costs)

    G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ,
                     GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
                     16,
                     0
        );

    //---------------------------------------------------------------------------
    // After you start the geometry engine and the rendering one
    // (by GX_SetPower etc.), G3_SwapBuffers must be called once before use
    //---------------------------------------------------------------------------
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

    //---------------------------------------------------------------------------
    // Set clear color, depth and polygon ID.
    //---------------------------------------------------------------------------
    G3X_SetClearColor(GX_RGB(0, 0, 0), // Clear color
                      31,              // Clear alpha
                      0x7fff,          // Clear depth
                      63,              // Clear polygon ID
                      FALSE            // Fog
        );

    G3_ViewPort(0, 0, 255, 191);       // Viewport

    //---------------------------------------------------------------------------
    // Set up the projection matrix
    //---------------------------------------------------------------------------
    {
        fx32    right = FX32_ONE;
        fx32    top = FX32_ONE * 3 / 4;
        fx32    near = FX32_ONE;
        fx32    far = FX32_ONE * 400;

        //---------------------------------------------------------------------------
        // Switch MTXMODE to GX_MTXMODE_PROJECTION, and
        // set a projection matrix on the current projection matrix on the matrix stack
        //---------------------------------------------------------------------------
        G3_Perspective(FX32_SIN30, FX32_COS30,  // Sine and cosine of FOVY
                       FX32_ONE * 4 / 3,        // Aspect
                       near,           // Near
                       far,            // Far
                       NULL            // A pointer to a matrix if you use one
            );

        G3_StoreMtx(0);
    }
}

void DEMOInit(void)
{
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();
}

void DEMOStartDisplay(void)
{
    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt
    GX_DispOn();
    GXS_DispOn();
}

#ifdef SDK_TWL
// Check whether system is running in TWL mode
void DEMOCheckRunOnTWL(void)
{
    if( !OS_IsRunOnTwl() )
    {
        OS_TPanic("demo is executable only on TWL"); // Demo is executable only on TWL
    }
}
#endif
