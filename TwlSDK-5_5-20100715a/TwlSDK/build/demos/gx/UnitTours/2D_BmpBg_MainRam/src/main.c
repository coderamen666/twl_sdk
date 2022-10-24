/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_BmpBg_MainRam
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
// A sample that displays the image on the main memory by main memory display mode:
//
// If display mode is main memory display mode, a bitmap image on
// the main memory is displayed through the specialized FIFO.
// This sample loads different four images to the main memory,
// and displays them by turns.
//
// HOWTO:
// 1. Switch to main memory display mode by GX_SetGraphicsMode(GX_DISPMODE_MMEM, ...).
// 2. Transfer images on the main memory by MI_DispMemDmaCopy() every V-Blank.
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
    int     count = 0;
    int     slot = 0;

    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, and initialize VRAM.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    GX_SetGraphicsMode(GX_DISPMODE_MMEM,        // display main memory
                       (GXBGMode)0,    // dummy
                       (GXBG0As)0);    // dummy

    DEMOStartDisplay();

#ifdef  SDK_AUTOTEST
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
#endif // SDK_AUTOTEST

    //-------------------------------------------------------------------------
    // Give the main memory access priority to ARM9.
    // In MI_DispMemDmaCopy, ARM9 should send a lot of pixel to VRAM via DMA
    // at highest priority (higher than ARM7 sound DMA).
    // This may be a risk that sound noise occurs.
    //---------------------------------------------------------------------------
    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);

    while (1)
    {
        if (count++ > 90)
        {
            slot++;
            slot &= 0x03;
            count = 0;
        }

#ifdef SDK_AUTOTEST
        EXT_TestScreenShot(100, 0x05230657);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt

        //---------------------------------------------------------------------------
        // Transfer images from main memory to VRAM through the specialized FIFO by DMA.
        // It must be completed within a period of V-Blank.
        //---------------------------------------------------------------------------
        MI_DispMemDmaCopy(3, IMAGE_MRAM256x192[slot]);
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
