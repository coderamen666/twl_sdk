/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - thread-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-02#$
  $Rev: 5265 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define	STACK_SIZE	1024

void    proc1(void *arg);
void    proc2(void *arg);

OSThread thread1;
OSThread thread2;

u64     stack1[STACK_SIZE / sizeof(u64)];
u64     stack2[STACK_SIZE / sizeof(u64)];

#define		THREAD1_PRIO	10
#define		THREAD2_PRIO	11


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

    OS_CreateThread(&thread1, proc1, (void *)0x111, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, (void *)0x222, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD2_PRIO);

    OS_DumpThreadList();

    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    for (n = 0; n < 5; n++)
    {
        OS_Printf("Idle\n");
        OS_WakeupThreadDirect(&thread2);
    }

    OS_Printf("==== Finish sample.\n");

    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    while (1)
    {
        OS_Printf("Thread1 (%x)\n", (u32)arg);
        OS_SleepThread(NULL);
    }
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( arg )
#endif
    while (1)
    {
        OS_Printf("Thread2 (%x)\n", (u32)arg);
        OS_SleepThread(NULL);
        OS_WakeupThreadDirect(&thread1);
    }
}

/*====== End of main.c ======*/
