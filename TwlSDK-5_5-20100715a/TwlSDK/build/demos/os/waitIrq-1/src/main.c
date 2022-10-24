/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - waitIrq-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define  STACK_SIZE     1024
#define  THREAD1_PRIO   17

OSThread thread1;
u32      stack1[STACK_SIZE / sizeof(u32)];

void VBlankIntr(void);
void proc1(void *arg);

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    s32     n;

    OS_Init();
    OS_InitThread();

    //---- create thread1
    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

    //---- VBlank setting
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    //---- main loop
    for (n = 0; n < 5; n++)
    {
        // while waiting vblank, lower priority thread runs and display 'X'.
        // if vblank interrupt occurred, return here immidiately.

        OS_Printf("\n[loop %d]", n);
        OS_WaitIrq(TRUE, OS_IE_V_BLANK);
    }

    OS_Printf("\n");
    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt handler
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused(arg)
    int     i = 0;

    while(1)
    {
        if ( ((i++) & 0xff) == 0 )
        {
            (void)OS_DisableInterrupts();
            OS_Printf("X");
            (void)OS_EnableInterrupts();
        }
    }
}

/*====== End of main.c ======*/
