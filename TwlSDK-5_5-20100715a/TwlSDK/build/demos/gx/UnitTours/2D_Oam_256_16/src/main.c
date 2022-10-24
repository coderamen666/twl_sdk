/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_256_16
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
// A sample that use extended 256x16 color palette in OBJ:
// 
// This sample displays some objects on the display with extended 256x16 color 
// palette
// 
// USAGE:
//   UP, DOWN, LEFT, RIGHT      : Slide the OBJs
//   A, B                       : Scaling the OBJs
//   L, R                       : Rotate the OBJs
//   SELECT     : Switch palette Number of the OBJs
//   START      : Initialize status of the OBJs
// 
// HOWTO:
// 1. Allocate VRAM to extended palette with GX_SetBankForOBJExtPltt
// 2. Transfer the palette data with GX_BeginLoadOBJExtPltt, GX_LoadOBJExtPltt,
//    and GX_EndLoadOBJExtPltt
// 3. Set GX_OAM_COLOR_256 to OAM attribute to use the extended palette
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
//        OAM buffer region
//---------------------------------------------------------------------------
static GXOamAttr s_OAMBuffer[128];

//---------------------------------------------------------------------------
//  Summary:
//    VBlank interrupt process
//  Description:
//    Enables a flag that confirms that a V-Blank interrupt has been performed
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
//   Displays 256 color x 16 extended palette character of affine OBJ
//  Description:
//   Performs an extended affine transformation (rotation / scale) on an OBJ with sixteen 256-color extended palettes and displays the result
//       
//  Controls:
//        UP, DOWN: Manipulate display position
//        BUTTON_A, B : Enlarge, reduce
//        BUTTON_L, R : rotation
//        SELECT     : Switch palette
//        START      : Initialize settings
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     base_pos_x = 0, base_pos_y = 0;
    int     base_pal = 0;
    int     base_pat = 0;
    fx32    scale = FX32_ONE;
    u16     rotate = 0;

    //---------------------
    // Initialization
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayOBJOnly();

    /* Allocate VRAM bank to OBJ extended palette. */
    GX_SetBankForOBJExtPltt(GX_VRAM_OBJEXTPLTT_0_F);

    /* Load character data and palette data. */
    GX_LoadOBJ(bitmapOBJ_256color_icg, // Transfer OBJ data to VRAM A
               0, sizeof(bitmapOBJ_256color_icg));
    {
        GX_BeginLoadOBJExtPltt();      // Prepare to transfer palette data
        GX_LoadOBJExtPltt(bitmapOBJ_256color_icl,       // Transfer palette data to VRAM
                          0, sizeof(bitmapOBJ_256color_icl));
        GX_EndLoadOBJExtPltt();        // Palette data transfer complete
    }

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        int     i, j;
        int     pos_x = base_pos_x;
        int     pos_y = base_pos_y;
        int     palette = base_pal;

        /* Obtain pad information and configure operations */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] =
                { {PAD_BUTTON_A, 10}, {PAD_BUTTON_SELECT, 1}, {PAD_BUTTON_L, 10},
            {PAD_KEY_UP | PAD_KEY_RIGHT, 20}
            };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_PRESS(PAD_KEY_UP))
        {
            --base_pos_y;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_DOWN))
        {
            ++base_pos_y;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
        {
            ++base_pos_x;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_LEFT))
        {
            --base_pos_x;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        if (DEMO_IS_PRESS(PAD_BUTTON_L))
        {
            rotate -= 512;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_BUTTON_R))
        {
            rotate += 512;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        if (DEMO_IS_PRESS(PAD_BUTTON_A))
        {
            scale += 128;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_BUTTON_B))
        {
            scale -= 128;
            if (scale < 0)
            {
                scale = 0;
            }
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        if (DEMO_IS_TRIG(PAD_BUTTON_SELECT))
        {
            if (++base_pal > 15)
            {
                base_pal = 0;
            }
            OS_Printf("base_pal=%d\n", base_pal);
        }

        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
            base_pal = 0;
            base_pos_x = 0;
            base_pos_y = 0;
            rotate = 0;
            scale = FX32_ONE;
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }

        /* Generation and configuration of the affine transformation matrix */
        {
            MtxFx22 mtx;
            fx16    sinVal = FX_SinIdx(rotate);
            fx16    cosVal = FX_CosIdx(rotate);
            fx32    rScale = FX_Inv(scale);
            GXOamAffine *oamBuffp = (GXOamAffine *)&s_OAMBuffer[0];

            mtx._00 = (fx32)((cosVal * rScale) >> FX32_SHIFT);
            mtx._01 = (fx32)((sinVal * rScale) >> FX32_SHIFT);
            mtx._10 = -(fx32)((sinVal * rScale) >> FX32_SHIFT);
            mtx._11 = (fx32)((cosVal * rScale) >> FX32_SHIFT);
            // Set OBJ affine transformation
            G2_SetOBJAffine(oamBuffp,  // OAM buffer pointer
                            &mtx);     // 2x2 matrix for affine transformation
        }

        /* OAM configuration (configure 16 32-dot x 32-dot OBJs) */
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
            {
                int     count = (i * 4) + j;
                int     pattern = ((count / 4) * 0x80) + ((count % 4) * 0x08);

                /* Configure the OAM attributes of the OBJ */
                G2_SetOBJAttr(&s_OAMBuffer[count],      // Specify the location of OAM
                              pos_x,   // x position
                              pos_y,   // y position
                              0,       // Priority order
                              GX_OAM_MODE_NORMAL,       // Normal OBJ
                              FALSE,   // Disable mosaic
                              GX_OAM_EFFECT_AFFINE,     // Affine effect
                              GX_OAM_SHAPE_32x32,       // OBJ size
                              GX_OAM_COLOR_256, // 256 colors
                              pattern, // Character name
                              palette, // Color palette
                              0);
                pos_x += 32;
                if (++palette > 15)
                {
                    palette = 0;
                }
            }
            pos_x = base_pos_x;
            pos_y += 32;
        }

        /* Flush the cache to main memory */
        DC_FlushRange(s_OAMBuffer, sizeof(s_OAMBuffer));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x8B9FC4A0);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        /* V-Blank wait */
        OS_WaitVBlankIntr();

        /* Transfer to OAM */
        GX_LoadOAM(s_OAMBuffer,        // Transfer OAM buffer to OAM
                   0, sizeof(s_OAMBuffer));
        MI_DmaFill32(3,                // Clear OAM buffer
                     s_OAMBuffer, 192, sizeof(s_OAMBuffer));
    }
}
