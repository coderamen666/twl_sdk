/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - exceptionDisplay-3
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "font.h"
#include "screen.h"

static void myInit(void);
static void myVBlankIntr(void);
static void myExceptionCallback(u32 context, void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void NitroMain()
{
	myInit();

	OS_EnableUserExceptionHandlerOnDebugger();
    OS_SetUserExceptionHandler(myExceptionCallback, (void *)0);
    DC_FlushAll();
    DC_WaitWriteBufferEmpty();

    while (TRUE)
    {
		u16 button = PAD_Read();

        //---- clear screen buffer
        ClearScreen();
		PrintString( 2, 1, 15, "PUSH [A] to cause EXCEPTION");

		if ( button & PAD_BUTTON_A )
		{
			//---- force to occur exception
			OS_Printf("now force to occur exception...\n");
			asm
			{
				/* *INDENT-OFF* */
				ldr      r0, =0
				ldr      r1, =0x12345678
#ifdef SDK_ARM9
				ldr      r2, [r0,#0]
#else
				dcd      0x06000010
#endif
				/* *INDENT-ON* */
			}
			OS_Printf("not occurred exception.\n");
		}

        //---- wait vblank
		OS_WaitVBlankIntr();
	}
}

/*---------------------------------------------------------------------------*
  Name:         myInit

  Description:  initialize

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void myInit(void)
{
    //---- init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    //---- init displaying
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- setting 2D for top screen
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



    //---- setting 2D for bottom screen
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


    //---- screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    //---- init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	//---- FileSytem init
	FS_Init(FS_DMA_NOT_USE);

    //---- start displaying
    GX_DispOn();
    GXS_DispOn();
}

/*---------------------------------------------------------------------------*
  Name:         myVBlankIntr

  Description:  vblank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void myVBlankIntr(void)
{
    //---- upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         myExceptionCallback

  Description:  user callback for exception

  Arguments:    context :
                arg     :

  Returns:      None
 *---------------------------------------------------------------------------*/
void myExceptionCallback(u32 context, void *arg)
{
#pragma unused( arg )

	ClearScreen();
    PrintString( 0, 0, 15, "**** Exception Occurred ****");
	{
		int i;
		OSExcpContext *c = (OSExcpContext*)context;

		for (i = 0; i < 13; i++)
		{
			PrintString( (s16)((i&1)?16:1), (s16)((i/2)+1), 15, "R%02d=%08X", i, c->context.r[i] ); 
		}

		PrintString(16, 7, 15, "SP =%08X", c->context.sp);
		PrintString( 1, 8, 15, "LR =%08X", c->context.lr);
		PrintString(16, 8, 15, "PC =%08X", c->context.pc_plus4);

		PrintString( 0, 9, 15, "CPSR=%08X  SPSR=%08X", c->context.cpsr, c->spsr);
		PrintString( 0, 10, 15, "CP15=%08X",c->cp15);
	}

    DC_FlushRange(gScreen, sizeof(gScreen));
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

	while(1)
	{
		// do nothing
	}
}

/*====== End of main.c ======*/
