/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - thread-3
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

#define STACK_SIZE       1024
#define THREAD1_PRIO     17
#define MESSAGE_EXIT     (OSMessage)100

OSMessage mesgBuffer[10];
OSMessageQueue mesgQueue;

OSThread thread1;
u32     stack1[STACK_SIZE / sizeof(u32)];

void    VBlankIntr(void);
void    proc1(void *p1);

void    OS_WaitInterrupt(BOOL clearEnable, OSIrqMask intrFlags);

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();
    OS_InitThread();

#ifdef SDK_ARM9
    GX_Init();
#endif // SDK_ARM9

    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    OS_InitMessageQueue(&mesgQueue, &mesgBuffer[0], 10);

    //================ test 1
    //---- create thread
    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

    //---- send message to be die for thread1
    OS_Printf("terminate thread1.\n");
    (void)OS_SendMessage(&mesgQueue, (OSMessage)MESSAGE_EXIT, OS_MESSAGE_BLOCK);
    OS_JoinThread(&thread1);
    OS_Printf("terminated.\n");


    //================ test 2
    //---- create thread
    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u32), STACK_SIZE, THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

    //---- send message to be die for thread1
    OS_Printf("terminate thread1.\n");
    (void)OS_SendMessage(&mesgQueue, (OSMessage)MESSAGE_EXIT, OS_MESSAGE_BLOCK);

    //---- wait till thread1 die
    while (!OS_IsThreadTerminated(&thread1))
    {
        OS_WaitIrq(1, OS_IE_V_BLANK);

        // Notice: if you write OS_WaitInterrupt() instead of OS_WaitIrq(),
        //         thread1 will not die because OS_WaitInterrupt() doesn't
        //         cause to switch thread.  OS_WaitIrq() do.
    }

    OS_Printf("terminated.\n");


    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    VBlank interrupt handler
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//--------------------------------------------------------------------------------
//    proc1, which is procedure of thread1
//
void proc1(void *p1)
{
#pragma unused(p1)
    OSMessage message;

    while (1)
    {
        (void)OS_ReceiveMessage(&mesgQueue, &message, OS_MESSAGE_BLOCK);

        if (message == MESSAGE_EXIT)
        {
            OS_Printf("thread1 will die...\n");
            OS_ExitThread();
        }
    }
}


/*====== End of main.c ======*/
