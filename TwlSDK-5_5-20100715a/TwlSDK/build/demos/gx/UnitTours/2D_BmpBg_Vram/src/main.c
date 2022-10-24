/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_BmpBg_Vram
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
// A sample that displays VRAM image:
//
// If display mode is VRAM display mode, a bitmap image on VRAM is displayed.
// This sample loads different four images to VRAM A-D, and displays them by turns.
//
// HOWTO:
// 1. Set the target VRAM banks to LCDC mode.
// 2. Load images to them (by MI_DmaCopy32()).
// 3. Start to display the VRAM image by GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, ...).
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "image.h"

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    int     vram_slot = 0, count = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, and initialize VRAM.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    //---------------------------------------------------------------------------
    // Map VRAM bank A-D onto LCDC.
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_A | GX_VRAM_LCDC_B | GX_VRAM_LCDC_C | GX_VRAM_LCDC_D);

    //---------------------------------------------------------------------------
    // Download images
    //---------------------------------------------------------------------------
    MI_DmaCopy32(3, IMAGE_VRAM256x192[0], (void *)HW_LCDC_VRAM_A,
                 256 * 192 * sizeof(unsigned short));
    MI_DmaCopy32(3, IMAGE_VRAM256x192[1], (void *)HW_LCDC_VRAM_B,
                 256 * 192 * sizeof(unsigned short));
    MI_DmaCopy32(3, IMAGE_VRAM256x192[2], (void *)HW_LCDC_VRAM_C,
                 256 * 192 * sizeof(unsigned short));
    MI_DmaCopy32(3, IMAGE_VRAM256x192[3], (void *)HW_LCDC_VRAM_D,
                 256 * 192 * sizeof(unsigned short));

    //---------------------------------------------------------------------------
    // Set graphics mode VRAM display mode
    //---------------------------------------------------------------------------
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A,      // display VRAM-A
                       (GXBGMode)0,    // dummy
                       (GXBG0As)0);    // dummy

    DEMOStartDisplay();
    while (1)
    {

#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0x05230657);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt

        //---------------------------------------------------------------------------
        // Change the VRAM slot displayed every 90 frames
        //---------------------------------------------------------------------------
        if (count++ > 90)
        {
            vram_slot++;
            vram_slot &= 0x03;
            switch (vram_slot)
            {
            case 0:
                GX_SetGraphicsMode(GX_DISPMODE_VRAM_A,  // display VRAM-A
                                   (GXBGMode)0, // dummy
                                   (GXBG0As)0); // dummy
                break;
            case 1:
                GX_SetGraphicsMode(GX_DISPMODE_VRAM_B,  // display VRAM-B
                                   (GXBGMode)0, // dummy
                                   (GXBG0As)0); // dummy
                break;
            case 2:
                GX_SetGraphicsMode(GX_DISPMODE_VRAM_C,  // display VRAM-C
                                   (GXBGMode)0, // dummy
                                   (GXBG0As)0); // dummy
                break;
            case 3:
                GX_SetGraphicsMode(GX_DISPMODE_VRAM_D,  // display VRAM-D
                                   (GXBGMode)0, // dummy
                                   (GXBG0As)0); // dummy
                break;
            }
            // reset a counter
            count = 0;
        }
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
