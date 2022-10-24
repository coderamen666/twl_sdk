/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - fnd - archive2
  File:     sdk_init.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "sdk_init.h"

#define	DEFAULT_DMA_NUMBER		MI_DMA_MAX_NUM


/* -------------------------------------------------------------------------
	V-blank callback
   ------------------------------------------------------------------------- */
void
VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/*---------------------------------------------------------------------------*
  Name:         InitSystem

  Description:  Initializes NITRO.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
InitSystem(void)
{
    OS_Init();
    FX_Init();
	GX_SetPower(GX_POWER_ALL);
    GX_Init();

    GX_DispOff();
    GXS_DispOff();

	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);

    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();

	FS_Init(DEFAULT_DMA_NUMBER);

    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request
}

/*---------------------------------------------------------------------------*
  Name:         InitVRAM

  Description:  Performs the initialization of VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
InitVRAM(void)
{
    // Assign all banks to LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    // Clear entire LCDC space
    MI_CpuClearFast( (void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    
    // Disable bank assigned to LCDC
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast ((void*)HW_OAM    , 192, HW_OAM_SIZE   );  // Clear OAM
    MI_CpuFillFast ((void*)HW_DB_OAM , 192, HW_DB_OAM_SIZE);  // Clear OAM

    MI_CpuClearFast((void*)HW_PLTT   , HW_PLTT_SIZE   );      // Clear palette
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);	  // Clear the standard palette
}

/*---------------------------------------------------------------------------*
  Name:         InitDisplay

  Description:  Display only BG0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
InitDisplay(void)
{
    GX_SetBankForBG (GX_VRAM_BG_128_A );                // Allocate bank A to BG

	GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);
	GX_SetBGScrOffset ( GX_BGSCROFFSET_0x10000);

    GX_SetGraphicsMode(
    	GX_DISPMODE_GRAPHICS,       					// Set to graphics display mode
        GX_BGMODE_0,                					// Set BGMODE to 0
        GX_BG0_AS_2D);            						// Set BG0 to 2D display

    GX_SetVisiblePlane(
    	GX_PLANEMASK_BG0								// Display BG0
    );

	G2_SetBG0Control(
   		GX_BG_SCRSIZE_TEXT_256x256,           			// 256 x 256 dots
        GX_BG_COLORMODE_256,                  			// 256-color mode
        GX_BG_SCRBASE_0x0000,                 			// Screen base
        GX_BG_CHARBASE_0x00000,               			// Character base.
        GX_BG_EXTPLTT_01                      			// BGExtPltt slot.
    );

    GX_DispOn();
}
