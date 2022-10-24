/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_Direct
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
// A sample that uses a 2D mapped bitmap OBJ
// 
// This sample displays objects on the display in 2D mapped bitmap format
// 
// USAGE:
//   UP, DOWN, LEFT, RIGHT      : Slide the OBJ
//   A, B                       : Scaling the OBJ
//   L, R                       : Rotate the OBJ
//   START      : Initialize status of the OBJ
// 
// HOWTO:
// 1. Set VRAM size and mapping mode of OBJ by GX_SetOBJVRamModeBmp
// 2. Transfer the bitmap data by GX_LoadOBJ
// 3. Set GX_OAM_MODE_BITMAPOBJ to OAM attribute to use the bitmap OBJ
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
//        VBlank interrupt process
//  Description:
//        Enables a flag that confirms that a V-Blank interrupt has been performed.
//
//        The following steps will be performed during common initialization (DEMOInitCommon), causing this function to be called during VBlanks
//        * Select IRQ interrupt (OS_SetIrqMask)
//        * Register this function, which performs IRQ interrupts (OS_SetIRQFunction)
//        * Enable IRQ interrupts (OS_EnableIrq)
//        
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Sets a VBlank interrupt confirmation flag
}

//---------------------------------------------------------------------------
//  Summary:
//        Displays 2D mapping bitmap OBJ
//  Description:
//        Affine conversion (rotate/scale) of the OBJ that holds the 2-dimensional mapping bitmap data and displays it
//      
//  Controls:
//        UP, DOWN: Manipulate display position
//        BUTTON_A, B : Enlarge, reduce
//        BUTTON_L, R : rotation
//        START      : Initialize settings
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     base_pos_x = 0, base_pos_y = 0;
    int     base_pat = 0;
    fx32    scale = FX32_ONE;
    u16     rotate = 0;

    //---------------------
    // Initialization
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    /* 2D configuration */
    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);       // Allocates VRAM-A, B to OBJ

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // 2D/3D mode
                       GX_BGMODE_0,    // Set BGMODE to 0
                       GX_BG0_AS_2D);  // Use BG 0 as 2D

    GX_SetVisiblePlane(GX_PLANEMASK_OBJ);       // Sets OBJ to visible
    // Sets OBJ-VRAM capacity and mapping mode for bitmap OBJ
    GX_SetOBJVRamModeBmp(GX_OBJVRAMMODE_BMP_2D_W256_32K);       // 2D mapping

    /* Load character data */
    GX_LoadOBJ((const void *)IMAGE_DATA,        // Transfer OBJ data to VRAM
               0, IMAGE_DATA_SIZE);

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        int     i, j;
        int     pos_x = base_pos_x;
        int     pos_y = base_pos_y;

        /* Obtain pad information and configure operations */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] =
                { {PAD_BUTTON_A, 10}, {PAD_BUTTON_L, 10}, {PAD_KEY_DOWN | PAD_KEY_RIGHT, 20}, {0,
                                                                                               0}
            };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_PRESS(PAD_KEY_UP))
        {
            if (--base_pos_y < 0)
            {
                base_pos_y = 255;
            }
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_DOWN))
        {
            if (++base_pos_y > 255)
            {
                base_pos_y = 0;
            }
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        if (DEMO_IS_PRESS(PAD_KEY_RIGHT))
        {
            if (++base_pos_x > 511)
            {
                base_pos_x = 0;
            }
            OS_Printf("pos=%d:%d rotate=%d scale=%d\n", base_pos_x, base_pos_y, rotate, scale);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_LEFT))
        {
            if (--base_pos_x < 0)
            {
                base_pos_x = 511;
            }
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

        if (DEMO_IS_TRIG(PAD_BUTTON_START))
        {
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

        /* OAM configuration (configures 16 OBJs) */
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
            {
                int     count = (i * 4) + j;
                int     pattern = ((count / 4) * 0x80) + ((count % 4) * 0x08);

                G2_SetOBJAttr(&s_OAMBuffer[count],      // Specify the location of OAM
                              pos_x,   // x position
                              pos_y,   // y position
                              0,       // Priority order
                              GX_OAM_MODE_BITMAPOBJ,    // Bitmap mode
                              FALSE,   // Disable mosaic
                              GX_OAM_EFFECT_AFFINE,     // Affine effect
                              GX_OAM_SHAPE_32x32,       // OBJ size
                              GX_OAM_COLOR_16,  // 16 colors
                              pattern, // Character name
                              15,      // Alpha value
                              0);
                pos_x += 32;
            }
            pos_x = base_pos_x;
            pos_y += 32;
        }

#ifdef SDK_AUTOTEST                    // Code for auto-test
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xA407A660);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        /* Flush the cache to main memory */
        DC_FlushRange(s_OAMBuffer, sizeof(s_OAMBuffer));
        /* I/O register is accessed using DMA operation, so cache wait is not needed */
        // DC_WaitWriteBufferEmpty();

        /* V-Blank wait */
        OS_WaitVBlankIntr();

        /* Transfer to OAM */
        GX_LoadOAM(s_OAMBuffer,        // Transfer OAM buffer to OAM
                   0, sizeof(s_OAMBuffer));
        MI_DmaFill32(3,                // Clear OAM buffer
                     s_OAMBuffer, 192, sizeof(s_OAMBuffer));
    }
}
