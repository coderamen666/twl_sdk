/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Pol_Vertex_Source
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
// A sample that calls G3_TexImageParam() with GX_TEXGEN_VERTEX mode.
// 
// This sample displays a cube with a constant viewpoint texture
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
//         Cube model data
//---------------------------------------------------------------------------
/* Texture matrix */
static MtxFx44 s_TextureMtx = {
    32 * FX32_ONE, 0, 0, 0,
    0, -32 * FX32_ONE, 0, 0,
    0, 0, FX32_ONE, 0,
    0, 0, 0, FX32_ONE
};
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
static VecFx10 s_Normal[8] = {
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, FX32_SQRT1_3, -FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, FX32_SQRT1_3),
    GX_VECFX10(-FX32_SQRT1_3, -FX32_SQRT1_3, -FX32_SQRT1_3)
};

//---------------------------------------------------------------------------
//  Summary:
//        Set vertex coordinates
//  Description:
//        Use specified vertex data to set vertex coordinates
//  Input:
//        idx: Vertex data ID
//---------------------------------------------------------------------------
inline void Vertex(int idx)
{
    G3_Vtx(s_Vertex[idx * 3], s_Vertex[idx * 3 + 1], s_Vertex[idx * 3 + 2]);
}

//---------------------------------------------------------------------------
//  Summary:
//    　　 Set normals
//  Description:
//    　　
//  Input:
//        idx: Normal data ID
//---------------------------------------------------------------------------
inline void Normal(int idx)
{
    G3_Direct1(G3OP_NORMAL, s_Normal[idx]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Generate a cube surface
//  Description:
//        Generate one cube surface
//  Input:
//        idx0 - i_id3 : Structural vertex data ID
//---------------------------------------------------------------------------
static void Quad(int idx0, int idx1, int idx2, int idx3)
{
    Normal(idx0);
    Vertex(idx0);
    Normal(idx1);
    Vertex(idx1);
    Normal(idx2);
    Vertex(idx2);
    Normal(idx3);
    Vertex(idx3);
}

//---------------------------------------------------------------------------
//  Summary:
//        Cube rendering (w/o normals)
//  Description:
//    
//---------------------------------------------------------------------------
static void DrawCube(void)
{
    G3_Begin(GX_BEGIN_QUADS);          // Beginning of vertex list (draw with quadrilateral polygons)
    {
        Quad(2, 0, 4, 6);
        Quad(7, 5, 1, 3);
        Quad(6, 4, 5, 7);
        Quad(3, 1, 0, 2);
        Quad(5, 4, 0, 1);
        Quad(6, 7, 3, 2);
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
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}

//---------------------------------------------------------------------------
//  Summary:
//        Setting GX_TEXGEN_VERTEX in G3_TexImageParam  (Fixed eye point texture expression)
//  Description:
//        Sets GX_TEXGEN_VERTEX with G3_TexImageParam and pastes a texture onto a cube.
//      The result is that the texture applied to the cube appears fixed when viewed from the viewpoint, despite the rotation of the cube.
//      
//
//        Note that the vertex vectors sent to the texture matrix are local.
//        Therefore, you must increase the appropriate rotation matrixes ahead of time.
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u32     myTexAddr = 0x1000;        // Slot address of the texture image
    u32     myTexPlttAddr = 0x1000;    // Texture palette address, slot address
    u16     Rotate = 0;

    //---------------------
    // Initialize (enable IRQ, initialize VRAM, use BG0 in 3D mode)
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    /* Load the texture image to the texture image slot */
    GX_BeginLoadTex();                 // Map the bank allocated to the slot to LCDC memory.
    {
        GX_LoadTex((void *)&tex_4plett64x64[0], // Load source pointer
                   myTexAddr,          // Address of the slot that is the load destination
                   1024);              // Load size
    }
    GX_EndLoadTex();                   // Restore the slot mapped to LCDC to its original state

    /* Load to the texture palette slot of the texture palette */
    GX_BeginLoadTexPltt();             // Map the bank allocated to the slot to LCDC memory.
    {
        GX_LoadTexPltt((void *)&pal_4plett[0],  // Load source pointer
                       myTexPlttAddr,  // Load destination palette slot address
                       8);             // Load size
    }
    GX_EndLoadTexPltt();               // Restore the slot mapped to LCDC to its original state

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        MtxFx43 camera;

        G3X_Reset();
        Rotate += 256;

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 0, 10 * FX32_ONE };      // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up
            G3_LookAt(&Eye, &vUp, &at, &camera);
        }

        /* Light settings
           (Note that immediately after the LightVector command, the light matrix will be transformed by the current matrix)
            */
        {
            G3_LightVector(GX_LIGHTID_0, 0, -FX32_ONE + 1, 0);
            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
        }

        G3_PushMtx();

        /* Calculate the cube rotation and set its position */
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_Scale(FX32_ONE * 3, FX32_ONE * 3, FX32_ONE * 3);
            G3_RotZ(s, c);
            G3_RotY(s, c);
            G3_RotX(s, c);
        }

        /* Cube rendering configuration */
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            /* Setting for environment mapping onto cube
               The argument matrix is scaled to (16,16,16) before the current texture matrix is configured
               
               If G3_MtxMode(GX_MTXMODE_TEXTURE) or G3_LoadMtx44 are used, the matrix must scale itself
                */

            // Set the texture matrix by setting the matrix to texture mode
            G3_LoadTexMtxEnv(&s_TextureMtx);

            /* If you increase the camera matrixes here, you can see the texture with the same method.
               Normally, this matrix specifies the direction of ambient light. */
            // Multiply the current matrix with a 3x3 matrix
            G3_MultMtx33((MtxFx33 *)&camera);   // Multiply the current matrix by the camera matrix

            G3_RotZ(s, c);
            G3_RotY(s, c);
            G3_RotX(s, c);

            /* Configure the texture to be applied to the cube */
            // The matrix is set to Position-Vector simultaneous set mode
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

            // Texture coordinate settings
            G3_TexCoord(32 * FX32_ONE, 32 * FX32_ONE);  // Specify center of 64x64 texture
        }

        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_PLTT4,       // 4-color palette texture
                         GX_TEXGEN_VERTEX,      // Vertex source
                         GX_TEXSIZE_S64,        // 64 texture s size
                         GX_TEXSIZE_T64,        // 64 texture t size
                         GX_TEXREPEAT_NONE,     // No repeat
                         GX_TEXFLIP_NONE,       // No flip
                         GX_TEXPLTTCOLOR0_USE,  // Enable palette color 0 set value
                         myTexAddr);   // Texture address

        // Set the texture palette address
        G3_TexPlttBase(myTexPlttAddr,  // Texture palette address
                       GX_TEXFMT_PLTT4);        // Apply to 4-color palette texture

        // Set material's diffuse reflection color and ambient reflection color.
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                TRUE); // Set diffuse reflection color as vertex color

        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used

        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Reflect light 0
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_BACK,   // Perform backface culling
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       0);

        DrawCube();                    // Draw cube

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
        /* Waiting for the V-Blank */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x7E85CB3E);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
