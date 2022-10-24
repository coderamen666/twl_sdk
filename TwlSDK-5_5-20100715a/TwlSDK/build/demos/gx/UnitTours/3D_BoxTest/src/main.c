/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_BoxTest
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9663 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that uses BoxTest
// 
// This sample displays a cube and a rotating sphere.
// If the sphere comes in contact with the cube, it is displayed normally
// Otherwise it is displayed with a wire frame
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
//        Perform the box test
//  Description:
//        When a rotating wireframe sphere touches a surface of the cube that surrounds it, the result of the box test will be rendered normally
//       
// 
//        1. Establish a space in the orthogonal projection to perform culling with BoxTest
//        2. Using a cube equal in size to the sphere, run BoxTest on the current clip coordinate matrix established in the orthogonal projection
//          
//        3. Next, convert to a perspective projection, and use the wire frame to draw the previously established culling space
//          
//        4. When rendering the sphere, switch between normal rendering and wireframe rendering based on the results of the previous BoxTest
//       
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     Rotate = 0;                // For rotating cubes (0-65535)

    //---------------------
    // Initialization
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    G3X_Init();                        // Initialize 3D graphics
    G3X_InitTable();                   // Initialize graphic table
    G3X_InitMtxStack();                // Initialize matrix stack

    /* 3D graphics configuration */
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);       // Use BG number 0

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // 2D/3D mode
                       GX_BGMODE_4,    // Set BGMODE to 4
                       GX_BG0_AS_3D);  // Use BG 0 as 3D
    G2_SetBG0Priority(0);              // BG 0 display priority to top

    G3X_SetShading(GX_SHADING_HIGHLIGHT);       // Set shading mode
    G3X_AntiAlias(TRUE);               // Enable antialiasing

    G3_SwapBuffers(GX_SORTMODE_AUTO,   // Swap buffer settings
                   GX_BUFFERMODE_W);
    // Clear color settings
    G3X_SetClearColor(GX_RGB(0, 0, 0), // clear color
                      31,              // clear alpha
                      0x7fff,          // clear depth
                      63,              // clear polygon ID
                      FALSE);          // fog

    G3_ViewPort(0, 0, 255, 191);       // Viewport settings

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        int     sphere_alpha;

        G3X_Reset();
        Rotate += 256;

        /* Orthogonal projection settings */
        {
            fx32    pos1_x = -((5 * FX32_ONE) / 2);
            fx32    pos1_y = ((5 * FX32_ONE) / 2);
            fx32    pos2_x = ((5 * FX32_ONE) / 2);
            fx32    pos2_y = -((5 * FX32_ONE) / 2);
            fx32    near = (5 * FX32_ONE) / 2;
            fx32    far = ((5 * FX32_ONE) / 2) + (5 * FX32_ONE);

            G3_MtxMode(GX_MTXMODE_PROJECTION);  // Set the matrix to projection mode
            G3_Identity();             // Initialize current matrix to a unit matrix
            // Orthogonal projection settings
            G3_Ortho(pos1_y,           // up    pos y
                     pos2_y,           // down  pos y
                     pos1_x,           // left  pos x
                     pos2_x,           // right pos x
                     near,             // Near
                     far,              // Far
                     NULL);

            G3_StoreMtx(0);
        }

        /* Box test */
        {
            G3_MtxMode(GX_MTXMODE_TEXTURE);     // Set matrix to texture mode
            G3_Identity();             // Initialize current matrix to a unit matrix
            // The matrix is set to Position-Vector simultaneous set mode
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
            G3_PushMtx();
            {
                GXBoxTestParam box;
                fx16    s = FX_SinIdx(Rotate);
                fx16    c = FX_CosIdx(Rotate);
                s32     result;

                G3_Translate(0, 0, -11 * FX32_ONE);
                G3_RotY(s, c);
                G3_Translate(0, 0, 6 * FX32_ONE);
                G3_RotX(s, c);
                G3_RotY(s, c);
                G3_RotZ(s, c);

                // Set FAR clip and 1-dot polygon rendering to ON before the BoxTest
                //  
                G3_PolygonAttr(GX_LIGHTMASK_0,
                               GX_POLYGONMODE_MODULATE,
                               GX_CULL_NONE,
                               0,
                               0,
                               GX_POLYGON_ATTR_MISC_FAR_CLIPPING | GX_POLYGON_ATTR_MISC_DISP_1DOT);
                G3_Begin(GX_BEGIN_TRIANGLES);
                G3_End();

                box.x = -7200;
                box.y = -7200;
                box.z = -7200;
                box.width = 7200 * 2;
                box.height = 7200 * 2;
                box.depth = 7200 * 2;
                G3_BoxTest(&box);      // Do the box test

                while (G3X_GetBoxTestResult(&result) != 0)
                {
                    // Loop to wait for box text results
                    ;
                }
                sphere_alpha = (result ? 31 : 0);
                OS_Printf("result %d\n", result);
            }
            G3_PopMtx(1);
        }

        /* Perspective projection settings */
        G3_MtxMode(GX_MTXMODE_PROJECTION);      // Set the matrix to projection mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        {
            fx32    right = FX32_ONE;
            fx32    top = FX32_ONE * 3 / 4;
            fx32    near = FX32_ONE;
            fx32    far = FX32_ONE * 400;
            // Perspective projection settings
            G3_Perspective(FX32_SIN30, // Sine   FOVY
                           FX32_COS30, // Cosine FOVY
                           FX32_ONE * 4 / 3,    // Aspect
                           near,       // Near
                           far,        // Far
                           NULL);

            G3_StoreMtx(0);
        }

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 8 * FX32_ONE, FX32_ONE };        // Eye point position
            VecFx32 at = { 0, 0, -5 * FX32_ONE };       // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);   // Eye point settings 
        }

        /* Light settings */
        {
            G3_LightVector(GX_LIGHTID_0, 0, -FX32_ONE + 1, 0);  // Light color
            G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));    // Light direction
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        /* Generate and render box */
        {
            G3_PushMtx();
            G3_Translate(0, 0, -5 * FX32_ONE);  // Set box position
            // Set material's diffuse reflection color and ambient reflection color
            G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // Diffuse reflection color
                                    GX_RGB(16, 16, 16), // Ambient reflection color
                                    FALSE);     // Do not set vertex color
            // Configure the specular color and the emission color of the material
            G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // Specular reflection color
                                    GX_RGB(0, 0, 0),    // The emission color
                                    FALSE);     // Do not use specular reflection table
            // Specify texture parameters
            G3_TexImageParam(GX_TEXFMT_NONE,    // No texture
                             GX_TEXGEN_NONE,    // No texture coordinate conversion
                             GX_TEXSIZE_S8,
                             GX_TEXSIZE_T8,
                             GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
            // Polygon attribute settings
            G3_PolygonAttr(GX_LIGHTMASK_0,      // Light 0 ON
                           GX_POLYGONMODE_MODULATE,     // Modulation mode
                           GX_CULL_NONE,        // No culling
                           1,          // Polygon ID 1
                           0,          // Alpha value
                           GX_POLYGON_ATTR_MISC_NONE);
            G3_Begin(GX_BEGIN_QUADS);  // Begin vertex list
            // (Declaration to draw with quadrilateral polygons)
            {
                G3_Normal(0, 1 << 9, 0);
                G3_Vtx(((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
                G3_Vtx(((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));

                G3_Normal(0, -1 << 9, 0);
                G3_Vtx(((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
                G3_Vtx(((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));

                G3_Normal(1 << 9, 0, 0);
                G3_Vtx(((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
                G3_Vtx(((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
                G3_Vtx(((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));

                G3_Normal(-1 << 9, 0, 0);
                G3_Vtx(-((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), -((5 * FX16_ONE) / 2));
                G3_Vtx(-((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2), ((5 * FX16_ONE) / 2));
            }
            G3_End();                  // End vertex list
            G3_PopMtx(1);
        }

        /* Generate and render sphere */
        {
            G3_PushMtx();              // Matrix to stack
            {                          // Calculate move position
                fx16    s = FX_SinIdx(Rotate);
                fx16    c = FX_CosIdx(Rotate);

                G3_Translate(0, 0, -11 * FX32_ONE);
                G3_RotY(s, c);
                G3_Translate(0, 0, 6 * FX32_ONE);
                G3_RotX(s, c);
                G3_RotY(s, c);
                G3_RotZ(s, c);
            }
            // Set material's diffuse reflection color and ambient reflection color
            G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // Diffuse reflection color
                                    GX_RGB(16, 16, 16), // Ambient reflection color
                                    FALSE);     // Do not set vertex color
            // Configure the specular color and the emission color of the material
            G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // Specular reflection color
                                    GX_RGB(0, 0, 0),    // The emission color
                                    FALSE);     // Do not use specular reflection table
            // Specify texture parameters
            G3_TexImageParam(GX_TEXFMT_NONE,    // No texture
                             GX_TEXGEN_NONE,    // No texture coordinate conversion
                             GX_TEXSIZE_S8,
                             GX_TEXSIZE_T8,
                             GX_TEXREPEAT_NONE, GX_TEXFLIP_NONE, GX_TEXPLTTCOLOR0_USE, 0);
            // Set polygon related attribute values
            G3_PolygonAttr(GX_LIGHTMASK_0,      // Light 0 ON
                           GX_POLYGONMODE_MODULATE,     // Modulation mode
                           GX_CULL_NONE,        // No culling
                           1,          // Polygon ID 1
                           sphere_alpha,        // Alpha value
                           GX_POLYGON_ATTR_MISC_NONE);

            OS_Printf("sphere_alpha=%d\n", sphere_alpha);
            // Draw sphere
            G3_Begin(GX_BEGIN_TRIANGLES);       /* Begin vertex list
                                                   (Declaration to draw with triangular polygons) */
            {
                MI_SendGXCommand(3, sphere1, sphere_size);
            }
            G3_End();                  // End vertex list
            G3_PopMtx(1);
        }

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        /* V-Blank wait */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xD28FEA7);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

    }
}
