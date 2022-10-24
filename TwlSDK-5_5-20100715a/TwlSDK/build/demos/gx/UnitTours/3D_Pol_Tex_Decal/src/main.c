/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Tex_Decal
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
// A sample that use DECAL polygons with an alpha texture:
//
// HOWTO:
// 1. Load texture images to the texture image slots by
//    GX_BeginLoadTex(), GX_LoadTex(), and GX_EndLoadTex().
// 2. Load texture palettes to the texture palette slots by
//    GX_BeginLoadTexPltt(), GX_LoadTexPltt(), GX_EndLoadTexPltt().
// 3. Specify the texture to use and etc. by G3_TexImageParam(GX_TEXFMT_ALPHA, ...).
// 4. Specify the palette to use by G3_TexPlttBase(..., GX_TEXFMT_ALPHA).
// 5. Prepare to use decal polygons by G3_PolygonAttr(...,  GX_POLYGONMODE_DECAL, ...).
// 6. The texcoords are specified by G3_TexCoord().
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "tex_alpha.h"


s16     gCubeGeometry[3 * 4] = {
    FX16_ONE, FX16_ONE, 0,
    FX16_ONE, -FX16_ONE, 0,
    -FX16_ONE, FX16_ONE, 0,
    -FX16_ONE, -FX16_ONE, 0
};

VecFx10 gCubeNormal[1] = {
    GX_VECFX10(0, 0, FX32_ONE - 1),
};

GXSt    gCubeTexCoord[] = {
    GX_ST(0, 0),
    GX_ST(0, 16 * FX32_ONE),
    GX_ST(16 * FX32_ONE, 0),
    GX_ST(16 * FX32_ONE, 16 * FX32_ONE)
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

inline void tex_coord(int idx)
{
    G3_Direct1(G3OP_TEXCOORD, gCubeTexCoord[idx]);
    // use G3_TexCoord if not packed yet
}


#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     Rotate = 0;                // for rotating cubes(0-65535)
    u32     myTexAddr = 0x01000;       // a texture image at 0x1000 of the texture image slots
    u32     myTexPlttAddr = 0x01000;   // a texture palette at 0x1000 of the texture palette slots

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    G3X_AlphaBlend(TRUE);              // enable alpha blending


    //---------------------------------------------------------------------------
    // Download the texture images:
    //
    // Transfer the texture data on the main memory to the texture image slots.
    //---------------------------------------------------------------------------
    GX_BeginLoadTex();                 // map the texture image slots onto LCDC address space
    {
        GX_LoadTex((void *)&tex_alpha[0],       // a pointer to the texture data on the main memory(4 bytes aligned)
                   myTexAddr,          // an offset address in the texture image slots
                   256                 // the size of the texture(s)(in bytes)
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
        GX_LoadTexPltt((void *)&pal_alpha[0],   // a pointer to the texture data on the main memory(4 bytes aligned)
                       myTexPlttAddr,  // an offset address in the texture palette slots
                       16);            // the size of the texture palette(s)(in bytes)
    }
    GX_EndLoadTexPltt();               // restore the texture palette slots

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        {
            G3_MtxMode(GX_MTXMODE_TEXTURE);
            G3_Identity();
            // Use an identity matrix for the texture matrix for simplicity
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
        }

        //---------------------------------------------------------------------------
        // Set up a camera matrix
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
        //
        // GX_LIGHTID_0: white, downward
        //---------------------------------------------------------------------------
        G3_LightVector(GX_LIGHTID_0, FX16_SQRT1_3, -FX16_SQRT1_3, -FX16_SQRT1_3);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        G3_PushMtx();
        G3_Translate(0, 0, -3 * FX32_ONE);

        //---------------------------------------------------------------------------
        // Modulation polygons with an alpha texture
        //---------------------------------------------------------------------------

        G3_PushMtx();
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_Translate(-3 * (FX32_ONE >> 1), 0, 0);
            G3_RotZ(s, c);
        }

        // Set the material color( diffuse, ambient , specular ) as basic white 
        DEMO_Set3DDefaultMaterial(TRUE, TRUE);
        DEMO_Set3DDefaultShininessTable();


        G3_TexImageParam(GX_TEXFMT_ALPHA,       // use alpha texture
                         GX_TEXGEN_TEXCOORD,    // use texcoord
                         GX_TEXSIZE_S16,        // 16 pixels
                         GX_TEXSIZE_T16,        // 16 pixels
                         GX_TEXREPEAT_NONE,     // no repeat
                         GX_TEXFLIP_NONE,       // no flip
                         GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
                         myTexAddr     // the offset of the texture image
            );

        G3_TexPlttBase(myTexPlttAddr,  // the offset of the texture palette
                       GX_TEXFMT_ALPHA // alpha texture
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_NONE,   // cull none
                       0,              // polygon ID(0 - 63)
                       16,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            tex_coord(1);
            normal(0);
            vtx(1);
            tex_coord(0);
            normal(0);
            vtx(0);
            tex_coord(2);
            normal(0);
            vtx(2);
            tex_coord(3);
            normal(0);
            vtx(3);
        }
        G3_End();

        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // Decal polygons with an alpha texture
        //---------------------------------------------------------------------------

        G3_PushMtx();
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_Translate(3 * (FX32_ONE >> 1), 0, 0);
            G3_RotZ(s, c);
        }

        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // diffuse
                                GX_RGB(16, 16, 16),     // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_TexImageParam(GX_TEXFMT_ALPHA,       // use alpha texture
                         GX_TEXGEN_TEXCOORD,    // use texcoord
                         GX_TEXSIZE_S16,        // 16 pixels
                         GX_TEXSIZE_T16,        // 16 pixels
                         GX_TEXREPEAT_NONE,     // no repeat
                         GX_TEXFLIP_NONE,       // no flip
                         GX_TEXPLTTCOLOR0_USE,  // use color 0 of the palette
                         myTexAddr     // the offset of the texture image
            );

        G3_TexPlttBase(myTexPlttAddr,  // the offset of the texture palette
                       GX_TEXFMT_ALPHA // alpha texture
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_DECAL,    // decal mode
                       GX_CULL_NONE,   // cull none
                       0,              // polygon ID(0 - 63)
                       16,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            tex_coord(1);
            normal(0);
            vtx(1);
            tex_coord(0);
            normal(0);
            vtx(0);
            tex_coord(2);
            normal(0);
            vtx(2);
            tex_coord(3);
            normal(0);
            vtx(3);
        }
        G3_End();

        G3_PopMtx(1);

        //---------------------------------------------------------------------------
        // The squares behind modulation/decal polygons
        //---------------------------------------------------------------------------

        G3_PushMtx();
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_Translate(0, 0, -10 * FX32_ONE);
            G3_RotZ((fx16)-s, c);
        }

        G3_MaterialColorDiffAmb(GX_RGB(31, 0, 0),       // diffuse
                                GX_RGB(31, 0, 0),       // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(31, 0, 0),       // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_TexImageParam(GX_TEXFMT_NONE,        // no texture
                         (GXTexGen)0,  // dummy
                         (GXTexSizeS)0, // dummy
                         (GXTexSizeT)0, // dummy
                         (GXTexRepeat)0,        // dummy
                         (GXTexFlip)0, // dummy
                         (GXTexPlttColor0)0,    // dummy
                         0             // dummy
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_NONE,   // cull none
                       1,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            normal(0);
            G3_Vtx(4 * FX16_ONE, 0, 0);
            G3_Vtx(4 * FX16_ONE, 4 * FX16_ONE, 0);
            G3_Vtx(0, 4 * FX16_ONE, 0);
            G3_Vtx(0, 0, 0);
        }
        G3_End();

        G3_MaterialColorDiffAmb(GX_RGB(0, 31, 0),       // diffuse
                                GX_RGB(0, 31, 0),       // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(0, 31, 0),       // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_NONE,   // cull none
                       2,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            normal(0);
            G3_Vtx(0, 0, 0);
            G3_Vtx(0, 4 * FX16_ONE, 0);
            G3_Vtx(-4 * FX16_ONE, 4 * FX16_ONE, 0);
            G3_Vtx(-4 * FX16_ONE, 0, 0);
        }
        G3_End();

        G3_MaterialColorDiffAmb(GX_RGB(0, 0, 31),       // diffuse
                                GX_RGB(0, 0, 31),       // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(0, 0, 31),       // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_NONE,   // cull none
                       3,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            normal(0);
            G3_Vtx(0, -4 * FX16_ONE, 0);
            G3_Vtx(0, 0, 0);
            G3_Vtx(-4 * FX16_ONE, 0, 0);
            G3_Vtx(-4 * FX16_ONE, -4 * FX16_ONE, 0);
        }
        G3_End();

        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 0),      // diffuse
                                GX_RGB(31, 31, 0),      // ambient
                                TRUE   // use diffuse as vtx color if TRUE
            );

        G3_MaterialColorSpecEmi(GX_RGB(31, 31, 0),      // specular
                                GX_RGB(0, 0, 0),        // emission
                                FALSE  // use shininess table if TRUE
            );

        G3_PolygonAttr(GX_LIGHTMASK_0, // Light #0 is on
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_NONE,   // cull none
                       4,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        G3_Begin(GX_BEGIN_QUADS);
        {
            normal(0);
            G3_Vtx(4 * FX16_ONE, -4 * FX16_ONE, 0);
            G3_Vtx(4 * FX16_ONE, 0, 0);
            G3_Vtx(0, 0, 0);
            G3_Vtx(0, -4 * FX16_ONE, 0);
        }
        G3_End();

        G3_PopMtx(1);

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x5B9F61B);
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
