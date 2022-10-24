/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_Oam_Translucent
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-30#$
  $Rev: 9157 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// A sample that uses OBJ with alpha blending effect
// 
// This sample displays two spheres
// One of the spheres is transparent
// 
// USAGE:
//   UP, DOWN   : Change level of transparency
// 
// HOWTO:
// 1. Set GX_OAM_MODE_XLU to OAM attribute to use transparent OBJ
// 2. Set transparent object with G2_SetBlendAlpha
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
//        Display translucent OBJs
//  Description:
//        Displays translucent OBJ characters
//        OBJs displayed on the left are displayed normally. OBJs displayed on the right are translucent.
//      OBJ
//  Controls:
//        UP, DOWN: Blending factor of the translucent OBJ can be changed
//---------------------------------------------------------------------------
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     eva = 12;

    //---------------------
    // Initialization
    //---------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayOBJOnly();

    /* Load character data and palette data */
    GX_LoadOBJ(d_64_256_obj_schDT, 0, sizeof(d_64_256_obj_schDT));
    GX_LoadOBJPltt(d_64_256_obj_sclDT, 0, sizeof(d_64_256_obj_sclDT));

    DEMOStartDisplay();

    //---------------------
    //  Main loop
    //---------------------
    while (1)
    {
        /* Load pad information */
        DEMOReadKey();
#ifdef SDK_AUTOTEST                    // Code for auto-test
        {
            const EXTKeys keys[8] = { {PAD_KEY_DOWN, 5}, {0, 0} };
            EXT_AutoKeys(keys, &gKeyWork.press, &gKeyWork.trigger);
        }
#endif

        if (DEMO_IS_PRESS(PAD_KEY_UP))
        {
            if (++eva > 0x10)
            {
                eva = 0x10;
            }
            OS_Printf("eva=%d\n", eva);
        }
        else if (DEMO_IS_PRESS(PAD_KEY_DOWN))
        {
            if (--eva < 0x00)
            {
                eva = 0x00;
            }
            OS_Printf("eva=%d\n", eva);
        }

        /* Configure the normal OAM attributes of the OBJ */
        G2_SetOBJAttr(&s_OAMBuffer[0], // Use the first OAM
                      0,               // x position
                      0,               // y position
                      0,               // Priority order
                      GX_OAM_MODE_NORMAL,       // Normal OBJ
                      FALSE,           // Disable mosaic
                      GX_OAM_EFFECT_NONE,       // No special effects
                      GX_OAM_SHAPE_64x64,       // OBJ size
                      GX_OAM_COLOR_256, // 256 colors
                      0,               // Character name
                      0, 0);

        /* Configure the OAM attributes of the translucent OBJ */
        G2_SetOBJAttr(&s_OAMBuffer[1], // Use 2nd OAM
                      64,              // x position
                      0,               // y position
                      0,               // Priority order
                      GX_OAM_MODE_XLU, // Translucent OBJ
                      FALSE,           // Disable mosaic
                      GX_OAM_EFFECT_NONE,       // No special effects
                      GX_OAM_SHAPE_64x64,       // OBJ size
                      GX_OAM_COLOR_256, // 256 colors
                      0,               // Character name
                      0, 0);
        /* Alpha blending settings */
        G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE,       // First target plane not specified
                         GX_BLEND_PLANEMASK_BD, // Second target plane uses backdrop
                         eva,          // Set blending factor for first plane
                         0);           // 

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x80C7021F);
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
