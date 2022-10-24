/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_TexSRT
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample for texture SRT animation:
//
// USAGE:
//   Up, Down, Left, Right: move the texture
//   A, B: change the scale of the texture
//   L, R: rotate the texture
//
// HOWTO:
// 1. Set up the texture matrix.
// 2. Specify GX_TEXGEN_TEXCOORD as a parameter of G3_TexImageParam.
//
// Notice that the translation must be multiplied by 16 in advance,
// or use G3_LoadTexMtxTexCoord() instead.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "tex_4plett.h"

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u32     myTexAddr = 0x1000;        // texture image address in texture image slots
    u32     myTexPlttAddr = 0x1000;    // texture palette address in texture palette slots

    fx32    texScale = FX32_ONE >> 1;
    fx32    texX = 0;
    fx32    texY = 0;
    u16     texRotate = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();


    //---------------------------------------------------------------------------
    // Download the texture images:
    //
    // Transfer the texture data on the main memory to the texture image slots.
    //---------------------------------------------------------------------------
    GX_BeginLoadTex();                 // map the texture image slots onto LCDC address space
    {
        GX_LoadTex((void *)&tex_4plett64x64[0], // a pointer to the texture data on the main memory(4 bytes aligned)
                   myTexAddr,          // an offset address in the texture image slots
                   1024                // the size of the texture(s)(in bytes)
            );
    }
    GX_EndLoadTex();                   // restore the texture image slots

    //---------------------------------------------------------------------------
    // Download the texture palettes:
    //
    // Transfer the texture palette data on the main memory to the texture palette slots.
    //---------------------------------------------------------------------------
    GX_BeginLoadTexPltt();             // map the texture palette slots onto LCDC address space
    {
        GX_LoadTexPltt((void *)&pal_4plett[0],  // a pointer to the texture data on the main memory(4 bytes aligned)
                       myTexPlttAddr,  // an offset address in the texture palette slots
                       8               // the size of the texture palette(s)(in bytes)
            );
    }
    GX_EndLoadTexPltt();               // restore the texture palette slots

    DEMOStartDisplay();
    while (1)
    {
        MtxFx43 camera;

        DEMOReadKey();
        if (DEMO_IS_PRESS(PAD_KEY_UP))
            texY += FX32_ONE >> 2;
        if (DEMO_IS_PRESS(PAD_KEY_DOWN))
            texY -= FX32_ONE >> 2;
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
            texX -= FX32_ONE >> 2;
        if (DEMO_IS_PRESS(PAD_KEY_LEFT))
            texX += FX32_ONE >> 2;
        if (DEMO_IS_PRESS(PAD_BUTTON_L))
            texRotate += 256;
        if (DEMO_IS_PRESS(PAD_BUTTON_R))
            texRotate -= 256;
        if (DEMO_IS_PRESS(PAD_BUTTON_A))
            texScale += FX32_ONE >> 4;
        if (DEMO_IS_PRESS(PAD_BUTTON_B))
            texScale -= FX32_ONE >> 4;
        if (texScale <= 0)
            texScale = FX32_ONE >> 4;

        G3X_Reset();

        //---------------------------------------------------------------------------
        // Set up a camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, 3 * FX32_ONE };       // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up
            G3_LookAt(&Eye, &vUp, &at, &camera);
        }

        //---------------------------------------------------------------------------n
        // Set up light colors and direction.
        // Notice that light vector is transformed by the current vector matrix
        // immediately after LightVector command is issued.
        //
        // GX_LIGHTID_0: white, downward
        //---------------------------------------------------------------------------
        G3_LightVector(GX_LIGHTID_0, FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        G3_PushMtx();

        //---------------------------------------------------------------------------
        // Set up the texture matrix for SRT animation
        //---------------------------------------------------------------------------
        {
            fx16    s = FX_SinIdx(texRotate);
            fx16    c = FX_CosIdx(texRotate);
            fx32    rScale = FX_Inv(texScale);

            //---------------------------------------------------------------------------
            // Initialize a texture matrix
            //---------------------------------------------------------------------------
            G3_MtxMode(GX_MTXMODE_TEXTURE);
            G3_Identity();
#if 1
            //---------------------------------------------------------------------------
            // Scale, Rotation, and Translation on the texture matrix:
            //
            // Notice that 3rd row and 4th row must be multiplied by 16.
            //---------------------------------------------------------------------------
            G3_Translate(32 * FX32_ONE, 32 * FX32_ONE, 0);      // 5. move the origin to the center of the texture
            G3_Translate(texX, texY, 0);        // 4. translate
            G3_RotZ(s, c);             // 3. rotate counterclockwise
            G3_Scale(rScale, rScale, FX32_ONE); // 2. scale
            G3_Translate(-32 * FX32_ONE, -32 * FX32_ONE, 0);    // 1. move the center of the texture to the origin
            {
                MtxFx44 mtx;
                MTX_Identity44(&mtx);
                mtx._22 = 16 * FX32_ONE;
                mtx._33 = 16 * FX32_ONE;
                G3_MultMtx44(&mtx);
            }
#else
            //---------------------------------------------------------------------------
            // CPU version
            //---------------------------------------------------------------------------
            {

                MtxFx44 mtx, tmp;
                MTX_Identity44(&mtx);

                mtx._30 = 32 * FX32_ONE + texX;
                mtx._31 = 32 * FX32_ONE + texY;

                MTX_RotZ44(&tmp, s, c);
                MTX_Concat44(&tmp, &mtx, &mtx);
                MTX_ScaleApply44(&mtx, &mtx, rScale, rScale, FX32_ONE);

                MTX_TransApply44(&mtx, &mtx, -32 * FX32_ONE, -32 * FX32_ONE, 0);

                //---------------------------------------------------------------------------
                // Load a texture matrix(in the case of GX_TEXGEN_TEXCOORD):
                //
                // Internaly, 3rd and 4th row are multiplied by 16 before
                // the argument matrix is set to the current texture matrix.
                //---------------------------------------------------------------------------
                G3_LoadTexMtxTexCoord(&mtx);    // Translation is internally multiplied by 16.
            }
#endif
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        }

        G3_TexImageParam(GX_TEXFMT_PLTT4,       // use 4 colors palette texture
                         GX_TEXGEN_TEXCOORD,    // use normal source
                         GX_TEXSIZE_S64,        // 64 pixels
                         GX_TEXSIZE_T64,        // 64 pixels
                         GX_TEXREPEAT_ST,       // repeat st
                         GX_TEXFLIP_NONE,       // no flip
                         GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
                         myTexAddr     // the offset of the texture image
            );

        G3_TexPlttBase(myTexPlttAddr,  // the offset of the texture palette
                       GX_TEXFMT_PLTT4 // 4 colors palette texture
            );

        // Set the material color( diffuse, ambient , specular ) as basic white 
        DEMO_Set3DDefaultMaterial(TRUE, TRUE);
        DEMO_Set3DDefaultShininessTable();

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_BACK,   // cull back
                       0,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);

        {
            G3_TexCoord(0, 0);
            G3_Vtx(-FX16_ONE, FX16_ONE, 0);
            G3_TexCoord(0, 64 * FX32_ONE);
            G3_Vtx(-FX16_ONE, -FX16_ONE, 0);
            G3_TexCoord(64 * FX32_ONE, 64 * FX32_ONE);
            G3_Vtx(FX16_ONE, -FX16_ONE, 0);
            G3_TexCoord(64 * FX32_ONE, 0);
            G3_Vtx(FX16_ONE, FX16_ONE, 0);
        }
        G3_End();

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        // default value for testing.
        texX = FX32_ONE >> 2 * 15;
        texY = FX32_ONE >> 2 * 25;
        texRotate = 256 * 8;
        texScale = FX32_ONE - (FX32_ONE >> 4) * 3;

        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xB17E279F);
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
