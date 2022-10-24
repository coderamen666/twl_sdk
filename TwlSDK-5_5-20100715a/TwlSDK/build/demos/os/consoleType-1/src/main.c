/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - consoleType-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  Revision 1.23  2006/01/18 02:11:29  kitase_hirotake
  do-indent

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include  <nitro.h>
#include  "font.h"
#include  "screen.h"

static void myInit(void);
static void myVBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    //---------------- Initialize
    myInit();

    //---------------- Main loop
    while (TRUE)
    {
        //---- Clear screen buffer
        ClearScreen();

        //---- Display info
		PrintString( 2,  1, 15, "OS_GetConsoleType");
		PrintString(22,  3, 15, "%08x", OS_GetConsoleType() );

		PrintString( 2,  5, 15, "OS_GetRunningConsoleType" );
		PrintString(22,  7, 15, "%08x", OS_GetRunningConsoleType() );

		PrintString( 2, 12, 15, "OS_IsRunOnDebugger" );
		PrintString(24, 12, 15, "%s", OS_IsRunOnDebugger()? "TRUE": "FALSE" );

		PrintString( 2, 14, 15, "OS_IsRunOnEmulator" );
		PrintString(24, 14, 15, "%s", OS_IsRunOnEmulator()? "TRUE": "FALSE" );

		PrintString( 2, 16, 15, "OS_IsRunOnTwl" );
		PrintString(24, 16, 15, "%s", OS_IsRunOnTwl()? "TRUE": "FALSE" );

		PrintString( 2, 18, 15, "OSi_IsNitroModeOnTwl" );
		PrintString(24, 18, 15, "%s", OSi_IsNitroModeOnTwl()? "TRUE": "FALSE" );

        //---- wait vblank
		OS_WaitVBlankIntr();

		{
			static int n=0;
			if ( n++ > 5 )
			{
				break;
			}
		}
	}

	OS_Printf("***End of demo\n");
	OS_Terminate();
}

//----------------------------------------------------------------
//  myInit
//
void myInit(void)
{
    //---- Init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    //---- Display init
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- Setting 2D for top screen
    GX_SetBankForBG(GX_VRAM_BG_128_A);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));



    //---- Setting 2D for bottom screen
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                      GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);

    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));


    //---- Screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    //---- Init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	//---- FileSytem init
	FS_Init(FS_DMA_NOT_USE);

    //---- Start displaying
    GX_DispOn();
    GXS_DispOn();
}

//----------------------------------------------------------------
//  myVBlankIntr
//             V-Blank interrupt handler
//
static void myVBlankIntr(void)
{
    //---- Upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
