/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/2D_CharBg_BankEx
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
// A sample that uses separated BG-VRAM with GX_SetBankForBGEx:
//
// HOWTO:
// 1. Set VRAM Bank for BG with GX_SetBankForBGEx
// 2. Set BG Screen offset with GX_SetBGScrOffset to 0x00000
// 3. Set Char offset with GX_SetBGCharOffset to 0x20000
//
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMO.h"
#include "data.h"

static u16 sScrnBuf[SCREEN_SIZE];      // Buffer for screen data (BG #0)

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    //---------------------------------------------------------------------------
    // Initialize:
    // Enables IRQ interrupts, initializes VRAM, and sets BG #0 for text mode
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    GX_SetBankForBGEx(GX_VRAM_BG_64_E, GX_VRAM_BG_128_A);       // VRAM-E, VRAM-A for BGs

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // Graphics mode
                       GX_BGMODE_0,    // BGMODE is 0
                       GX_BG0_AS_2D);  // BG #0 is for 2D

    GX_SetVisiblePlane(GX_PLANEMASK_BG0);       // Display only BG #0

    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x20000);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,        // 256pix x 256pix text
                     GX_BG_COLORMODE_256,       // use 256 color mode
                     GX_BG_SCRBASE_0x0000,      // screen base offset + 0x0000 is the address for BG #0 screen
                     GX_BG_CHARBASE_0x00000,    // character base offset + 0x04000 is the address for BG #0 characters
                     GX_BG_EXTPLTT_01  // use BGExtPltt slot #0 if BGExtPltt is enabled
        );

    G2_SetBG0Priority(0);

    G2_BG0Mosaic(FALSE);

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    GX_LoadBG0Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GX_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));

    {
        int     i, j;
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                sScrnBuf[(i * 32) + j] = (u16)((i * 0x10) + j);
            }
        }
    }
    // Store the data in the main memory, and invalidate the cache
    DC_FlushRange(sScrnBuf, sizeof(sScrnBuf));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();

    // DMA transfer to BG #0 screen
    GX_LoadBG0Scr(sScrnBuf, 0, sizeof(sScrnBuf));

    DEMOStartDisplay();

    //---------------------------------------------------------------------------
    // Main Loop
    //---------------------------------------------------------------------------
    while (1)
    {
#ifdef SDK_AUTOTEST
        GX_SetBankForLCDC(GX_VRAM_LCDC_C);
        EXT_TestSetVRAMForScreenShot(GX_VRAM_LCDC_C);
        EXT_TestScreenShot(100, 0xC7953680);
        EXT_TestTickCounter();
#endif //SDK_AUTOTEST

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
        GX_LoadBG0Scr(sScrnBuf, 0, sizeof(sScrnBuf));
    }
}

//---------------------------------------------------------------------------
// V-Blank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call OS_SetIrqCheckFlag to check a V-Blank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}
