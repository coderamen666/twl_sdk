/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - waitIrq-2
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define    STACK_SIZE     1024
#define    THREAD1_PRIO   17
#define    THREAD2_PRIO   18

typedef enum 
{
	WAIT_VBLANK,
	WAIT_ANY
} WaitType;

OSThread thread1;
OSThread thread2;
u32      stack1[STACK_SIZE / sizeof(u32)];
u32      stack2[STACK_SIZE / sizeof(u32)];

OSMessage      mesgBuffer[1];
OSMessageQueue mesgQueue;

WaitType waitType;

void startThreads(WaitType t);
void VBlankIntr(void);
void proc1(void *arg);

//================================================================================
// Description:
//
// Creates two threads from the running thread. (We'll call these thread 1 and thread 2.)
//
// In test 1, the two threads wait for a V-Blank interrupt.
// Internally, they wait using the IRQ thread queue.
// Both threads enter the executable state when the V-Blank interrupt occurs, but only one of the threads will actually run because the OS_WaitIrq function also checks the interrupt flag.
// 
// 
//
// In test 2, two threads wait on a V-Blank interrupt in the same way, but because the OS_WaitAnyIrq function just starts the threads registered in the IRQ thread queue, both threads will run.
// 
// 
//
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();
    OS_InitThread();

	//---- Message queue setting
    OS_InitMessageQueue(&mesgQueue, &mesgBuffer[0], 1);

    //---- V-Blank interrupt setting
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

	//======== TEST1
	OS_Printf("==== TEST1: WAIT_VBLANK\n");
	startThreads( WAIT_VBLANK );

	//======== TEST2
	OS_Printf("==== TEST2: WAIT_ANY\n");
	startThreads( WAIT_ANY );

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    Start test threads
//
void startThreads(WaitType t)
{
	waitType = t;

    OS_CreateThread(&thread1, proc1, (void*)1, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_CreateThread(&thread2, proc1, (void*)2, stack2 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD2_PRIO);
    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);
	(void)OS_ReceiveMessage(&mesgQueue, NULL, OS_MESSAGE_BLOCK);
	OS_DestroyThread(&thread1);
	OS_DestroyThread(&thread2);
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt handler
//
void VBlankIntr(void)
{
	OS_Printf("VBLANK\n");
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
	int threadNo = (int)arg;
	int n;
    for( n=0; n<5; n++ )
    {
		switch( waitType )
		{
			case WAIT_VBLANK:
				OS_WaitIrq(TRUE, OS_IE_V_BLANK);
				break;
			case WAIT_ANY:
				OS_WaitAnyIrq();
				break;
		}

        OS_Printf("thread %d\n", threadNo);
    }

	(void)OS_SendMessage(&mesgQueue, NULL, OS_MESSAGE_NOBLOCK);
}

/*====== End of main.c ======*/
