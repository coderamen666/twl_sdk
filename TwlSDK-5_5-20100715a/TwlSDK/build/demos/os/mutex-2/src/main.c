/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - mutex-2
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>


void    VBlankIntr(void);
void    proc1(void *p1);
void    proc2(void *p1);
void    common(u32 val);

//---- thread
#define    STACK_SIZE     1024
#define    THREAD1_PRIO   10
#define    THREAD2_PRIO   11
OSThread thread1;
OSThread thread2;
u64     stack1[STACK_SIZE / sizeof(u64)];
u64     stack2[STACK_SIZE / sizeof(u64)];

BOOL    th1_sw = FALSE;

OSMutex mutex;

//---- pad
u16     preKeyData;
u16     keyData;
u16     keyTrigger;

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    //================ initialize
    OS_Init();
    GX_Init();
    OS_InitThread();

    //---- start thread-2
    OS_InitMutex(&mutex);
    OS_CreateThread(&thread2, proc2, NULL, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD2_PRIO);
    OS_WakeupThreadDirect(&thread2);

    //----  VBlank setting
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    //---- Dummy READ workaround for pushing 'A' at IPL
    preKeyData = PAD_Read();

    //================ main loop
    while (1)
    {
        //---- wait VBlank
        OS_WaitVBlankIntr();

        //---- read pad
        keyData = PAD_Read();
        keyTrigger = (u16)((preKeyData ^ keyData) & keyData);
        preKeyData = keyData;

        //---- press A button to start thread-1
        if (keyTrigger & PAD_BUTTON_A && !th1_sw)
        {
            th1_sw = TRUE;
            OS_CreateThread(&thread1, proc1, NULL, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                            THREAD1_PRIO);
            OS_WakeupThreadDirect(&thread1);
        }

        OS_WakeupThreadDirect(&thread2);
    }
}


//--------------------------------------------------------------------------------
//    VBlank interrupt handler
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

//--------------------------------------------------------------------------------
//    thread 1
//
void proc1(void *p1)
{
#pragma unused(p1)
    int     cnt = 0;

    OS_LockMutex(&mutex);
    while (1)
    {
        common(1);

        //---- exit thread after count 10
        if (++cnt > 10)
        {
            th1_sw = FALSE;
            OS_ExitThread();           // exit thread with locking mutex

            // even if exit thread-1 with locking mutex,
            // release the mutex automatically in OS_ExitThread()
        }
    }
}

//--------------------------------------------------------------------------------
//    thread 2
//
void proc2(void *p1)
{
#pragma unused(p1)
    while (1)
    {
        OS_LockMutex(&mutex);
        common(2);
        OS_UnlockMutex(&mutex);

        OS_SleepThread(NULL);
    }
}

//--------------------------------------------------------------------------------
//    common routine ( as critical routine here )
//
void common(u32 val)
{
#ifdef SDK_FINALROM
#pragma unused( val )
#endif

    OS_Printf("critical routine called from thread=%d\n", val);
}

/*====== End of main.c ======*/
