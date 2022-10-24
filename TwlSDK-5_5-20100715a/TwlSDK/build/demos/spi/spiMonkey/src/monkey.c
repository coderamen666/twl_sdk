/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos
  File:     monkey.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 10700 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include    "monkey.h"
#include    <nitro/spi.h>
#include    <nitro/spi/common/pm_common.h>
#include    <nitro/spi/ARM9/pm.h>


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void MonkeyThread(void *arg);
static void MonkeySamplingAlarm(void *arg);
static void MonkeyMicCallback(MICResult result, void *arg);
static void MonkeyTpCallback(TPRequestCommand command, TPRequestResult result, u16 index);


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static MonkeyWork monkey;


/*---------------------------------------------------------------------------*
  Name:         MonkeyInit

  Description:  Starts the thread for SPI device sampling.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyInit(void)
{
    // Create a thread exclusively for sampling.
    OS_InitMessageQueue(&(monkey.msg_q), monkey.msg_buf, MONKEY_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&(monkey.thread),
                    MonkeyThread,
                    0,
                    (void *)(monkey.stack + (MONKEY_STACK_SIZE / sizeof(u32))),
                    MONKEY_STACK_SIZE, MONKEY_THREAD_PRIORITY);
    OS_WakeupThreadDirect(&(monkey.thread));
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyGetNewTpData

  Description:  Gets the newest touch panel sampling data.
                Does not consider whether the data has already been read once.

  Arguments:    num: Number of pieces of data to get
                array: Array that will get the data.
                        Data is stored starting at the beginning of the array with the newest data first.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyGetNewTpData(s32 num, TPData *array)
{
    s32     i;
    s32     index;

    index = (s32)(monkey.tpIndex);
    for (i = 0; i < num; i++)
    {
        index = (index + (MONKEY_TP_ARRAY_MAX - 1)) % MONKEY_TP_ARRAY_MAX;
        array[i] = monkey.tpBuf[index];
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyGetNewMicData

  Description:  Gets the newest mic sampling data.
                Does not consider whether the data has already been read once.

  Arguments:    num: Number of pieces of data to get
                array: Array that will get the data.
                        Data is stored starting at the beginning of the array with the newest data first.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyGetNewMicData(s32 num, u16 *array)
{
    s32     i;
    s32     index;

    index = (s32)(monkey.micIndex);
    for (i = 0; i < num; i++)
    {
        index = (index + (MONKEY_MIC_ARRAY_MAX - 1)) % MONKEY_MIC_ARRAY_MAX;
        array[i] = monkey.micBuf[index];
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyThread

  Description:  The thread that performs sampling.

  Arguments:    arg: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyThread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;

    // Microphone API Initialization
    {
        MIC_Init();
        monkey.micIndex = 0;

        // Initialize PMIC
        PM_Init();
        // AMP on
        (void)PM_SetAmp(PM_AMP_ON);
        // Adjust AMP gain
        (void)PM_SetAmpGain(PM_AMPGAIN_80);
    }
    // Touch Panel API Initialization
    {
        TPCalibrateParam calibrate;

        TP_Init();
        if (TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
        }
        else
        {
            OS_Panic("Can't find TP calibration data.");
        }
        TP_SetCallback(MonkeyTpCallback);
        monkey.tpIndex = 0;
    }

    // Start the sampling timer
    OS_CreateAlarm(&(monkey.alarm));
    monkey.timerCount = 0;
    OS_SetPeriodicAlarm(&(monkey.alarm),
                        OS_GetTick(),
                        (MONKEY_MIC_SPAN_TICK * MONKEY_SAMPLING_SPAN_LINE),
                        MonkeySamplingAlarm, NULL);

    while (TRUE)
    {
        // Pause the thread until message is received
        (void)OS_ReceiveMessage(&(monkey.msg_q), &msg, OS_MESSAGE_BLOCK);

        // Touch Panel sampling
        if ((u32)msg == MONKEY_MESSAGE_TYPE_TP)
        {
            TP_RequestSamplingAsync();
        }
        // Mic sampling
        else if ((u32)msg == MONKEY_MESSAGE_TYPE_MIC)
        {
            if (MIC_RESULT_SUCCESS != MIC_DoSamplingAsync(MIC_SAMPLING_TYPE_12BIT,
                                                          &(monkey.micBuf[monkey.micIndex]),
                                                          MonkeyMicCallback, NULL))
            {
                OS_Printf("Monkey: MIC request failure.\n");
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeySamplingAlarm

  Description:  Alarm handler that controls when to send messages to the thread.

  Arguments:    arg: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Periodic timer and sampling timing chart

    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
MIC   >----           >----           >----           >----           >----
TP          >--------                       >--------                       >--

 *---------------------------------------------------------------------------*/
static void MonkeySamplingAlarm(void *arg)
{
#pragma unused( arg )

    if (0 == (monkey.timerCount % 8))
    {
        (void)OS_SendMessage(&(monkey.msg_q), (void *)MONKEY_MESSAGE_TYPE_MIC, OS_MESSAGE_BLOCK);
    }
    else if (3 == (monkey.timerCount % 16))
    {
        (void)OS_SendMessage(&(monkey.msg_q), (void *)MONKEY_MESSAGE_TYPE_TP, OS_MESSAGE_BLOCK);
    }

    monkey.timerCount++;
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyMicCallback

  Description:  Response callback function for mic sampling requests.
                When sampling was completed without trouble, it advances the data storage destination buffer to the next position.
                

  Arguments:    result: Processing result corresponding to mic operation request
                arg: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyMicCallback(MICResult result, void *arg)
{
#pragma unused( arg )

    if (result == MIC_RESULT_SUCCESS)
    {
        // Advance to the next data storage buffer position
        monkey.micIndex = (u16)((monkey.micIndex + 1) % MONKEY_MIC_ARRAY_MAX);
    }
    else
    {
        // Sound has prevented the previous sampling process from completing, so this new sampling process was not started
        // 
        OS_Printf("Monkey: MIC request failure.\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyTpCallback

  Description:  Callback function that responds to the touch panel's sampling requests.
                When sampling was completed without trouble, it advances the data storage destination buffer to the next position.
                

  Arguments:    command: Operation request command that the response responds to. Not used.
                result: Processing result for the touch panel operation request.
                index: Index for auto-sampling. Not used.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyTpCallback(TPRequestCommand command, TPRequestResult result, u16 index)
{
#pragma unused( command , index )

    if (result == TP_RESULT_SUCCESS)
    {
        // Convert raw sampling values to screen coordinates
        (void)TP_GetCalibratedResult(&(monkey.tpBuf[monkey.tpIndex]));
        // Advance to the next data storage buffer position
        monkey.tpIndex = (u16)((monkey.tpIndex + 1) % MONKEY_TP_ARRAY_MAX);
    }
    else
    {
        // Sound has prevented the previous sampling process from completing, so this new sampling process was not started
        // 
        OS_Printf("Monkey: TP request failure.\n");
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
