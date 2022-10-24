/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Env_Mapping
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
// A sample for environment mapping
//
// HOWTO:
// 1. Set up the texture matrix.
// 2. Specify GX_TEXGEN_NORMAL as a parameter for G3_TexImageParam.
//
// Notice that the normal vectors sent to the texture matrix are in the local space.
// Thus, you must multiply the appropriate rotational matrices in advance.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "tex_4plett.h"

MtxFx44 textureMtx = {
    32 * FX32_ONE, 0, 0, 0,
    0, -32 * FX32_ONE, 0, 0,
    0, 0, FX32_ONE, 0,
    0, 0, 0, FX32_ONE
};


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

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u32     myTexAddr = 0x1000;        // texture image address in texture image slots
    u32     myTexPlttAddr = 0x1000;    // texture palette address in texture palette slots
    u16     Rotate = 0;

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

        G3X_Reset();
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up a camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, 10 * FX32_ONE };      // Eye position
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

        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotZ(s, c);
            G3_RotY(s, c);
            G3_RotX(s, c);
        }

        //---------------------------------------------------------------------------
        // Set up the texture matrix for environment mapping
        //---------------------------------------------------------------------------
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            //---------------------------------------------------------------------------
            // Load a texture matrix(in the case of GX_TEXGEN_NORMAL):
            // 
            // Internally, the argument matrix is (16, 16, 16) scaled
            // before it is set to the current texture matrix.
            // If you use G3_MtxMode(GX_MTXMODE_TEXTURE) and G3_LoadMtx44,
            // you have to scale the matrix by yourself.
            //---------------------------------------------------------------------------
            G3_LoadTexMtxEnv(&textureMtx);
#if 0
            // You can see the texture in the same way if you multiply the camera matrix at this point.
            // Generally, the matrix at this point specifies the direction of the light/environment.
            G3_MultMtx33((MtxFx33 *)&camera);   // a 3x3 matrix is enough
#endif
            G3_RotZ(s, c);
            G3_RotY(s, c);
            G3_RotX(s, c);

            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

            //---------------------------------------------------------------------------
            // Specify the center of the texture.
            // (32, 32) specified because a 64x64 texture is used.
            //---------------------------------------------------------------------------
            G3_TexCoord(32 * FX32_ONE, 32 * FX32_ONE);
        }

        G3_TexImageParam(GX_TEXFMT_PLTT4,       // use 4 colors palette texture
                         GX_TEXGEN_NORMAL,      // use normal source
                         GX_TEXSIZE_S64,        // 64 pixels
                         GX_TEXSIZE_T64,        // 64 pixels
                         GX_TEXREPEAT_NONE,     // no repeat
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
            NrmVtxQuad(2, 0, 4, 6);
            NrmVtxQuad(7, 5, 1, 3);
            NrmVtxQuad(6, 4, 5, 7);
            NrmVtxQuad(3, 1, 0, 2);
            NrmVtxQuad(5, 4, 0, 1);
            NrmVtxQuad(6, 7, 3, 2);
        }
        G3_End();

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x440EB69B);
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
