/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_PolAttr_DpthTest
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
// A sample that sets GX_POLYGON_ATTR_MISC_DEPTHTEST_DECAL in G3_PolygonAttr
// 
// This sample displays two squares. If switch is ON, the right one is drawn
// only when the polygon's depth value equals the value in the depth buffer.
// 
// USAGE:
//   START      : Switch ON/OFF GX_POLYGON_ATTR_MISC_DEPTHTEST_DECAL
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
//        Rectangle model data
//---------------------------------------------------------------------------
/* Vertex data */
static s16 s_Vertex[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
};
/* Normal data */
static VecFx10 s_Normal = GX_VECFX10(0, 0, FX32_ONE - 1);
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
    G3_Vtx(s_Vertex[i_idx * 3], s_Vertex[i_idx * 3 + 1], s_Vertex[i_idx * 3 + 2]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Set normals
//  Description:
//    
//---------------------------------------------------------------------------
inline void Normal(void)
{
    G3_Direct1(G3OP_NORMAL, s_Normal);
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
//        Draw rectangles
//  Description:
// 
//---------------------------------------------------------------------------
static void DrawRectangle(void)
{
    G3_Begin(GX_BEGIN_QUADS);          // Begin vertex list (draw with quadrilateral polygons)
    {
        TextureCoord(1);
        Normal();
        Vertex(1);
        TextureCoord(3);
        Normal();
        Vertex(3);
        TextureCoord(2);
        Normal();
        Vertex(2);
        TextureCoord(0);
        Normal();
        Vertex(0);
    }
    G3_End();                          // End vertex list
}

//---------------------------------------------------------------------------
//  Summary:
//        V-Blank interrupt process
//  Description:
//        Enables a flag that confirms that a V-Blank interrupt has been performed.
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
//        Setting GX_POLYGON_ATTR_MISC_DEPTHTEST_DECAL in G3_PolygonAttr
//  Description:
//        Draws when the polygon depth value is the same as the depth buffer's depth value
//              
//        The rectangles shown on the left side are displayed normally. The rectangles shown on the right side are depth-targeted polygons.
//      When the effect is enabled, parts of the right rectangle will be displayed over the left rectangle where the depth value is the same and the rectangles overlap.
//      
//  Controls:
//        START: Toggles the effect ON/OFF
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u32     texAddr = 0x00000;         // Slot address of the texture image
    unsigned int count = 0;
    BOOL    trg = 0;
    u16     rotate = 0;

    //---------------------
    // Initialize (enable IRQ, initialize VRAM , use BG0 in 3D mode)
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

    DEMOStartDisplay();

    OS_Printf("mssg%d:PolygonAttr_DepthTest=%s\n", count++, (trg ? "ON" : "OFF"));

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        G3X_Reset();
        rotate += 128;

        /* Pad input */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] = { {0, 20}, {PAD_BUTTON_START, 20}, {0, 0} };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
            trg = (trg + 1) & 0x01;
            OS_Printf("mssg%d:PolygonAttr_DepthTest=%s\n", count++, (trg ? "ON" : "OFF"));
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
            G3_LightVector(GX_LIGHTID_0, 0, 0, -FX32_ONE + 1);  // Light direction
            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));    // Light color
        }

        /* Render settings for the rectangle shown on the left */
        G3_MtxMode(GX_MTXMODE_TEXTURE);
        G3_Identity();
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        G3_PushMtx();                  // Add matrix to the stack

        /* Left rectangle rotation calculation and position configuration */
        {
            fx16    s = FX_SinIdx(rotate);
            fx16    c = FX_CosIdx(rotate);

            G3_Translate(-FX32_ONE / 2, 0, -3 * FX32_ONE);
            G3_RotZ(s, c);
        }

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
        G3_PolygonAttr(GX_LIGHTMASK_0, // Reflect light 0
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_NONE,   // Do not cull
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);

        DrawRectangle();               // Draw rectangles

        G3_PopMtx(1);

        G3_PushMtx();

        /* Right rectangle rotation calculation and position configuration */
        {
            fx16    s = FX_SinIdx(rotate);
            fx16    c = FX_CosIdx(rotate);

            G3_Translate(FX32_ONE / 2, 0, -3 * FX32_ONE);
            G3_RotZ(s, c);
        }
        /* right display rectangle render configuration */
        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
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

        // Depth change settings for the rectangles that are displayed on the right
        {
            int     attr;

            if (trg)
            {
                attr = GX_POLYGON_ATTR_MISC_DEPTHTEST_DECAL;
            }
            else
            {
                attr = GX_POLYGON_ATTR_MISC_NONE;
            }
            // Polygon attribute settings
            G3_PolygonAttr(GX_LIGHTMASK_0,      // Light 0 ON
                           GX_POLYGONMODE_MODULATE,     // Modulation mode
                           GX_CULL_NONE,        // No culling
                           0,          // Polygon ID 0
                           31,         // Alpha value
                           attr);
        }

        DrawRectangle();               // Draw rectangles

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        OS_WaitVBlankIntr();           // V-Blank wait

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x292691D2);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

    }
}
