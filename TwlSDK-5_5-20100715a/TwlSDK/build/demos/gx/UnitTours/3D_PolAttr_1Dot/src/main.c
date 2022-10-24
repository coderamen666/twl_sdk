/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_PolAttr_1Dot
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
// A sample that controls the display of 1-dot polygons by depth value
// 
// This sample displays three cubes, and allows the user to change scale and depth using the +Control Pad.
// If the cubes are smaller than 1 dot, toggle visibility by pressing A.
// 
// USAGE:
//   UP, DOWN: Change the value of the depth limit at which a 1-pixel polygon is visible
//   L, R: Change the scale of the object
//   A: Toggle visibility of 1-dot polygons
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
//       Set vertex color
//---------------------------------------------------------------------------
inline void Color(void)
{
    G3_Color(GX_RGB(31, 31, 31));
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
//        Cube drawing information type
//  Description:
//---------------------------------------------------------------------------
#pragma  warn_padding off
typedef struct
{
    int     trg;
    fx32    scale;
    fx32    depth;
    u32     texAddr;
    u16     rotate;
    // [PADDING 2 BYTE HERE]
}
cubu_c;
#pragma  warn_padding reset


//---------------------------------------------------------------------------
//  Summary:
//        Draw cube
//  Description:
//
//  Input:
//        *p : Cube information pointer
//        x: x position
//        y: y position
//        z: z position
//---------------------------------------------------------------------------
static void Cube(cubu_c * p, fx32 x, fx32 y, fx32 z)
{
    G3_PushMtx();

    /* Cube rotation and position configuration */
    {
        fx16    s = FX_SinIdx(p->rotate);
        fx16    c = FX_CosIdx(p->rotate);

        G3_Translate(x, y, z);

        G3_RotX(s, c);
        G3_RotY(s, c);
        G3_RotZ(s, c);

        G3_Scale(p->scale, p->scale, p->scale);
    }

    /* Cube rendering configuration */
    // Set material's diffuse reflection color and ambient reflection color
    G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31), // Diffuse reflection color
                            GX_RGB(16, 16, 16), // Ambient reflection color
                            TRUE);     // Set vertex color
    // Configure the specular color and the emission color of the material
    G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16), // Specular reflection color
                            GX_RGB(0, 0, 0),    // The emission color
                            FALSE);    // Specular reflection not used
    // Polygon attribute settings

    // Specify texture parameters
    G3_TexImageParam(GX_TEXFMT_DIRECT, // Use direct texture
                     GX_TEXGEN_TEXCOORD,        // Set texture coordinate conversion mode
                     GX_TEXSIZE_S64,   // 64 texture s size
                     GX_TEXSIZE_T64,   // 64 texture t size
                     GX_TEXREPEAT_NONE, // No repeat
                     GX_TEXFLIP_NONE,  // No flip
                     GX_TEXPLTTCOLOR0_USE,      // Enable palette color 0 set value
                     p->texAddr);      // Texture address

    {
        int     attr;

        if (p->trg)
        {
            attr = GX_POLYGON_ATTR_MISC_DISP_1DOT;
        }
        else
        {
            attr = GX_POLYGON_ATTR_MISC_NONE;
        }
        // Polygon attribute settings
        G3_PolygonAttr(GX_LIGHTMASK_NONE,       // Do not reflect light
                       GX_POLYGONMODE_MODULATE, // Modulation mode
                       GX_CULL_NONE,   // No culling
                       0,              // Polygon ID 0
                       31,             // Alpha value
                       attr);          // 1-dot polygon display setting
    }
    // 
    G3X_SetDisp1DotDepth(p->depth);    // 1-dot polygon display border depth value

    G3_Begin(GX_BEGIN_QUADS);          // Start vertex list (draw with quadrilateral polygons)
    {
        TextureCoord(1);
        Color();
        Vertex(2);
        TextureCoord(0);
        Color();
        Vertex(0);
        TextureCoord(2);
        Color();
        Vertex(4);
        TextureCoord(3);
        Color();
        Vertex(6);

        TextureCoord(1);
        Color();
        Vertex(7);
        TextureCoord(0);
        Color();
        Vertex(5);
        TextureCoord(2);
        Color();
        Vertex(1);
        TextureCoord(3);
        Color();
        Vertex(3);

        TextureCoord(1);
        Color();
        Vertex(6);
        TextureCoord(0);
        Color();
        Vertex(4);
        TextureCoord(2);
        Color();
        Vertex(5);
        TextureCoord(3);
        Color();
        Vertex(7);

        TextureCoord(1);
        Color();
        Vertex(3);
        TextureCoord(0);
        Color();
        Vertex(1);
        TextureCoord(2);
        Color();
        Vertex(0);
        TextureCoord(3);
        Color();
        Vertex(2);

        TextureCoord(1);
        Color();
        Vertex(5);
        TextureCoord(0);
        Color();
        Vertex(4);
        TextureCoord(2);
        Color();
        Vertex(0);
        TextureCoord(3);
        Color();
        Vertex(1);

        TextureCoord(1);
        Color();
        Vertex(6);
        TextureCoord(0);
        Color();
        Vertex(7);
        TextureCoord(2);
        Color();
        Vertex(3);
        TextureCoord(3);
        Color();
        Vertex(2);
    }
    G3_End();                          // End vertex list

    G3_PopMtx(1);
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
//        Setting GX_POLYGON_ATTR_MISC_DISP_1DOT in G3_PolygonAttr
//        (Use depth value to suppress 1-dot polygon display)
//  Description:
//        Use depth value to suppress 1-dot polygon display
//
//        1. When the sample starts up, press BUTTON_L and set the object's Scale value to 0
//         
//        2. The scale value becomes 0, and three objects are displayed as 1 dot
//        3. By pressing UP and DOWN on the +Control Pad and increasing or decreasing the depth value, objects displayed in the background will blink in order, starting from the object on the left (which is the deepest object)
//         
//  Controls:
//        BUTTON_L, R: Object scale value
//        UP, DOWN: Increase and decrease the object's depth value
//        START: Return to initial value
//        SELECT: Update the depth value with a close value in the position test
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    cubu_c  cubu;

    cubu.trg = 1;
    cubu.scale = FX32_ONE;
    cubu.depth = 16384;
    cubu.texAddr = 0x01000;            // Slot address of the texture image
    cubu.rotate = 0;

    /*
     * Initialize (enable IRQ, initialize VRAM, use BG0 in 3D mode)
     */
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    /* Load the texture image to the texture image slot */
    GX_BeginLoadTex();                 // Map the bank allocated to the slot to LCDC memory
    {
        GX_LoadTex((void *)&tex_32768_64x64[0], // Load source pointer
                   cubu.texAddr,       // Load destination slot address
                   8192);              // Load size
    }
    GX_EndLoadTex();                   // Restore the slot mapped to LCDC to its original state

    DEMOStartDisplay();

    while (1)
    {
        G3X_Reset();
        cubu.rotate += 256;

        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] =
                { {PAD_BUTTON_L, 80}, {PAD_KEY_DOWN, 8}, {PAD_BUTTON_A, 1}, {0, 0} };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_PRESS(PAD_BUTTON_R))
        {
            cubu.scale += 64;
            OS_Printf("Scale=%d Depth=%d\n", cubu.scale, cubu.depth);
        }
        else if (DEMO_IS_PRESS(PAD_BUTTON_L))
        {
            cubu.scale -= 64;
            if (cubu.scale < 0)
            {
                cubu.scale = 0;
            }
            OS_Printf("Scale=%d Depth=%d\n", cubu.scale, cubu.depth);
        }
        if (DEMO_IS_PRESS(PAD_KEY_UP))
        {
            cubu.depth += 256;
            if (cubu.depth > 0xffffff)
            {
                cubu.depth = 0xffffff;
            }
            OS_Printf("Scale=%d Depth=%d\n", cubu.scale, cubu.depth);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_DOWN))
        {
            cubu.depth -= 256;
            if (cubu.depth < 0)
            {
                cubu.depth = 0;
            }
            OS_Printf("Scale=%d Depth=%d\n", cubu.scale, cubu.depth);
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            cubu.trg = (cubu.trg + 1) & 0x01;
            OS_Printf("PolygonAttr_1DotPoly=%s\n", (cubu.trg ? "ON" : "OFF"));
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
            cubu.scale = FX32_ONE;
            cubu.depth = 16384;
            OS_Printf("Scale=%d Depth=%d\n", cubu.scale, cubu.depth);
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
        {
            fx16    s = FX_SinIdx(cubu.rotate);
            fx16    c = FX_CosIdx(cubu.rotate);

            G3_Translate(-FX32_ONE, 0, -4 * FX32_ONE);

            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);

            G3_Scale(cubu.scale, cubu.scale, cubu.scale);

            {
                VecFx32 m;
                fx32    w;

                G3_PositionTest(FX16_ONE, FX16_ONE, FX16_ONE);
                while (G3X_GetPositionTestResult(&m, &w))
                {
                    // Wait for position test
                }

                OS_Printf("Position Test : Pos(%d, %d, %d) W(%d)\n", m.x, m.y, m.z, w);
                cubu.depth = w;
            }
        }

        /* Camera configuration */
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Eye position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

            G3_LookAt(&Eye, &vUp, &at, NULL);   // Eye point settings
        }

        G3_MtxMode(GX_MTXMODE_TEXTURE); // Set matrix to texture mode
        G3_Identity();                 // Initialize current matrix to a unit matrix
        // The matrix is set to Position-Vector simultaneous set mode
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        /* Draw cube */
        Cube(&cubu, FX32_ONE, 0, -2 * FX32_ONE);
        Cube(&cubu, 0, 0, -3 * FX32_ONE);
        Cube(&cubu, -FX32_ONE, 0, -4 * FX32_ONE);

        // Swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        /* V-Blank wait */
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // For auto test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(150, 0xA00DF599);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

    }
}
