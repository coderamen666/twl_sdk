/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - threadQueue-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-02#$
  $Rev: 5266 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define STACK_SIZE      1024
#define THREAD1_PRIO    10
#define THREAD2_PRIO    11

OSThread thread1;
OSThread thread2;
OSThreadQueue threadQueue;

u64 stack1[STACK_SIZE / sizeof(u64)];
u64 stack2[STACK_SIZE / sizeof(u64)];

void proc1(void *arg);
void proc2(void *arg);

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    s32     n;

    OS_Init();
    OS_InitThread();

    OS_InitThreadQueue(&threadQueue);

    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, NULL, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD2_PRIO);

    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    for( n=0; n<5; n++ )
    {
        OS_Printf("wakeup\n");
        OS_WakeupThread(&threadQueue);
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused(arg)
    while (1)
    {
        OS_Printf("Thread1\n");
        OS_SleepThread(&threadQueue);
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
        OS_Printf("Thread2\n");
        OS_SleepThread(&threadQueue);
    }
}

/*====== End of main.c ======*/
