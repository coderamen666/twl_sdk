/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - thread-2
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-02#$
  $Rev: 5266 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define STACK_SIZE     1024
#define THREAD1_PRIO   1
#define THREAD2_PRIO   2

OSThread thread1;
OSThread thread2;

u32 stack1[STACK_SIZE / sizeof(u32)];
u32 stack2[STACK_SIZE / sizeof(u32)];

void VBlankIntr(void);
void proc1(void *arg);
void proc2(void *arg);

//================================================================================
//      Sample of thread switch using interrupt
//      Starts a thread from within the IRQ handler.
//      However, the actual thread switching is postponed until the IRQ routine ends.
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    s32     n;

    OS_Init();
    OS_InitThread();

    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, NULL, stack2 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD2_PRIO);

    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    //================ Settings
#ifdef SDK_ARM9
    //---- Initialize graphics
    GX_Init();
#endif // SDK_ARM9

    //----  Enable V-Blank interrupt
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    //---- V-Blank occurrence settings
    (void)GX_VBlankIntr(TRUE);

    //================ Main loop
    for (n = 0; n < 5; n++)
    {
        //---- Wait for V-Blank interrupt completion
        OS_WaitVBlankIntr();

        OS_Printf("main\n");
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt processing(processing in IRQ)
//
void VBlankIntr(void)
{
    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);

    //---- Start thread
    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused(arg)
    while (1)
    {
        OS_Printf("---- Thread1 sleep\n");
        OS_SleepThread(NULL);
        OS_Printf("---- Thread1 wakeup\n");
    }
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#pragma unused(arg)
    while (1)
    {
        OS_Printf("---- Thread2 sleep\n");
        OS_SleepThread(NULL);
        OS_Printf("---- Thread2 wakeup\n");
    }
}

/*====== End of main.c ======*/
