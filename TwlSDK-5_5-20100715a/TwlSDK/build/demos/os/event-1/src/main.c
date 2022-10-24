/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - event-1
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

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

//---------------- defines
#define STACK_SIZE      1024
#define THREAD1_PRIO    20
#define COUNT           OS_MilliSecondsToTicks( 1000 )

#define WAKEUP_EVENT    0x00000001

//---------------- variables
OSThread thread1;
u64      stack1[STACK_SIZE / sizeof(u64)];
OSAlarm  alarm;
int      alarmCount = 0;
OSEvent  event;

//---------------- prototypes
void alarmCallback(void* arg);
void VBlankIntr(void);
void proc1(void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();

    //---- init event
    OS_InitEvent(&event);

    //---- create thread
    OS_CreateThread(&thread1,
                    proc1,
                    (void *)0x123,
                    stack1 + STACK_SIZE / sizeof(u64),
                    STACK_SIZE,
                    THREAD1_PRIO);
    OS_WakeupThreadDirect(&thread1);

    //---- interrupt setting
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    OS_Printf("*** start event-1 demo\n");

    //---- alarm
    OS_SetPeriodicAlarm( &alarm, COUNT, COUNT, alarmCallback, NULL );

    //---- main loop
    while( alarmCount <= 10 )
    {
        static u16 prevButton = 0;
        u16 button = PAD_Read();
        u16 trigger = (u16)((button ^ prevButton) & button);
        prevButton = button;

        SVC_WaitVBlankIntr();

        if ( trigger & PAD_BUTTON_A )
        {
            //---- signal event
            OS_SignalEvent(&event, WAKEUP_EVENT);
        }
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         alarmCallback

  Description:  alarm handler

  Arguments:    arg : 

  Returns:      None
 *---------------------------------------------------------------------------*/
void alarmCallback(void* arg)
{
#pragma unused(arg)
    OS_Printf( "alarm callback (%d)\n", alarmCount );
    alarmCount ++;

    //---- signal event
    OS_SignalEvent(&event, WAKEUP_EVENT);
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  vblank handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         proc1

  Description:  thread1

  Arguments:    arg :

  Returns:      None
 *---------------------------------------------------------------------------*/
void proc1(void *arg)
{
#pragma unused( arg )
    while (1)
    {
        //---- wait event
        (void)OS_WaitEventEx(&event, WAKEUP_EVENT, OS_EVENT_MODE_AND, WAKEUP_EVENT);

        OS_Printf("Thread1\n");
    }
}
