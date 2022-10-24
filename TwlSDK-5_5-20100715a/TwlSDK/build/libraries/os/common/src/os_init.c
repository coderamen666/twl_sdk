/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_init.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-20#$
  $Rev: 10999 $
  $Author: yada $

 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/mi/wram.h>
#include <nitro/pxi.h>
#include <nitro/ctrdg/common/ctrdg_common.h>
#include <nitro/card/common.h>

#ifdef	SDK_ARM9
#include <nitro/spi/ARM9/pm.h>
#else //SDK_ARM7
#include <nitro/spi/ARM7/pm.h>
#endif


#ifdef SDK_TWL
#include <application_jump_private.h>
#include <twl/scfg.h>
#endif

//#include <nitro/spi.h>

#ifndef SDK_TEG
//#include <nitro/ctrdg.h>
#endif

#include        <nitro/code32.h>
static asm void OSi_WaitVCount0( void )
{
        //---- set IME = 0
        //     ( use that LSB of HW_REG_BASE equal to 0 )
        mov             r12, #HW_REG_BASE
        ldr             r1,  [r12, #REG_IME_OFFSET]
        str             r12, [r12, #REG_IME_OFFSET]

        //---- adjust VCOUNT.
@wait_vcount_0:
        ldrh            r0, [r12, #REG_VCOUNT_OFFSET]
        cmp             r0, #0
        bne             @wait_vcount_0
        str             r1, [r12, #REG_IME_OFFSET]
        bx              lr
}
#include         <nitro/codereset.h>

#if defined(SDK_ARM9) && defined(SDK_TWLLTD)
#include <os_attention.h>
#endif


/*---------------------------------------------------------------------------*
 * Function
 *---------------------------------------------------------------------------*/
void OSi_InitCommon(void);


/*---------------------------------------------------------------------------*
  Name:         OS_Init

  Description:  initialize sdk os

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void OS_Init(void)
{
#ifdef SDK_ARM9
    //---- system shared area check
    SDK_ASSERT((u32)&(OS_GetSystemWork()->command_area) == HW_CMD_AREA);
#endif  // SDK_ARM9

    //---- Initialize sdk os common
    OSi_InitCommon();
}
#pragma profile reset


/*---------------------------------------------------------------------------*
  Name:         OSi_InitCommon

  Description:  initialize sdk os common

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void OSi_InitCommon(void)
{
#ifdef SDK_ARM9
    //----------------------------------------------------------------
    // for ARM9

#ifdef SDK_ENABLE_ARM7_PRINT
    // Init PrintServer for ARM7 (if specified)
    OS_InitPrintServer();
#endif

    //---- Init interProcessor I/F
    //  Sync with ARM7 to enable OS_GetConsoleType()
    //  PXI_Init() must be called before OS_InitArenaEx()
    PXI_Init();

    //---- Init Arena (arenas except SUBPRIV-WRAM)
    OS_InitArena();

    //---- Init Spinlock
    OS_InitLock();

    //---- Init Arena (extended main)
    OS_InitArenaEx();

    //---- Init IRQ Table
    OS_InitIrqTable();

    //---- Init IRQ Stack checker
    OS_SetIrqStackChecker();

    //---- Init Exception System
    OS_InitException();

    //---- Init MI (Wram bank and DMA0 arranged)
    MI_Init();

    //---- Init VCountAlarm
    OS_InitVAlarm();

    //---- Init VRAM exclusive System
    OSi_InitVramExclusive();

    //---- Init Thread System
#ifndef SDK_NO_THREAD
    OS_InitThread();
#endif
    //---- Init Reset System
#ifndef SDK_SMALL_BUILD
    OS_InitReset();
#endif

    //---- Init Cartridge
    CTRDG_Init();

    //---- Init Card
#ifndef SDK_SMALL_BUILD
    CARD_Init();
#endif

#ifdef SDK_TWL
	MI_InitWramManager();
#endif

	//---- init System config
#ifdef SDK_TWL
	if (OS_IsRunOnTwl() == TRUE)
	{
		SCFG_Init();
	}
#endif

    //---- Init Power Manager
    PM_Init();

    //---- adjust VCOUNT
    OSi_WaitVCount0();

#ifdef SDK_TWL
	if (OS_IsRunOnTwl() == TRUE)
	{
		OSi_InitPrevTitleId();
	}
#endif

#else  // SDK_ARM9
    //----------------------------------------------------------------
    // for ARM7

    //---- Init interProcessor I/F
    PXI_Init();

    //---- Init Arena (SUBPRIV-WRAM arena)
    OS_InitArena();

    //---- Init Spinlock
    OS_InitLock();

    //---- Init IRQ Table
    OS_InitIrqTable();

    //---- Init Exception System
    OS_InitException();

    //---- Init MI
#ifdef SDK_TWL
    MI_Init();
#endif

    //---- Init Tick
    OS_InitTick();

    //---- Init Alarm System
    OS_InitAlarm();

    //---- Init Thread System
    OS_InitThread();

    //---- Init Reset System
#ifndef SDK_SMALL_BUILD
    OS_InitReset();
#endif

    //---- Init Cartridge
#ifndef SDK_TWLLTD
    CTRDG_Init();
#endif

#ifdef SDK_TWL
	MI_InitWramManager();
#endif

	//---- init System config
#ifdef SDK_TWL
	if (OS_IsRunOnTwl() == TRUE)
	{
		SCFG_Init();
	}
#endif

#endif // SDK_ARM9
}
#pragma profile reset
