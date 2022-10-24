/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_PolAttr_DpthUpdate
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that sets GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE in G3_PolygonAttr.
// The depth buffer is updated when the transparent polygon is drawn.
// 
// This sample displays a transparent cube and cylinder. If the switch is ON, 
// the cube is drawn with GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE parameter.
// 
// USAGE:
//   UP, DOWN   : Change alpha blend parameter
//   START      : Switch ON/OFF GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE
// 
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"

//---------------------------------------------------------------------------
//  Summary:
//        Cube model data
//---------------------------------------------------------------------------
/* Vertex data */
static const s16 s_Vertex[3 * 8] = {
    FX16_ONE, FX16_ONE, FX16_ONE,
    FX16_ONE, FX16_ONE, -FX16_ONE,
    FX16_ONE, -FX16_ONE, FX16_ONE,
    FX16_ONE, -FX16_ONE, -FX16_ONE,
    -FX16_ONE, FX16_ONE, FX16_ONE,
    -FX16_ONE, FX16_ONE, -FX16_ONE,
    -FX16_ONE, -FX16_ONE, FX16_ONE,
    -FX16_ONE, -FX16_ONE, -FX16_ONE
};

//---------------------------------------------------------------------------
//        Set vertex coordinates
//  Description:
//        Use specified vertex data to set vertex coordinates
//  Input:
//        i_idx: Vertex data ID
//---------------------------------------------------------------------------
static void Vertex(int i_idx)
{
    G3_Vtx(s_Vertex[i_idx * 3], s_Vertex[i_idx * 3 + 1], s_Vertex[i_idx * 3 + 2]);
}

//---------------------------------------------------------------------------
//  Summary:
//        Generate a cube surface
//  Description:
//        Generate one cube surface
//  Input:
//        i_idx0 - i_id3 : Structural vertex data ID
//---------------------------------------------------------------------------
inline void Quad(int i_idx0, int i_idx1, int i_idx2, int i_idx3)
{
    Vertex(i_idx0);
    Vertex(i_idx1);
    Vertex(i_idx2);
    Vertex(i_idx3);
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
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Sets a V-Blank interrupt confirmation flag
}

//---------------------------------------------------------------------------
//  Summary:
//       Sets TR_MISC_XLU_DEPTH_UPDATE in G3_PolygonAttr
//       (Updates the depth buffer when drawing translucent polygons)
//  Description:
//        The semi-transparent cube with fog applied that is the target of the depth test orbits the periphery of the normal display rectangle
//       
//        If depth value updating is OFF, the influence of the fog on the depth value will not be shown on the semi-transparent cube.
//      When the depth value update is ON, the effect of fog will be reflected.
//  Controls:
//       START: Switch depth value update ON/OFF
//       UP, DOWN: Increase and decrease the object's alpha value
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    unsigned int count = 0;
    int     alpha = 0x0a;
    BOOL    trg = 1;
    u16     Rotate = 0;

    //---------------------
    // Initialize (enable IRQ, initialize VRAM, use BG0 in 3D mode)
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    /* Fog configuration */
    {
        u32     fog_table[8];
        int     i;

        // Clear color settings
        G3X_SetClearColor(GX_RGB(0, 0, 0),      // Clear color
                          0,           // Clear alpha value
                          0x7fff,      // Clear depth value
                          63,          // Clear polygon ID
                          TRUE);       // Use fog
        // Fog attribute settings
        G3X_SetFog(TRUE,               // Enable fog
                   GX_FOGBLEND_COLOR_ALPHA,     // Apply fog to color and alpha
                   GX_FOGSLOPE_0x2000, // Fog gradient settings
                   0x5800);            // Fog calculation depth value
        G3X_SetFogColor(GX_RGB(31, 31, 31), 0); // Fog color settings

        // Fog table settings (increasing the values thickens the fog)
        for (i = 0; i < 8; i++)
        {
            fog_table[i] = (u32)(((i * 16) << 0) |
                                 ((i * 16 + 4) << 8) |
                                 ((i * 16 + 8) << 16) | ((i * 16 + 12) << 24));
        }
        G3X_SetFogTable(&fog_table[0]);
    }

    G3X_AlphaBlend(TRUE);              // Enable alpha blending

    /* Swap rendering engine reference data groups
       (Notice that in Z buffer mode and W buffer mode depth values are different)
        */
    G3_SwapBuffers(GX_SORTMODE_AUTO,   // Auto sort mode
                   GX_BUFFERMODE_Z);   // Buffering mode using Z values

    DEMOStartDisplay();

    OS_Printf("mssg%d:PolygonAttr_DepthUpdate=%s\n", count++, (trg ? "ON" : "OFF"));

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        /* Obtain pad information and configure operations */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] = { {PAD_KEY_UP, 10}, {0, 1}, {PAD_KEY_UP, 10}, {0, 1},
            {PAD_KEY_UP, 10}, {0, 1}, {PAD_KEY_UP, 10}, {0, 0}
            };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_TRIG(PAD_KEY_UP))
        {
            if (++alpha > 31)
            {
                alpha = 31;
            }
            OS_Printf("mssg%d:alpha=%d\n", count++, alpha);
        }
        else if (DEMO_IS_TRIG(PAD_KEY_DOWN))
        {
            if (--alpha < 0)
            {
                alpha = 0;
            }
            OS_Printf("mssg%d:alpha=%d\n", count++, alpha);
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
            trg = (trg + 1) & 0x01;
            OS_Printf("mssg%d:PolygonAttr_DepthUpdate=%s\n", count++, (trg ? "ON" : "OFF"));
        }

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 8 };       // Eye point position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);   // Eye point settings
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        G3_PushMtx();                  // Add matrix to the stack

        /* Calculate the cube rotation and set its position */
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotY(s, c);
            G3_Translate(0, 0, 5 * FX32_ONE);
            G3_RotX(s, c);
            G3_RotZ(s, c);
        }

        /* Cube render configuration (perform fog blending) */
        // Set a diffuse color and an ambient color of texture
        G3_MaterialColorDiffAmb(GX_RGB(31, 0, 0),       // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                TRUE); // Set vertex color
        // Set a specular color and an emission color of texture
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used

        // Cube polygon attribute (alpha and depth) settings
        {
            int     attr;

            if (trg)
            {
                attr = GX_POLYGON_ATTR_MISC_XLU_DEPTH_UPDATE;
            }
            else
            {
                attr = GX_POLYGON_ATTR_MISC_NONE;
            }
            attr |= GX_POLYGON_ATTR_MISC_FOG;
            // Polygon attribute settings
            G3_PolygonAttr(GX_LIGHTMASK_NONE,   // No light influence
                           GX_POLYGONMODE_MODULATE,     // Modulation mode
                           GX_CULL_BACK,        // Perform backface culling
                           0,          // Polygon ID 0
                           alpha,      // Alpha value setting
                           attr);
        }

        DrawCube();                    // Draw cube

        G3_PopMtx(1);

        /* Rectangle render configuration (do not perform fog blending) */
        G3_PushMtx();
        // Set a diffuse color and an ambient color of texture
        G3_MaterialColorDiffAmb(GX_RGB(0, 0, 31),       // Diffuse reflection color
                                GX_RGB(16, 16, 16),     // Ambient reflection color
                                TRUE); // Set vertex color
        // Set a specular color and an emission color of texture
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular reflection color
                                GX_RGB(0, 0, 0),        // The emission color
                                FALSE); // Specular reflection not used
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_NONE,       // No light influence
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_BACK,   // Perform backface culling
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       0);

        G3_Begin(GX_BEGIN_QUADS);      // Begin vertex list (draw with quadrilateral polygons)
        {
            G3_Vtx(FX32_ONE << 2, -FX32_ONE << 2, 0);
            G3_Vtx(FX32_ONE << 2, FX32_ONE << 2, 0);
            G3_Vtx(-FX32_ONE, FX32_ONE << 2, 0);
            G3_Vtx(-FX32_ONE, -FX32_ONE << 2, 0);
        }
        G3_End();                      // End vertex list

        G3_PopMtx(1);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

        /* Waiting for the V-Blank */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x2A787000);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
