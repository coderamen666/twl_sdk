/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - snd - common
  File:     snd_main.c

  Copyright 2004-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro/snd/common/main.h>

#include <nitro/os.h>
#include <nitro/snd/common/global.h>
#include <nitro/snd/common/exchannel.h>
#include <nitro/snd/common/seq.h>
#include <nitro/snd/common/alarm.h>
#include <nitro/snd/common/command.h>
#include <nitro/snd/common/util.h>
#include <nitro/snd/common/work.h>

#ifdef SDK_TWL
#ifdef SDK_ARM7
#include <twl/snd/ARM7/sndex_api.h>
#endif
#endif

/******************************************************************************
	Macro definitions
 ******************************************************************************/

#ifdef SDK_ARM7

#define SND_THREAD_STACK_SIZE      1024
#define SND_THREAD_MESSAGE_BUFSIZE 8

#define SND_ALARM_COUNT_P1  0x10000

#endif /* SDK_ARM7 */

/******************************************************************************
	Static variables
 ******************************************************************************/

#ifdef SDK_ARM9

static OSMutex sSndMutex;

#else  /* SDK_ARM7 */

static OSThread sndThread;
static u64 sndStack[SND_THREAD_STACK_SIZE / sizeof(u64)];
static OSAlarm sndAlarm;
static OSMessageQueue sndMesgQueue;
static OSMessage sndMesgBuffer[SND_THREAD_MESSAGE_BUFSIZE];

#endif /* SDK_ARM9 */


/******************************************************************************
	Static function declarations
 ******************************************************************************/

#ifdef SDK_ARM7

static void SndThread(void *arg);
static void SndAlarmCallback(void *arg);

#endif /* SDK_ARM7 */

/******************************************************************************
	Public functions
 ******************************************************************************/

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         SND_Init

  Description:  Initializes sound library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_Init(void)
{
    {
        static BOOL initialized = FALSE;
        if (initialized)
            return;
        initialized = TRUE;
    }

    OS_InitMutex(&sSndMutex);
    SND_CommandInit();
    SND_AlarmInit();
}

#else  /* SDK_ARM7 */

/*---------------------------------------------------------------------------*
  Name:         SND_Init

  Description:  Initializes sound and starts the sound thread.

  Arguments:    threadPrio - Thread priority.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_Init(u32 threadPrio)
{
    {
        static BOOL initialized = FALSE;
        if (initialized)
            return;
        initialized = TRUE;
    }

    SND_CommandInit();

    SND_CreateThread(threadPrio);
}

/*---------------------------------------------------------------------------*
  Name:         SND_CreateThread

  Description:  Starts the sound thread.

  Arguments:    threadPrio - Thread priority.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_CreateThread(u32 threadPrio)
{
    OS_CreateThread(&sndThread,
                    SndThread,
                    NULL,
                    sndStack + SND_THREAD_STACK_SIZE / sizeof(u64),
                    SND_THREAD_STACK_SIZE, threadPrio);
    OS_WakeupThreadDirect(&sndThread);
}

/*---------------------------------------------------------------------------*
  Name:         SND_SetThreadPriority

  Description:  Configures the priority of the sound thread.

  Arguments:    prio - Thread priority.

  Returns:      Whether it was successful or not.
 *---------------------------------------------------------------------------*/
BOOL SND_SetThreadPriority(u32 prio)
{
    return OS_SetThreadPriority(&sndThread, prio);
}

/*---------------------------------------------------------------------------*
  Name:         SND_InitIntervalTimer

  Description:  Initializes the interval timer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_InitIntervalTimer(void)
{
    OS_InitMessageQueue(&sndMesgQueue, sndMesgBuffer, SND_THREAD_MESSAGE_BUFSIZE);

    OS_CreateAlarm(&sndAlarm);
}

/*---------------------------------------------------------------------------*
  Name:         SND_StartIntervalTimer

  Description:  Starts the interval timer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_StartIntervalTimer(void)
{
    OS_SetPeriodicAlarm(&sndAlarm,
                        OS_GetTick() + SND_ALARM_COUNT_P1,
                        SND_PROC_INTERVAL, &SndAlarmCallback, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         SND_StopIntervalTimer

  Description:  Stops the interval timer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_StopIntervalTimer(void)
{
    OS_CancelAlarm(&sndAlarm);
}

/*---------------------------------------------------------------------------*
  Name:         SND_WaitForIntervalTimer

  Description:  Waits for the interval timer.

  Arguments:    None.

  Returns:      Returns a message.
 *---------------------------------------------------------------------------*/
OSMessage SND_WaitForIntervalTimer(void)
{
    OSMessage message;

    (void)OS_ReceiveMessage(&sndMesgQueue, &message, OS_MESSAGE_BLOCK);

    return message;
}

/*---------------------------------------------------------------------------*
  Name:         SND_SendWakeupMessage

  Description:  Sends a message to wake up the sound thread.

  Arguments:    None.

  Returns:      Flag expressing whether or not the message was successfully sent.
 *---------------------------------------------------------------------------*/
BOOL SND_SendWakeupMessage(void)
{
    return OS_SendMessage(&sndMesgQueue, (OSMessage)SND_MESSAGE_WAKEUP_THREAD, OS_MESSAGE_NOBLOCK);
}

#endif /* SDK_ARM7 */

/******************************************************************************
	Private function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         SNDi_LockMutex

  Description:  Locks the sound mutex.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SNDi_LockMutex(void)
{
#ifdef SDK_ARM9
    OS_LockMutex(&sSndMutex);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SNDi_UnlockMutex

  Description:  Unlocks the sound mutex.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SNDi_UnlockMutex(void)
{
#ifdef SDK_ARM9
    OS_UnlockMutex(&sSndMutex);
#endif
}

/******************************************************************************
	Static functions
 ******************************************************************************/

#ifdef SDK_ARM7

/*---------------------------------------------------------------------------*
  Name:         SndAlarmCallback

  Description:  Callback called in the alarm cycle.

  Arguments:    arg: User data (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SndAlarmCallback(void * /*arg */ )
{
    if (!OS_SendMessage(&sndMesgQueue, (OSMessage)SND_MESSAGE_PERIODIC, OS_MESSAGE_NOBLOCK))
    {
        OS_PutString("Failed sound alarm OS_SendMessage\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         SndThread

  Description:  Sound thread function.

  Arguments:    arg: User data (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SndThread(void * /*arg */ )
{
#ifdef  SDK_TWL
    if (OS_IsRunOnTwl() == TRUE)
    {
        /* Initialize I2S-related settings */
        SDNEXi_InitializeSMIX();
    }
    /* Power supply to the sound circuit  */
    reg_SND_POWCNT  |=  REG_SND_POWCNT_SPE_MASK;
#endif

    SND_InitIntervalTimer();
    SND_ExChannelInit();
    SND_SeqInit();
    SND_AlarmInit();
    SND_Enable();
    SND_SetOutputSelector(SND_OUTPUT_MIXER,
                          SND_OUTPUT_MIXER, SND_CHANNEL_OUT_MIXER, SND_CHANNEL_OUT_MIXER);
    SND_SetMasterVolume(SND_MASTER_VOLUME_MAX);

    SND_StartIntervalTimer();

    while (1)
    {
        OSMessage message;
        BOOL    doPeriodicProc = FALSE;

        //-----------------------------------------------------------------------------
        // Wait interval timer

        message = SND_WaitForIntervalTimer();

        switch ((u32)message)
        {
        case SND_MESSAGE_PERIODIC:
            doPeriodicProc = TRUE;
            break;
        case SND_MESSAGE_WAKEUP_THREAD:
            break;
        }

        //-----------------------------------------------------------------------------
        // Update registers

        SND_UpdateExChannel();

        //-----------------------------------------------------------------------------
        // ARM9 command proc

        SND_CommandProc();

        //-----------------------------------------------------------------------------
        // Framework

        SND_SeqMain(doPeriodicProc);
        SND_ExChannelMain(doPeriodicProc);

        SND_UpdateSharedWork();

        (void)SND_CalcRandom();
    }

    SND_Disable();
    SND_StopIntervalTimer();
}

#endif /* SDK_ARM7 */

/*====== End of snd_main.c ======*/
