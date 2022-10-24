/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Shadow_Pol
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
// A sample that uses shadow polygons
// 
// This sample displays a sphere and the ground.
// The sphere's shadow is drawn on the ground.
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
//        Uses shadow polygons to display shadows
//  Description:
//        1. Prepare two identical cylinders that will be used to make the shadow cast on the ground by the sphere.
//          
//        2. Prepare a cylinder whose PolygonAttr polygon attributes are shadow polygons for the mask; only rear surfaces are to be rendered.
//          
//        3. Next, at the same coordinates as in the preceding step, prepare a cylinder whose PolygonAttr polygon attributes are shadow polygons for rendering; only front surfaces are to be rendered.
//          
//           Assign vertex color to the rendered cylinder.
//        4. If this cylinder is made to intersect with the polygons that will become the ground, the part where the cylinder and the ground intersect will be displayed as a shadow.
//          
//           The color of the shadow displayed will be the shadow polygon's color for rendering.
//          
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     Rotate = 0;                // For rotating sphere (0-65535)

    //---------------------
    // Initialize (enable IRQ, initialize VRAM, use BG0 in 3D mode)
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    G3X_AlphaBlend(TRUE);              // Enable alpha blending

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 15 * FX32_ONE, -15 * FX32_ONE }; // Eye point position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);   // Eye point settings
        }

        /* Light configuration (configures light color and direction) */
        {
            G3_LightVector(GX_LIGHTID_0, 0, -FX32_ONE + 1, 0);
            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        /* Terrain rendering settings */
        G3_PushMtx();
        // Set material's diffuse reflection color and ambient reflection color
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                FALSE); // Do not use vertex color

        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_NONE,        // Do not use texture
                         GX_TEXGEN_NONE,
                         GX_TEXSIZE_S8,
                         GX_TEXSIZE_T8,
                         GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Light 0 ON
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_NONE,   // Do not perform backface culling
                       2,              // Polygon ID 2
                       31,             // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);

        G3_Scale(10 * FX32_ONE, 0, 10 * FX32_ONE);

        G3_Begin(GX_BEGIN_QUADS);      // Start vertex list (draw with quadrilateral polygons)
        G3_Normal(0, 1 << 9, 0);
        G3_Vtx(FX16_ONE, 0, FX16_ONE);
        G3_Vtx(FX16_ONE, 0, -FX16_ONE);
        G3_Vtx(-FX16_ONE, 0, -FX16_ONE);
        G3_Vtx(-FX16_ONE, 0, FX16_ONE);
        G3_End();                      // End vertex list

        G3_PopMtx(1);


        G3_PushMtx();
        /* Sphere rotation calculation and position configuration */
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotY(s, c);
            G3_Translate(0, 5 * FX32_ONE, -5 * FX32_ONE);
            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        /* Sphere render settings */
        // Set material's diffuse reflection color and ambient reflection color
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                FALSE); // Do not use vertex color

        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_NONE,        // Do not use texture
                         GX_TEXGEN_NONE,
                         GX_TEXSIZE_S8,
                         GX_TEXSIZE_T8,
                         GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Light 0 ON
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_BACK,   // Perform backface culling
                       1,              // Polygon ID 1
                       31,             // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);

        G3_Begin(GX_BEGIN_TRIANGLES);  // Begin vertex list (use triangular polygons)
        {
            MI_SendGXCommand(3, sphere1, sphere_size);
        }
        G3_End();                      // End vertex list

        G3_PopMtx(1);

        /* Shadow polygon position configuration for mask */
        G3_PushMtx();
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotY(s, c);
            G3_Translate(0, 0, -5 * FX32_ONE);
        }

        /* Shadow polygon render configuration for mask */
        // Set material's diffuse reflection color and ambient reflection color.
        G3_MaterialColorDiffAmb(GX_RGB(0, 0, 0),        // Diffuse reflection color
                                GX_RGB(0, 0, 0),        // Ambient reflection color
                                FALSE); // Do not use vertex color
        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(0, 0, 0),        // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_NONE,        // Do not use texture
                         GX_TEXGEN_NONE,
                         GX_TEXSIZE_S8,
                         GX_TEXSIZE_T8,
                         GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Do not reflect light
                       GX_POLYGONMODE_SHADOW,   // Shadow polygon mode
                       GX_CULL_FRONT,  // Perform frontface culling
                       0,              // Polygon ID 0
                       0x08,           // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);

        // Draw mask (cylinder)
        G3_Begin(GX_BEGIN_TRIANGLES);  // Begin vertex list (use triangular polygons)
        {
            MI_SendGXCommand(3, cylinder1, cylinder_size);
        }
        G3_End();                      // End vertex list

        /* Shadow polygon render configuration for rendering */
        // Set material's diffuse reflection color and ambient reflection color
        G3_MaterialColorDiffAmb(GX_RGB(0, 0, 0),        // Diffuse reflection color
                                GX_RGB(0, 0, 0),        // Ambient reflection color
                                TRUE); // Use vertex color
        // Configure the specular color and the emission color of the material
        G3_MaterialColorSpecEmi(GX_RGB(0, 0, 0),        // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Specify texture parameters
        G3_TexImageParam(GX_TEXFMT_NONE,        // Do not use texture
                         GX_TEXGEN_NONE,
                         GX_TEXSIZE_S8,
                         GX_TEXSIZE_T8,
                         GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_0, // Do not reflect light
                       GX_POLYGONMODE_SHADOW,   // Shadow polygon mode
                       GX_CULL_BACK,   // Perform backface culling
                       1,              // Polygon ID 1
                       0x08,           // Alpha value
                       GX_POLYGON_ATTR_MISC_NONE);      // 

        // Draw shadow (cylinder)
        G3_Begin(GX_BEGIN_TRIANGLES);  // Begin vertex list (use triangular polygons)
        {
            MI_SendGXCommand(3, cylinder1, cylinder_size);
        }
        G3_End();                      // End vertex list

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);

        /* Waiting for the V-Blank */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x213FA661);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
