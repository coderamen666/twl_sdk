/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_PolAttr_FARClip
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that sets GX_POLYGON_ATTR_MISC_FAR_CLIPPING in G3_PolygonAttr
//
// This sample displays two cubes.
// The left one has normal settings and the right one uses far clipping.
// If a polygon crosses the far face, the right one is clipped.
//
// USAGE:
//   UP, DOWN   : Move z position of the object
//   START      : Reset position of the object
//
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

//---------------------------------------------------------------------------
//  Summary:
//        Cube model data
//---------------------------------------------------------------------------
/*  */
/* Vertex data */
static s16 s_Vertex[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, FX16_ONE, -FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, -FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, -FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, -FX16_ONE
};

/* Normal data */
static VecFx10 s_Normal[6] = {
    GX_VECFX10(0, 0, FX32_ONE - 1),
    GX_VECFX10(0, FX32_ONE - 1, 0),
    GX_VECFX10(FX32_ONE - 1, 0, 0),
    GX_VECFX10(0, 0, -FX32_ONE + 1),
    GX_VECFX10(0, -FX32_ONE + 1, 0),
    GX_VECFX10(-FX32_ONE + 1, 0, 0)
};

/* Texture coordinates */
static GXSt s_TextureCoord[] = {
    GX_ST(0, 0),
    GX_ST(0, 64 * FX32_ONE),
    GX_ST(64 * FX32_ONE, 0),
    GX_ST(64 * FX32_ONE, 64 * FX32_ONE)
};

//---------------------------------------------------------------------------
//  Summary:
//        Set vertex coordinates
//  Description:
//        Use specified vertex data to set vertex coordinates
//  Input:
//        i_idx: Vertex data ID
//---------------------------------------------------------------------------
inline void Vertex(int i_idx)
{
    G3_Vtx(s_Vertex[i_idx * 3],        // Set vertex coordinates
           s_Vertex[i_idx * 3 + 1], s_Vertex[i_idx * 3 + 2]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Set normals
//  Description:
//    
//  Input:
//        i_idx: Normal data ID
//---------------------------------------------------------------------------
inline void Normal(int i_idx)
{
    G3_Direct1(G3OP_NORMAL, s_Normal[i_idx]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Texture coordinate settings
//  Description:
//
//  Input:
//        i_idx: Texture data ID
//---------------------------------------------------------------------------
inline void TextureCoord(int i_idx)
{
    G3_Direct1(G3OP_TEXCOORD, s_TextureCoord[i_idx]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Draw cube
//---------------------------------------------------------------------------
static void DrawCube(void)
{
    G3_Begin(GX_BEGIN_QUADS);          // Begin vertex list (draw with quadrilateral polygons)
    {
        TextureCoord(1);
        Normal(0);
        Vertex(2);
        TextureCoord(0);
        Normal(0);
        Vertex(0);
        TextureCoord(2);
        Normal(0);
        Vertex(4);
        TextureCoord(3);
        Normal(0);
        Vertex(6);

        TextureCoord(1);
        Normal(3);
        Vertex(7);
        TextureCoord(0);
        Normal(3);
        Vertex(5);
        TextureCoord(2);
        Normal(3);
        Vertex(1);
        TextureCoord(3);
        Normal(3);
        Vertex(3);

        TextureCoord(1);
        Normal(5);
        Vertex(6);
        TextureCoord(0);
        Normal(5);
        Vertex(4);
        TextureCoord(2);
        Normal(5);
        Vertex(5);
        TextureCoord(3);
        Normal(5);
        Vertex(7);

        TextureCoord(1);
        Normal(2);
        Vertex(3);
        TextureCoord(0);
        Normal(2);
        Vertex(1);
        TextureCoord(2);
        Normal(2);
        Vertex(0);
        TextureCoord(3);
        Normal(2);
        Vertex(2);

        TextureCoord(1);
        Normal(1);
        Vertex(5);
        TextureCoord(0);
        Normal(1);
        Vertex(4);
        TextureCoord(2);
        Normal(1);
        Vertex(0);
        TextureCoord(3);
        Normal(1);
        Vertex(1);

        TextureCoord(1);
        Normal(4);
        Vertex(6);
        TextureCoord(0);
        Normal(4);
        Vertex(7);
        TextureCoord(2);
        Normal(4);
        Vertex(3);
        TextureCoord(3);
        Normal(4);
        Vertex(2);
    }

    G3_End();                          // End vertex list
}


//---------------------------------------------------------------------------
//  Summary:
//        V-Blank interrupt process
//  Description:
//        Enables a flag that confirms that a V-Blank interrupt has been performed
//
//        The following steps will be performed during common initialization (DEMOInitCommon), causing this function to be called during V-Blanks
//        * Select IRQ interrupt (OS_SetIrqMask)
//        * Register this function, which performs IRQ interrupts (OS_SetIRQFunction)
//        * Enable IRQ interrupts (OS_EnableIrq)
//        
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Sets a V-Blank interrupt confirmation flag
}

//---------------------------------------------------------------------------
//  Summary:
//         Clips polygons that intersect the far plane defined in G3_PolygonAttr
//  Description:
//          Far-plane clipping is not applied to the cube displayed on the left, but it is applied to the cube displayed on the right.
//          The far value of the view volume is significantly smaller than the value normally set for the view volume.
//          This is so that the programmer can watch the clipping process.
//          
//  Controls:
//        UP, DOWN: Manipulate the object's z coordinate position
//        START: Return to initial position
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     pos_z = -5;
    u16     rotate = 0;
    u32     texAddr = 0x01000;         // Slot address of the texture image

    //---------------------
    // Initialize (enable IRQ, initialize VRAM, use BG0 in 3D mode)
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    /* Load the texture image to the texture image slot */
    GX_BeginLoadTex();                 // Map the bank allocated to the slot to LCDC memory
    {
        GX_LoadTex((void *)&tex_32768_64x64[0], // Load source pointer
                   texAddr,            // Load destination slot address
                   8192);              // Load size
    }
    GX_EndLoadTex();                   // Restore the slot mapped to LCDC to its original state

    /* Perspective projection settings */
    {
        fx32    right = FX32_ONE;
        fx32    top = FX32_ONE * 3 / 4;
        fx32    near = FX32_ONE;
        fx32    far = FX32_ONE * 6;
        // Perspective projection settings
        G3_Perspective(FX32_SIN30,     // Sine   FOVY
                       FX32_COS30,     // Cosine FOVY
                       FX32_ONE * 4 / 3,        // Aspect
                       near,           // Near
                       far,            // Far
                       NULL);

        G3_StoreMtx(0);                // Save the matrix in stack number 0
    }

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        G3X_Reset();
        rotate += 256;

        /* Processes pad input */
        DEMOReadKey();

        if (DEMO_IS_TRIG(PAD_KEY_DOWN))
        {
            if (++pos_z > -3)
            {
                pos_z = -3;
            }
            OS_Printf("Pos_Z=%d\n", pos_z);
        }
        else if (DEMO_IS_TRIG(PAD_KEY_UP))
        {
            if (--pos_z < -7)
            {
                pos_z = -7;
            }
            OS_Printf("Pos_Z=%d\n", pos_z);
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
            pos_z = -5;
            OS_Printf("Pos_Z=%d\n", pos_z);
        }

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye point position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);   // Eye point settings
        }

        /* Light configuration (configures light color and direction) */
        {
            G3_LightVector(GX_LIGHTID_0, 0, 0, -FX32_ONE + 1);
            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        G3_PushMtx();                  // Add matrix to the stack

        /* Calculate the rotation of the cube on the right and set its position */
        {
            fx16    s = FX_SinIdx(rotate);
            fx16    c = FX_CosIdx(rotate);

            G3_Translate(FX32_ONE * 2, 0, pos_z * FX32_ONE);
            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        /* Render configuration for right cube */
        // Set material's diffuse reflection color and ambient reflection color
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                TRUE); // Set vertex color
        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used

        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_DIRECT,      // Use direct texture
                         GX_TEXGEN_TEXCOORD,    // Set texture coordinate conversion mode
                         GX_TEXSIZE_S64,        // 64 texture s size
                         GX_TEXSIZE_T64,        // 64 texture t size
                         GX_TEXREPEAT_NONE,     // No repeat
                         GX_TEXFLIP_NONE,       // No flip
                         GX_TEXPLTTCOLOR0_USE,  // Enable palette color 0 set value
                         texAddr);     // Texture address
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Light 0 ON
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_NONE,   // No culling
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       GX_POLYGON_ATTR_MISC_FAR_CLIPPING);

        DrawCube();                    // Draw cube

        G3_PopMtx(1);

        G3_PushMtx();
        /* Calculate the rotation of the cube on the left and set its position */
        {
            fx16    s = FX_SinIdx(rotate);
            fx16    c = FX_CosIdx(rotate);

            G3_Translate(-FX32_ONE * 2, 0, pos_z * FX32_ONE);

            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        /* Render configuration for left cube */
        // Set material's diffuse reflection color and ambient reflection color
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                TRUE); // Set vertex color
        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_DIRECT,      // Use direct texture
                         GX_TEXGEN_TEXCOORD,    // Set texture coordinate conversion mode
                         GX_TEXSIZE_S64,        // 64 texture s size
                         GX_TEXSIZE_T64,        // 64 texture t size
                         GX_TEXREPEAT_NONE,     // No repeat
                         GX_TEXFLIP_NONE,       // No flip
                         GX_TEXPLTTCOLOR0_USE,  // Enable palette color 0 set value
                         texAddr);     // Texture address
        G3_PolygonAttr(GX_LIGHTMASK_0, // Light 0 ON
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_NONE,   // Do not perform backface culling
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);

        DrawCube();                    // Draw cube

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        /* V-Blank wait */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x954F11C7);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
