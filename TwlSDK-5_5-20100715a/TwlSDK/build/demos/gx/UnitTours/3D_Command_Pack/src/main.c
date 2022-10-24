/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/3D_Command_Pack
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
// Use the command pack data generated in advance:
//
// Send the data by DMA/CPU transfer, and display a sphere rotating and lighted.
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "commandList.h"

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     count = 0;
    u16     Rotate = 0;                // for rotating spheresa(0-65535)

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for 3D mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplay3D();

    DEMOStartDisplay();
    while (1)
    {
        G3X_Reset();
        count = (u16)((count + 1) & 0x00ff);
        Rotate += 256;

        //---------------------------------------------------------------------------
        // Set up camera matrix
        //---------------------------------------------------------------------------
        {
            VecFx32 Eye = { 0, 0, FX32_ONE * 5 };       // Eye Position
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
        // GX_LIGHTID_1: red, leftward
        // GX_LIGHTID_2: green, upward
        // GX_LIGHTID_3: blue, rightward
        //---------------------------------------------------------------------------
        G3_LightVector(GX_LIGHTID_0, 0, (fx16)-FX32_SQRT1_2, (fx16)-FX32_SQRT1_2);
        G3_LightColor(GX_LIGHTID_0, GX_RGB(31, 31, 31));

        G3_LightVector(GX_LIGHTID_1, (fx16)-FX32_SQRT1_2, 0, (fx16)-FX32_SQRT1_2);
        G3_LightColor(GX_LIGHTID_1, GX_RGB(31, 0, 0));

        G3_LightVector(GX_LIGHTID_2, 0, (fx16)FX32_SQRT1_2, (fx16)-FX32_SQRT1_2);
        G3_LightColor(GX_LIGHTID_2, GX_RGB(0, 31, 0));

        G3_LightVector(GX_LIGHTID_3, (fx16)FX32_SQRT1_2, 0, (fx16)-FX32_SQRT1_2);
        G3_LightColor(GX_LIGHTID_3, GX_RGB(0, 0, 31));

        G3_PushMtx();
        G3_Translate(0, 0, -2 * FX32_ONE);

        //---------------------------------------------------------------------------
        // Rotate a sphere
        //---------------------------------------------------------------------------
        G3_PushMtx();
        {
            fx16    s = FX_SinIdx(Rotate);
            fx16    c = FX_CosIdx(Rotate);

            G3_RotX(s, c);
            G3_RotY(s, c);
            G3_RotZ(s, c);
        }

        // Set the material color( diffuse, ambient , specular ) as basic white 
        DEMO_Set3DDefaultMaterial(TRUE, TRUE);
        DEMO_Set3DDefaultShininessTable();

        G3_PolygonAttr((GXLightMask)(count >> 4),       // no lights
                       GX_POLYGONMODE_MODULATE, // modulation mode
                       GX_CULL_BACK,   // cull back
                       0,              // polygon ID(0 - 63)
                       31,             // alpha(0 - 31)
                       0               // OR of GXPolygonAttrMisc's value
            );

        //---------------------------------------------------------------------------
        // Draw a sphere:
        // The geometry data for a sphere is generated in advance(commandList.c).
        // Send them to the geometry FIFO by DMA/CPU, and display a sphere.
        //---------------------------------------------------------------------------
        G3_Begin(GX_BEGIN_TRIANGLES);

#if 1
        // Transfer by DMA
        MI_SendGXCommand(3, sphere1, sphere_size);

#else
        // Transfer by CPU
        {
            int     i;
            for (i = 0; i < sphere_size / 4; ++i)
            {
                reg_G3X_GXFIFO = sphere1[i];
            }
        }
#endif
        G3_End();

        G3_PopMtx(1);

        G3_PopMtx(1);

        // swapping the polygon list RAM, the vertex RAM, etc.
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xE405BA9B);
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
