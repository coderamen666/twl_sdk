/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - yield-1
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

/* ***************************************************************
   --- Test for OS_YieldThread() ---

  If not use OS_YieldThread(), the result will be:
     Thread 3
     Thread 2
     Thread 1
     Thread 3
     Thread 2
     Thread 1
        :

   If use OS_YieldThread(), the result will be:
     Thread 3
     Thread 2
     Thread 3
     Thread 2
     Thread 3
     Thread 2
        :

  Please try it.
 * ****************************************************************/

//================================================================================
#define	STACK_SIZE	   1024
#define	THREAD1_PRIO   20
#define	THREAD2_PRIO   20
#define	THREAD3_PRIO   20

OSThread thread1;
OSThread thread2;
OSThread thread3;
OSThread* launcherThread;

u64 stack1[STACK_SIZE / sizeof(u64)];
u64 stack2[STACK_SIZE / sizeof(u64)];
u64 stack3[STACK_SIZE / sizeof(u64)];

int cnt;

void startTest(void);
void stopTest(void);
void proc1(void *arg);
void proc2(void *arg);
void proc3(void *arg);

BOOL useYield;

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    OS_InitThread();

	launcherThread = OS_GetCurrentThread();

	//================ TEST1
	OS_Printf("================TEST1: use OS_YieldThread()\n");
	cnt = 0;
	useYield = TRUE;
	startTest();
	OS_SleepThread(NULL);
	stopTest();

	//================ TEST2
	OS_Printf("================TEST2: not use OS_YieldThread()\n");
	cnt = 0;
	useYield = FALSE;
	startTest();
	OS_SleepThread(NULL);
	stopTest();

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    startTest
//
void startTest(void)
{
    OS_CreateThread(&thread1, proc1, (void *)0, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, (void *)0, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD2_PRIO);
    OS_CreateThread(&thread3, proc3, (void *)0, stack3 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD3_PRIO);

    OS_DumpThreadList();

    (void)OS_DisableScheduler();
    {
        OS_WakeupThreadDirect(&thread1);
        OS_WakeupThreadDirect(&thread2);
        OS_WakeupThreadDirect(&thread3);
    }
    (void)OS_EnableScheduler();
    OS_RescheduleThread();
}

//--------------------------------------------------------------------------------
//    stopTest
//
void stopTest(void)
{
    (void)OS_DisableScheduler();
	{
		OS_DestroyThread(&thread1);
		OS_DestroyThread(&thread2);
		OS_DestroyThread(&thread3);
	}
    (void)OS_EnableScheduler();
    OS_RescheduleThread();
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused( arg )
    while (cnt<10)
    {
        OS_Printf("Thread 1\n");

		if ( useYield )
		{
			OS_YieldThread();
		}
		else
		{
			(void)OS_DisableScheduler();
			{
				OS_WakeupThreadDirect(&thread2);
				OS_WakeupThreadDirect(&thread3);
			}
			(void)OS_EnableScheduler();
			OS_SleepThread(NULL);
		}
		cnt++;
    }
	OS_WakeupThreadDirect(launcherThread);
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#pragma unused( arg )
    while (cnt<10)
    {
        OS_Printf("Thread 2\n");

		if ( useYield )
		{
			OS_YieldThread();
		}
		else
		{
			(void)OS_DisableScheduler();
			{
				OS_WakeupThreadDirect(&thread1);
				OS_WakeupThreadDirect(&thread3);
			}
			(void)OS_EnableScheduler();
			OS_SleepThread(NULL);
		}
		cnt++;
    }
	OS_WakeupThreadDirect(launcherThread);
}

//--------------------------------------------------------------------------------
//    proc3
//
void proc3(void *arg)
{
#pragma unused( arg )
    while (cnt<10)
    {
        OS_Printf("Thread 3\n");

		if ( useYield )
		{
			OS_YieldThread();
		}
		else
		{
			(void)OS_DisableScheduler();
			{
				OS_WakeupThreadDirect(&thread1);
				OS_WakeupThreadDirect(&thread2);
			}
			(void)OS_EnableScheduler();
			OS_SleepThread(NULL);
		}
		cnt++;
    }
	OS_WakeupThreadDirect(launcherThread);
}

/*====== End of main.c ======*/
