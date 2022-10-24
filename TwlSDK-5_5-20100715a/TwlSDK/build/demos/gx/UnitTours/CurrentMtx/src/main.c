/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/CurrentMtx
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
//  A sample to get current matrix.
//
//  One cube is displayed.
//  And you can see matrix data on time.
//
//  HOWTO:
//  1.  Call "G3X_GetClipMtx" to get current clip matrix.
//  2.  Call "G3X_GetVectorMtx" to get current vector matrix.
//
//  OPERATION:
//  1.  Push A button to see current clip matrix data on debug window.
//  2.  Push B button to see current vector matrix data on debug window.
//
//---------------------------------------------------------------------------
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "tex_32768.h"

//---------------------------------------------------------------------------
//  Cube model data
//---------------------------------------------------------------------------
// Vertex data
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
// Normal data
static VecFx10 s_Normal[6] = {
    GX_VECFX10(0, 0, FX32_ONE - 1),
    GX_VECFX10(0, FX32_ONE - 1, 0),
    GX_VECFX10(FX32_ONE - 1, 0, 0),
    GX_VECFX10(0, 0, -FX32_ONE + 1),
    GX_VECFX10(0, -FX32_ONE + 1, 0),
    GX_VECFX10(-FX32_ONE + 1, 0, 0)
};
// Texture coordinate data
static GXSt s_TextureCoord[] = {
    GX_ST(0, 0),
    GX_ST(0, 64 * FX32_ONE),
    GX_ST(64 * FX32_ONE, 0),
    GX_ST(64 * FX32_ONE, 64 * FX32_ONE)
};

//---------------------------------------------------------------------------
//  Set vertex coordinate.
//  input:
//      idx:  ID of vertex data
//---------------------------------------------------------------------------
inline void Vertex(int idx)
{
    G3_Vtx(s_Vertex[idx * 3], s_Vertex[idx * 3 + 1], s_Vertex[idx * 3 + 2]);
}

//---------------------------------------------------------------------------
//  Set normal setting.
//  input:
//      idx:  ID of normal data
//---------------------------------------------------------------------------
inline void Normal(int idx)
{
    G3_Direct1(G3OP_NORMAL, s_Normal[idx]);
}

//---------------------------------------------------------------------------
//  Set texture coordinate.
//  input:
//      idx:  ID of texture data
//---------------------------------------------------------------------------
inline void TextureCoord(int idx)
{
    G3_Direct1(G3OP_TEXCOORD, s_TextureCoord[idx]);
}

//---------------------------------------------------------------------------
//  Draw a cube and set texture.
//---------------------------------------------------------------------------
static void DrawCube(void)
{
    G3_Begin(GX_BEGIN_QUADS);          // start to set vertex.(use square polygon)
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
    G3_End();                          // end
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
    // Set flag which checks VBlank interrupt.
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//---------------------------------------------------------------------------
//  main
//---------------------------------------------------------------------------
#ifdef  SDK_CW_WA_CONSTPOOLS
#pragma optimization_level 1
#endif
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    unsigned int count = 0;
    u32     myTexAddr = 0x00000;       // Address of texture image slot
    u16     Rotate = 0;

    // Initialize
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    // Load texture image into texture image slot.
    GX_BeginLoadTex();
    {
        GX_LoadTex((void *)&tex_32768_64x64[0], // Src address
                   myTexAddr,          // Destination address
                   8192);              // Size to load
    }
    GX_EndLoadTex();

    DEMOStartDisplay();

    // main loop
    while (1)
    {
        G3X_Reset();
        Rotate += 256;

        // Read input
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // code for auto test
        {
            const EXTKeys keys[8] =
                { {0, 40}, {PAD_BUTTON_A, 1}, {0, 40}, {PAD_BUTTON_B, 1}, {0, 0} };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        // get current clip matrix
        if (DEMO_IS_TRIG(PAD_BUTTON_A))
        {
            MtxFx44 m;
            s32     result;
            while (1)
            {
                //---------------------------------------------------------------------------
                // get current clip matrix
                result = G3X_GetClipMtx(&m);
                //---------------------------------------------------------------------------
                if (!result)
                {
                    OS_Printf("mssg%d:Current Clip Matrix\n", count++);
                    OS_Printf("mssg%d:%d, %d, %d, %d\n", count++, m._00, m._01, m._02, m._03);
                    OS_Printf("mssg%d:%d, %d, %d, %d\n", count++, m._10, m._11, m._12, m._13);
                    OS_Printf("mssg%d:%d, %d, %d, %d\n", count++, m._20, m._21, m._22, m._23);
                    OS_Printf("mssg%d:%d, %d, %d, %d\n\n", count++, m._30, m._31, m._32, m._33);

#ifdef SDK_AUTOTEST
                    EXT_CompPrint("5321, 0, 0, 0", "%d, %d, %d, %d", m._00, m._01, m._02, m._03);
                    EXT_CompPrint("0, 7094, 0, 0", "%d, %d, %d, %d", m._10, m._11, m._12, m._13);
                    EXT_CompPrint("0, 0, -4117, -4096", "%d, %d, %d, %d", m._20, m._21, m._22,
                                  m._23);
                    EXT_CompPrint("0, 0, -8213, 0", "%d, %d, %d, %d", m._30, m._31, m._32, m._33);
#endif
                    break;
                }
            }
        }
        // get current vector matrix
        if (DEMO_IS_TRIG(PAD_BUTTON_B))
        {
            MtxFx33 m;
            s32     result;

            while (1)
            {
                //---------------------------------------------------------------------------
                // get current vector matrix
                result = G3X_GetVectorMtx(&m);
                //---------------------------------------------------------------------------
                if (!result)
                {
                    OS_Printf("mssg%d:Current Vector Matrix\n", count++);
                    OS_Printf("mssg%d:%d, %d, %d\n", count++, m._00, m._01, m._02);
                    OS_Printf("mssg%d:%d, %d, %d\n", count++, m._10, m._11, m._12);
                    OS_Printf("mssg%d:%d, %d, %d\n\n", count++, m._20, m._21, m._22);

#ifdef SDK_AUTOTEST
                    EXT_CompPrint("4096, 0, 0", "%d, %d, %d", m._00, m._01, m._02);
                    EXT_CompPrint("0, 4096, 0", "%d, %d, %d", m._10, m._11, m._12);
                    EXT_CompPrint("0, 0, 4096", "%d, %d, %d", m._20, m._21, m._22);
#endif
                    break;
                }
            }
        }

        // Camera setting
        {
            VecFx32 Eye = { 0, 0, FX32_ONE };   // Sight position
            VecFx32 at = { 0, 0, 0 };  // Viewpoint
            VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up
            G3_LookAt(&Eye, &vUp, &at, NULL);   // Sight setting
        }

        // Light setting
        G3_LightVector(GX_LIGHTID_0, 0, -FX32_ONE + 1, 0);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        // Matrix setting
        G3_MtxMode(GX_MTXMODE_TEXTURE);
        G3_Identity();
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
        G3_PushMtx();

        // Rotate and translate cube
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_Translate(0, 0, -5 * FX32_ONE);

            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        // Draw setting
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),     // Diffuse
                                GX_RGB(16, 16, 16),     // Ambient
                                TRUE); // Color
        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),     // Specular
                                GX_RGB(0, 0, 0),        // Emission
                                FALSE); // Shininess
        G3_TexImageParam(GX_TEXFMT_DIRECT,      // Texture format
                         GX_TEXGEN_TEXCOORD,    // Texture generation
                         GX_TEXSIZE_S64,        // Texture width
                         GX_TEXSIZE_T64,        // Texture height
                         GX_TEXREPEAT_NONE,     // Texture repeat
                         GX_TEXFLIP_NONE,       // Texture flip
                         GX_TEXPLTTCOLOR0_USE,  // Palette color
                         myTexAddr);   // Texture address
        G3_PolygonAttr(GX_LIGHTMASK_0, // Light
                       GX_POLYGONMODE_MODULATE, // Polygon mode
                       GX_CULL_NONE,   // Culling
                       0,              // Polygon ID
                       31,             // Alpha
                       GX_POLYGON_ATTR_MISC_NONE);      // Misc

        // Draw cube
        DrawCube();

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

        // Wait VBlank
        OS_WaitVBlankIntr();

#ifdef SDK_AUTOTEST                    // code for auto test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x8C9F5E42);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST
    }
}
