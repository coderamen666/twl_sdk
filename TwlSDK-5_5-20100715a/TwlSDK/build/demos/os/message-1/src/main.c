/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - message-1
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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

#define STACK_SIZE    1024
#define MESG_DEPTH    1
#define THREAD1_PRIO  10

OSMessage mesgBuffer[MESG_DEPTH];
OSMessageQueue mesgQueue;

OSThread thread1;
u64 stack1[STACK_SIZE / sizeof(u64)];

void proc1(void *arg);

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

    OS_InitMessageQueue(&mesgQueue, &mesgBuffer[0], MESG_DEPTH);

    OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE, THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

	for( n=0; n<5; n++ )
    {
		OSMessage message = (OSMessage)0x12345;

        OS_Printf("send message = %x\n", message);
        (void)OS_SendMessage(&mesgQueue, message, OS_MESSAGE_BLOCK);
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
        OSMessage message;

        (void)OS_ReceiveMessage(&mesgQueue, &message, OS_MESSAGE_BLOCK);
        OS_Printf("recv message = %x\n", message);
    }
}

/*====== End of main.c ======*/
