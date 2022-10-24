/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     external.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author:$
 *---------------------------------------------------------------------------*/

#include <nitro/os.h>
#include <nitro/rtc.h>

#ifdef  SDK_TWL
#include <twl/rtc/ARM9/api_ex.h>
#include "private.h"
#endif

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWL

// Lock definition for exclusive processing of asynchronous functions
typedef enum RTCLock
{
    RTC_LOCK_OFF = 0,                  // Unlock status
    RTC_LOCK_ON,                       // Lock status
    RTC_LOCK_MAX
}
RTCLock;

// Sequence definitions for processing that includes continuous command transmission.
typedef enum RTCSequence
{
    RTC_SEQ_GET_DATE = 0,              // Sequence for getting dates
    RTC_SEQ_GET_TIME,                  // Sequence for getting times
    RTC_SEQ_GET_DATETIME,              // Sequence for getting dates and times
    RTC_SEQ_SET_DATE,                  // Sequence for setting dates
    RTC_SEQ_SET_TIME,                  // Sequence for setting times
    RTC_SEQ_SET_DATETIME,              // Sequence for setting dates and times
    RTC_SEQ_GET_ALARM1_STATUS,         // Sequence for getting alarm 1's status
    RTC_SEQ_GET_ALARM2_STATUS,         // Sequence for getting alarm 2's status
    RTC_SEQ_GET_ALARM_PARAM,           // Sequence for getting alarm setting values
    RTC_SEQ_SET_ALARM1_STATUS,         // Sequence for changing alarm 1's status
    RTC_SEQ_SET_ALARM2_STATUS,         // Sequence for changing alarm 2's status
    RTC_SEQ_SET_ALARM1_PARAM,          // Sequence for changing alarm 1's setting values
    RTC_SEQ_SET_ALARM2_PARAM,          // Sequence for changing alarm 2's setting values
    RTC_SEQ_SET_HOUR_FORMAT,           // Sequence for changing the format of the time notation
    RTC_SEQ_SET_REG_STATUS2,           // Sequence for status 2 register writes
    RTC_SEQ_SET_REG_ADJUST,            // Sequence for adjust register writes
    RTC_SEQ_MAX
}
RTCSequence;

// Work structure
typedef struct RTCWork
{
    u32     lock;                      // Exclusive lock
    RTCCallback callback;              // For saving an asynchronous callback function
    void   *buffer[2];                 // For storing asynchronous function parameters
    void   *callbackArg;               // For saving arguments to the callback function
    u32     sequence;                  // For controlling continuous processing mode
    u32     index;                     // For controlling continuous processing status
    RTCInterrupt interrupt;            // For saving the call function(s) when an alarm notification occurs
    RTCResult commonResult;            // For saving asynchronous function processing results

}
RTCWork;

#endif

/*---------------------------------------------------------------------------*
    Static Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 rtcInitialized;             // Initialized verify flag
static RTCWork rtcWork;                // Structure that combines work variables

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void RtcCommonCallback(PXIFifoTag tag, u32 data, BOOL err);
static u32 RtcBCD2HEX(u32 bcd);
static u32 RtcHEX2BCD(u32 hex);
static BOOL RtcCheckAlarmParam(const RTCAlarmParam *param);
static RTCRawAlarm RtcMakeAlarmParam(const RTCAlarmParam *param);
static BOOL RtcCheckDate(const RTCDate *date, RTCRawDate *raw);
static BOOL RtcCheckTime(const RTCTime *time, RTCRawTime *raw);
static void RtcGetResultCallback(RTCResult result, void *arg);
static void RtcWaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_Init

  Description:  Initializes the RTC library.
       Notice:  If component-side initialization detects that the power supply to the RTC device has been completely cut (this is caused for example by removing the battery), the internal state of the device will be reset. When this happens, the date is initialized to 2000/01/01 (Saturday), and the time is set to 00:00:00 (24-hour clock).
                
                
                
                The RTC alarm-related settings values are zero-cleared in the system startup sequence regardless of removed batteries, but when applications are restarted by a software reset, the settings carry over. 
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void RTC_Init(void)
{
    // Verification of non-initialization
    if (rtcInitialized)
    {
        return;
    }
    rtcInitialized = 1;

    // Work variable initialization
    rtcWork.lock = RTC_LOCK_OFF;
    rtcWork.callback = NULL;
    rtcWork.interrupt = NULL;
    rtcWork.buffer[0] = NULL;
    rtcWork.buffer[1] = NULL;

    // Wait until the ARM7 RTC library is started
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_RTC, PXI_PROC_ARM7))
    {
    }

    // Set the PXI callback function
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_RTC, RtcCommonCallback);
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateAsync

  Description:  Asynchronously reads date data from the RTC.

  Arguments:    date: Buffer for storing date data
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateAsync(RTCDate *date, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_ASSERT(date != NULL);
    SDK_ASSERT(callback != NULL);

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send date read command
    rtcWork.sequence = RTC_SEQ_GET_DATE;
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)date;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_ReadRawDateAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDate

  Description:  Reads date data from the RTC.

  Arguments:    date: Buffer for storing date data

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDate(RTCDate *date)
{
    rtcWork.commonResult = RTC_GetDateAsync(date, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetTimeAsync

  Description:  Asynchronously reads time data from the RTC.

  Arguments:    time: Buffer for storing time data
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetTimeAsync(RTCTime *time, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(time);
    SDK_NULL_ASSERT(callback);

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send time read command
    rtcWork.sequence = RTC_SEQ_GET_TIME;
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)time;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_ReadRawTimeAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetTime

  Description:  Reads time data from the RTC.

  Arguments:    time: Buffer for storing time data

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetTime(RTCTime *time)
{
    rtcWork.commonResult = RTC_GetTimeAsync(time, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeAsync

  Description:  Asynchronously reads date and time data from the RTC.

  Arguments:    date: Buffer for storing date data
                time: Buffer for storing time data
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeAsync(RTCDate *date, RTCTime *time, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(date);
    SDK_NULL_ASSERT(time);
    SDK_NULL_ASSERT(callback);

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send time read command
    rtcWork.sequence = RTC_SEQ_GET_DATETIME;
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)date;
    rtcWork.buffer[1] = (void *)time;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_ReadRawDateTimeAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTime

  Description:  Read date and time data from RTC.

  Arguments:    date: Buffer for storing date data
                time: Buffer for storing time data

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTime(RTCDate *date, RTCTime *time)
{
    rtcWork.commonResult = RTC_GetDateTimeAsync(date, time, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateAsync

  Description:  Asynchronously writes date data to the RTC.

  Arguments:    date: Buffer where date data is stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDateAsync(const RTCDate *date, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(date);
    SDK_NULL_ASSERT(callback);

    // Verify and edit the date to set
    if (!RtcCheckDate(date, &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date)))
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send date write command
    rtcWork.sequence = RTC_SEQ_SET_DATE;
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_WriteRawDateAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDate

  Description:  Writes date data to the RTC.

  Arguments:    date: Buffer where date data is stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDate(const RTCDate *date)
{
    rtcWork.commonResult = RTC_SetDateAsync(date, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetTimeAsync

  Description:  Asynchronously writes time data to the RTC.

  Arguments:    time: Buffer where time data is stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetTimeAsync(const RTCTime *time, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(time);
    SDK_NULL_ASSERT(callback);

    // Verify and edit the time to set
    if (!RtcCheckTime(time, &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time)))
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send time write command
    rtcWork.sequence = RTC_SEQ_SET_TIME;
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_WriteRawTimeAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetTime

  Description:  Writes time data to the RTC.

  Arguments:    time: Buffer where time data is stored

  Returns:      RTCResult: Device operation result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetTime(const RTCTime *time)
{
    rtcWork.commonResult = RTC_SetTimeAsync(time, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateTimeAsync

  Description:  Asynchronously writes date and time data to the RTC.

  Arguments:    date: Buffer where date data is stored
                time: Buffer where time data is stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTC_SetDateTimeAsync(const RTCDate *date, const RTCTime *time, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(date);
    SDK_NULL_ASSERT(time);
    SDK_NULL_ASSERT(callback);

    // Verify and edit the date and time that will be set
    if (!RtcCheckDate(date, &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date)))
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }
    if (!RtcCheckTime(time, &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time)))
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send date/time write command
    rtcWork.sequence = RTC_SEQ_SET_DATETIME;
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_WriteRawDateTimeAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateTime

  Description:  Writes date and time data to the RTC.

  Arguments:    date: Buffer where date data is stored
                time: Buffer where time data is stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDateTime(const RTCDate *date, const RTCTime *time)
{
    rtcWork.commonResult = RTC_SetDateTimeAsync(date, time, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetRegStatus2Async

  Description:  Writes data to the RTC status 2 register.

  Arguments:    status2: Buffer where status 2 register contents are stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTCi_SetRegStatus2Async(const RTCRawStatus2 *status2, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(status2);
    SDK_NULL_ASSERT(callback);

    ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.intr_mode = status2->intr_mode;
    ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.intr2_mode =
        status2->intr2_mode;
    ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2.test = status2->test;

    /* No parameter check */
    // Return RTC_RESULT_ILLEGAL_PARAMETER;


    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send the status 2 register write command
    rtcWork.sequence = RTC_SEQ_SET_REG_STATUS2;
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_WriteRawStatus2Async())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetRegStatus2

  Description:  Writes data to the RTC status 2 register.

  Arguments:    status2: Buffer where status 2 register contents are stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTCi_SetRegStatus2(const RTCRawStatus2 *status2)
{
    rtcWork.commonResult = RTCi_SetRegStatus2Async(status2, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}


/*---------------------------------------------------------------------------*
  Name:         RTC_SetRegAdjustAsync

  Description:  Writes data to the RTC adjust register.

  Arguments:    adjust: Buffer where the adjust register contents are stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTCi_SetRegAdjustAsync(const RTCRawAdjust *adjust, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(adjust);
    SDK_NULL_ASSERT(callback);

    ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.adjust.adjust = adjust->adjust;

    /* No parameter check */
    // Return RTC_RESULT_ILLEGAL_PARAMETER;

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send the status 2 register write command
    rtcWork.sequence = RTC_SEQ_SET_REG_ADJUST;
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_WriteRawAdjustAsync())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}


/*---------------------------------------------------------------------------*
  Name:         RTC_SetRegAdjust

  Description:  Writes data to the RTC adjust register.

  Arguments:    status2: Buffer where the adjust register contents are stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTCi_SetRegAdjust(const RTCRawAdjust *Adjust)
{
    rtcWork.commonResult = RTCi_SetRegAdjustAsync(Adjust, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}



/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmStatusAsync

  Description:  Asynchronously reads alarm ON/OFF status from the RTC.

  Arguments:    chan: Alarm channel
                status: Buffer for storing alarm status
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTC_GetAlarmStatusAsync(RTCAlarmChan chan, RTCAlarmStatus *status, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(status);
    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (chan >= RTC_ALARM_CHAN_MAX)
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }
    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send status 2 register read command
    switch (chan)
    {
    case RTC_ALARM_CHAN_1:
        rtcWork.sequence = RTC_SEQ_GET_ALARM1_STATUS;
        break;
    case RTC_ALARM_CHAN_2:
        rtcWork.sequence = RTC_SEQ_GET_ALARM2_STATUS;
        break;
    }
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)status;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_ReadRawStatus2Async())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmStatus

  Description:  Reads alarm ON/OFF status from the RTC.

  Arguments:    chan: Alarm channel
                status: Buffer for storing alarm status

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmStatus(RTCAlarmChan chan, RTCAlarmStatus *status)
{
    rtcWork.commonResult = RTC_GetAlarmStatusAsync(chan, status, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmParamAsync

  Description:  Asynchronously reads alarm setting values from the RTC.

  Arguments:    chan: Alarm channel
                param: Buffer for storing alarm setting values
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTC_GetAlarmParamAsync(RTCAlarmChan chan, RTCAlarmParam *param, RTCCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(param);
    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (chan >= RTC_ALARM_CHAN_MAX)
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send a read command for the alarm 1 or 2 setting values
    rtcWork.sequence = RTC_SEQ_GET_ALARM_PARAM;
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)param;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (chan == RTC_ALARM_CHAN_1)
    {
        if (RTCi_ReadRawAlarm1Async())
        {
            return RTC_RESULT_SUCCESS;
        }
        else
        {
            rtcWork.lock    =   RTC_LOCK_OFF;
            return RTC_RESULT_SEND_ERROR;
        }
    }
    if (RTCi_ReadRawAlarm2Async())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmParam

  Description:  Reads alarm setting values from the RTC.

  Arguments:    chan: Alarm channel
                param: Buffer for storing alarm setting values

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmParam(RTCAlarmChan chan, RTCAlarmParam *param)
{
    rtcWork.commonResult = RTC_GetAlarmParamAsync(chan, param, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmInterrupt

  Description:  Sets the callback function for when an alarm interrupt is generated.

  Arguments:    interrupt: Callback function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void RTC_SetAlarmInterrupt(RTCInterrupt interrupt)
{
    rtcWork.interrupt = interrupt;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmStatusAsync

  Description:  Asynchronously writes alarm status to the RTC.

  Arguments:    chan: Alarm channel
                status: Buffer where alarm status is stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTC_SetAlarmStatusAsync(RTCAlarmChan chan, const RTCAlarmStatus *status, RTCCallback callback,
                        void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(status);
    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (chan >= RTC_ALARM_CHAN_MAX)
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }
    if (*status > RTC_ALARM_STATUS_ON)
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send status 2 register read command
    switch (chan)
    {
    case RTC_ALARM_CHAN_1:
        rtcWork.sequence = RTC_SEQ_SET_ALARM1_STATUS;
        break;
    case RTC_ALARM_CHAN_2:
        rtcWork.sequence = RTC_SEQ_SET_ALARM2_STATUS;
        break;
    }
    rtcWork.index = 0;
    rtcWork.buffer[0] = (void *)status;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    if (RTCi_ReadRawStatus2Async())
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        rtcWork.lock    =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmStatus

  Description:  Writes alarm status to the RTC.

  Arguments:    chan: Alarm channel
                status: Buffer where alarm status is stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmStatus(RTCAlarmChan chan, const RTCAlarmStatus *status)
{
    rtcWork.commonResult = RTC_SetAlarmStatusAsync(chan, status, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmParamAsync

  Description:  Asynchronously writes alarm settings to the RTC.
       Notice:  Write will fail if the RTC alarm status is not ON, because the device side will not accept write.
                

  Arguments:    chan: Alarm channel
                param: Buffer where alarm setting values are stored
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTC_SetAlarmParamAsync(RTCAlarmChan chan, const RTCAlarmParam *param, RTCCallback callback,
                       void *arg)
{
    OSIntrMode enabled;
    RTCRawAlarm *pAlarm = &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm);
    BOOL    result = FALSE;

    SDK_NULL_ASSERT(param);
    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (chan >= RTC_ALARM_CHAN_MAX)
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }
    if (!RtcCheckAlarmParam(param))
    {
        return RTC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (rtcWork.lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    rtcWork.lock = RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Edit the data to set
    rtcWork.index = 0;
    rtcWork.callback = callback;
    rtcWork.callbackArg = arg;
    ((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm = RtcMakeAlarmParam(param);
    // Divide the send command by the alarm number
    switch (chan)
    {
    case RTC_ALARM_CHAN_1:
        // Send the alarm 1 register write command
        rtcWork.sequence = RTC_SEQ_SET_ALARM1_PARAM;
        result = RTCi_WriteRawAlarm1Async();
        break;
    case RTC_ALARM_CHAN_2:
        // Send the alarm 2 register write command
        rtcWork.sequence = RTC_SEQ_SET_ALARM2_PARAM;
        result = RTCi_WriteRawAlarm2Async();
        break;
    }
    if (result)
    {
        return RTC_RESULT_SUCCESS;
    }
    rtcWork.lock    =   RTC_LOCK_OFF;
    return RTC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmParam

  Description:  Writes alarm setting values to the RTC.

  Arguments:    chan: Alarm channel
                param: Buffer where alarm setting values are stored

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmParam(RTCAlarmChan chan, const RTCAlarmParam *param)
{
    rtcWork.commonResult = RTC_SetAlarmParamAsync(chan, param, RtcGetResultCallback, NULL);
    if (rtcWork.commonResult == RTC_RESULT_SUCCESS)
    {
        RtcWaitBusy();
    }
    return rtcWork.commonResult;
}

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         RTCi_GetSysWork

  Description:  Gets pointer to the library-internal shared working variable.

  Arguments:    None.

  Returns:      RTCWork*: Pointer to library-internal shared working variable.
 *---------------------------------------------------------------------------*/
RTCWork*
RTCi_GetSysWork(void)
{
    return &rtcWork;
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_GetCounterAsync

  Description:  Asynchronously reads the Up Counter value from the RTC.

  Arguments:    count: Buffer to store the Up Counter value
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_GetCounterAsync(u32* count, RTCCallback callback, void* arg)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
            RTC_RESULT_INVALID_COMMAND : RTCEXi_GetCounterAsync(count, callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_GetCounter

  Description:  Reads the Up Counter value from the RTC.

  Arguments:    count: Buffer to store the Up Counter value

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_GetCounter(u32* count)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
        RTC_RESULT_INVALID_COMMAND : RTCEXi_GetCounter(count));
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_GetFoutAsync

  Description:  Asynchronously reads FOUT setting values from the RTC.

  Arguments:    fout: Buffer for storing FOUT setting values
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_GetFoutAsync(u16* fout, RTCCallback callback, void* arg)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
        RTC_RESULT_INVALID_COMMAND : RTCEXi_GetFoutAsync(fout, callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_GetFout

  Description:  Asynchronously reads FOUT setting values from the RTC.

  Arguments:    fout: Buffer for storing FOUT setting values

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_GetFout(u16* fout)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
        RTC_RESULT_INVALID_COMMAND : RTCEXi_GetFout(fout));
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_SetFoutAsync

  Description:  Asynchronously writes FOUT settings to the RTC.

  Arguments:    fout: Buffer storing FOUT setting values
                callback: Function to be called when the asynchronous process is completed
                arg: Argument used when calling the callback function

  Returns:      RTCResult: Result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_SetFoutAsync(const u16* fout, RTCCallback callback, void* arg)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
        RTC_RESULT_INVALID_COMMAND : RTCEXi_SetFoutAsync(fout, callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_SetFout

  Description:  Writes FOUT setting values to the RTC.

  Arguments:    fout: Buffer storing FOUT setting values

  Returns:      RTCResult: Device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCi_SetFout(const u16* fout)
{
    return ((OS_IsRunOnTwl() == FALSE) ?
        RTC_RESULT_INVALID_COMMAND : RTCEXi_SetFout(fout));
}
#endif

/*---------------------------------------------------------------------------*
  Name:         RtcCommonCallback

  Description:  Common callback function for asynchronous RTC functions.

  Arguments:    tag: PXI tag that shows the message type
                data: Message from ARM7
                err: PXI transfer error flag

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void RtcCommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    RTCResult result;
    RTCPxiResult pxiresult;
    u8      command;
    RTCCallback cb;

    // Verify PXI communication error
    if (err)
    {
        // Forcibly end sequence
        if (rtcWork.index)
        {
            rtcWork.index = 0;
        }
        if (rtcWork.lock != RTC_LOCK_OFF)
        {
            rtcWork.lock = RTC_LOCK_OFF;
        }
        if (rtcWork.callback)
        {
            cb = rtcWork.callback;
            rtcWork.callback = NULL;
            cb(RTC_RESULT_FATAL_ERROR, rtcWork.callbackArg);
        }
        return;
    }

    // Analyze received data
    command = (u8)((data & RTC_PXI_COMMAND_MASK) >> RTC_PXI_COMMAND_SHIFT);
    pxiresult = (RTCPxiResult)((data & RTC_PXI_RESULT_MASK) >> RTC_PXI_RESULT_SHIFT);

    // Verify alarm interrupt
    if (command == RTC_PXI_COMMAND_INTERRUPT)
    {
        // While it is possible to decide whether the alarm is 1 or 2 in pxiresult, because the callbacks are unified, which one it is will be ignored
        //  
        if (rtcWork.interrupt)
        {
            // Callback alarm interrupt notification
            rtcWork.interrupt();
        }
        return;
    }

    // If the response indicates that processing was successful, processing performed after that will depend on the type of the internal state
    if (pxiresult == RTC_PXI_RESULT_SUCCESS)
    {
        result = RTC_RESULT_SUCCESS;
        switch (rtcWork.sequence)
        {
            // Sequence for getting dates
        case RTC_SEQ_GET_DATE:
            {
                RTCDate *pDst = (RTCDate *)(rtcWork.buffer[0]);
                RTCRawDate *pSrc = &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date);

                pDst->year = RtcBCD2HEX(pSrc->year);
                pDst->month = RtcBCD2HEX(pSrc->month);
                pDst->day = RtcBCD2HEX(pSrc->day);
                pDst->week = RTC_GetDayOfWeek(pDst);
            }
            break;
            // Sequence for getting times
        case RTC_SEQ_GET_TIME:
            {
                RTCTime *pDst = (RTCTime *)(rtcWork.buffer[0]);
                RTCRawTime *pSrc = &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time);

                pDst->hour = RtcBCD2HEX(pSrc->hour);
                pDst->minute = RtcBCD2HEX(pSrc->minute);
                pDst->second = RtcBCD2HEX(pSrc->second);
            }
            break;
            // Sequence for getting dates and times
        case RTC_SEQ_GET_DATETIME:
            {
                RTCDate *pDst = (RTCDate *)(rtcWork.buffer[0]);
                RTCRawDate *pSrc = &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.date);

                //pDst->year =  RtcBCD2HEX( pSrc->year );   // Changed to the following code because for some reason the value is not passed
                pDst->year = RtcBCD2HEX(*(u32 *)pSrc & 0x000000ff);
                pDst->month = RtcBCD2HEX(pSrc->month);
                pDst->day = RtcBCD2HEX(pSrc->day);
                pDst->week = RTC_GetDayOfWeek(pDst);
            }
            {
                RTCTime *pDst = (RTCTime *)(rtcWork.buffer[1]);
                RTCRawTime *pSrc = &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->t.time);

                pDst->hour = RtcBCD2HEX(pSrc->hour);
                pDst->minute = RtcBCD2HEX(pSrc->minute);
                pDst->second = RtcBCD2HEX(pSrc->second);
            }
            break;
            // Sequence for changing dates
        case RTC_SEQ_SET_DATE:
        case RTC_SEQ_SET_TIME:
        case RTC_SEQ_SET_DATETIME:
            // No processing in particular
            break;
            // Sequence for getting alarm ‚P's status
        case RTC_SEQ_GET_ALARM1_STATUS:
            {
                RTCAlarmStatus *pDst = (RTCAlarmStatus *)(rtcWork.buffer[0]);
                RTCRawStatus2 *pSrc =
                    &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

                switch (pSrc->intr_mode)
                {
                case RTC_INTERRUPT_MODE_ALARM:
                    *pDst = RTC_ALARM_STATUS_ON;
                    break;
                default:
                    *pDst = RTC_ALARM_STATUS_OFF;
                }
            }
            break;
            // Sequence for getting alarm ‚Q's status
        case RTC_SEQ_GET_ALARM2_STATUS:
            {
                RTCAlarmStatus *pDst = (RTCAlarmStatus *)(rtcWork.buffer[0]);
                RTCRawStatus2 *pSrc =
                    &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

                if (pSrc->intr2_mode)
                {
                    *pDst = RTC_ALARM_STATUS_ON;
                }
                else
                {
                    *pDst = RTC_ALARM_STATUS_OFF;
                }
            }
            break;
            // Sequence for getting alarm 1 or 2's setting values
        case RTC_SEQ_GET_ALARM_PARAM:
            {
                RTCAlarmParam *pDst = (RTCAlarmParam *)(rtcWork.buffer[0]);
                RTCRawAlarm *pSrc =
                    &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.alarm);

                pDst->week = (RTCWeek)(pSrc->week);
                pDst->hour = RtcBCD2HEX(pSrc->hour);
                pDst->minute = RtcBCD2HEX(pSrc->minute);
                pDst->enable = RTC_ALARM_ENABLE_NONE;
                if (pSrc->we)
                    pDst->enable += RTC_ALARM_ENABLE_WEEK;
                if (pSrc->he)
                    pDst->enable += RTC_ALARM_ENABLE_HOUR;
                if (pSrc->me)
                    pDst->enable += RTC_ALARM_ENABLE_MINUTE;
            }
            break;
            // Sequence for setting alarm ‚P's status
        case RTC_SEQ_SET_ALARM1_STATUS:
            if (rtcWork.index == 0)
            {
                RTCRawStatus2 *pSrc =
                    &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

                // Status 2 register read result
                if (*(RTCAlarmStatus *)(rtcWork.buffer[0]) == RTC_ALARM_STATUS_ON)
                {
                    // If interrupts are permitted
                    if (pSrc->intr_mode != RTC_INTERRUPT_MODE_ALARM)
                    {
                        // Write to the Status 2 register
                        rtcWork.index++;        // Go to next sequence
                        pSrc->intr_mode = RTC_INTERRUPT_MODE_ALARM;
                        if (!RTCi_WriteRawStatus2Async())
                        {
                            rtcWork.index = 0;  // Interrupt the sequence
                            result = RTC_RESULT_SEND_ERROR;
                        }
                    }
                }
                else
                {
                    // If interrupts are disabled
                    if (pSrc->intr_mode != RTC_INTERRUPT_MODE_NONE)
                    {
                        // Write to the Status 2 register
                        rtcWork.index++;        // Go to next sequence
                        pSrc->intr_mode = RTC_INTERRUPT_MODE_NONE;
                        if (!RTCi_WriteRawStatus2Async())
                        {
                            rtcWork.index = 0;  // Interrupt the sequence
                            result = RTC_RESULT_SEND_ERROR;
                        }
                    }
                }
            }
            else
            {
                // Status 2 register write result
                rtcWork.index = 0;     // End sequence
            }
            break;
            // Sequence for setting alarm ‚Q's status
        case RTC_SEQ_SET_ALARM2_STATUS:
            if (rtcWork.index == 0)
            {
                RTCRawStatus2 *pSrc =
                    &(((RTCRawData *)(OS_GetSystemWork()->real_time_clock))->a.status2);

                // Status 2 register read result
                if (*(RTCAlarmStatus *)(rtcWork.buffer[0]) == RTC_ALARM_STATUS_ON)
                {
                    // If interrupts are permitted
                    if (!pSrc->intr2_mode)
                    {
                        // Write to the Status 2 register
                        rtcWork.index++;        // Go to next sequence
                        pSrc->intr2_mode = 1;
                        if (!RTCi_WriteRawStatus2Async())
                        {
                            rtcWork.index = 0;  // Interrupt the sequence
                            result = RTC_RESULT_SEND_ERROR;
                        }
                    }
                }
                else
                {
                    // If interrupts are disabled
                    if (pSrc->intr2_mode)
                    {
                        // Write to the Status 2 register
                        rtcWork.index++;        // Go to next sequence
                        pSrc->intr2_mode = 0;
                        if (!RTCi_WriteRawStatus2Async())
                        {
                            rtcWork.index = 0;  // Interrupt the sequence
                            result = RTC_RESULT_SEND_ERROR;
                        }
                    }
                }
            }
            else
            {
                // Status 2 register write result
                rtcWork.index = 0;     // End sequence
            }
            break;
            // Sequence for setting alarm ‚P's parameters
        case RTC_SEQ_SET_ALARM1_PARAM:
            // Sequence for setting alarm ‚Q's parameters
        case RTC_SEQ_SET_ALARM2_PARAM:
            // Sequence for changing the time notation
        case RTC_SEQ_SET_HOUR_FORMAT:
            // Sequence for status 2 register writes
        case RTC_SEQ_SET_REG_STATUS2:
            // Sequence for adjust register writes
        case RTC_SEQ_SET_REG_ADJUST:
            // No processing in particular
            break;

            // Other unknown sequences 
        default:
#ifdef  SDK_TWL
            if (OS_IsRunOnTwl() == TRUE)
            {
                result  =   RTCEXi_CommonCallback();
                break;
            }
#endif
            result = RTC_RESULT_INVALID_COMMAND;
            rtcWork.index = 0;
        }
    }
    else
    {
        // A failure response was returned to the command, so the sequence is interrupted
        rtcWork.index = 0;
        // Determine the processing result from the classification of PXI communication responses
        switch (pxiresult)
        {
        case RTC_PXI_RESULT_INVALID_COMMAND:
            result = RTC_RESULT_INVALID_COMMAND;
            break;
        case RTC_PXI_RESULT_ILLEGAL_STATUS:
            result = RTC_RESULT_ILLEGAL_STATUS;
            break;
        case RTC_PXI_RESULT_BUSY:
            result = RTC_RESULT_BUSY;
            break;
        case RTC_PXI_RESULT_FATAL_ERROR:
        default:
            result = RTC_RESULT_FATAL_ERROR;
        }
    }

    // Sequence termination processing if a continuous sequence completes
    if (rtcWork.index == 0)
    {
        // Open exclusive lock
        if (rtcWork.lock != RTC_LOCK_OFF)
        {
            rtcWork.lock = RTC_LOCK_OFF;
        }
        // Invoke the callback function.
        if (rtcWork.callback)
        {
            cb = rtcWork.callback;
            rtcWork.callback = NULL;
            cb(result, rtcWork.callbackArg);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         RtcBCD2HEX

  Description:  Convert a numeric value expressed in the BCD type to a numeric value expressed as a general u32.

  Arguments:    bcd: Numeric value expressed in the BCD type

  Returns:      u32: Numeric value expressed as a general u32.
                       Return 0 if the input parameter is not of the BCD type.
 *---------------------------------------------------------------------------*/
static u32 RtcBCD2HEX(u32 bcd)
{
    u32     hex = 0;
    s32     i;
    s32     w;

    // Verify that 0xA-0xF is not included in any of the digits
    for (i = 0; i < 8; i++)
    {
        if (((bcd >> (i * 4)) & 0x0000000f) >= 0x0a)
        {
            return hex;                // Stop conversion and forcibly return 0
        }
    }

    // Conversion loop
    for (i = 0, w = 1; i < 8; i++, w *= 10)
    {
        hex += (((bcd >> (i * 4)) & 0x0000000f) * w);
    }
    return hex;
}

/*---------------------------------------------------------------------------*
  Name:         RtcHEX2BCD

  Description:  Convert a numeric value expressed as a general u32 to a numeric value expressed in the BCD type.

  Arguments:    hex:   Numeric value expressed as a general u32

  Returns:      u32:   Numeric value expressed in the BCD type.
                       Return 0 if the input parameter cannot be expressed in the BCD type.
 *---------------------------------------------------------------------------*/
static u32 RtcHEX2BCD(u32 hex)
{
    u32     bcd = 0;
    s32     i;
    u32     w;

    // Make sure that this is not over 99999999
    if (hex > 99999999)
    {
        return 0;
    }

    // Conversion loop
    for (i = 0, w = hex; i < 8; i++)
    {
        bcd += ((w % 10) << (i * 4));
        w /= 10;
    }
    return bcd;
}

/*---------------------------------------------------------------------------*
  Name:         RtcCheckAlarmParam

  Description:  Checks if an alarm setting value is one that will not cause problems when it is set in the RTC.

  Arguments:    param: Alarm setting value to check

  Returns:      BOOL:        Returns TRUE if, as an alarm setting value, there are no problems. Returns FALSE if there is some sort of problem.
                         
 *---------------------------------------------------------------------------*/
static BOOL RtcCheckAlarmParam(const RTCAlarmParam *param)
{
    if (param->week >= RTC_WEEK_MAX)
        return FALSE;
    if (param->hour >= 24)
        return FALSE;
    if (param->minute >= 60)
        return FALSE;
    if (param->enable & ~RTC_ALARM_ENABLE_ALL)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         RtcMakeAlarmParam

  Description:  Converts an alarm setting value into a format that can be set in the RTC.

  Arguments:    param:  Target alarm setting value to convert

  Returns:      RTCRawAlarm:  Data converted into a format that can be set in the RTC.
 *---------------------------------------------------------------------------*/
static RTCRawAlarm RtcMakeAlarmParam(const RTCAlarmParam *param)
{
    RTCRawAlarm dst;

    // Zero out the return value
    *((u32 *)(&dst)) = 0;

    // Just to be sure, check the compatibility of the setting value
    if (!RtcCheckAlarmParam(param))
    {
        return dst;
    }

    // Data for the day of the week
    dst.week = (u32)(param->week);
    // Time data as well as an AM-PM flag
    if (param->hour >= 12)
    {
        dst.afternoon = 1;
    }
    dst.hour = RtcHEX2BCD(param->hour);
    // Minute data
    dst.minute = RtcHEX2BCD(param->minute);
    // Enable flag
    if (param->enable & RTC_ALARM_ENABLE_WEEK)
    {
        dst.we = 1;
    }
    if (param->enable & RTC_ALARM_ENABLE_HOUR)
    {
        dst.he = 1;
    }
    if (param->enable & RTC_ALARM_ENABLE_MINUTE)
    {
        dst.me = 1;
    }

    return dst;
}

/*---------------------------------------------------------------------------*
  Name:         RtcCheckDate

  Description:  Verifies if the date value can be set in the RTC without any problems.
                If there are no problems, edits the format to one that can be set in the RTC.

  Arguments:    date: Inputs the date to check
                raw: Outputs the data edited into a format that can be set in the RTC

  Returns:      BOOL:  Returns FALSE if there was a problem while checking.
 *---------------------------------------------------------------------------*/
static BOOL RtcCheckDate(const RTCDate *date, RTCRawDate *raw)
{
    // Check whether each member is in the allowable range
    if (date->year >= 100)
        return FALSE;
    if ((date->month < 1) || (date->month > 12))
        return FALSE;
    if ((date->day < 1) || (date->day > 31))
        return FALSE;
    if (date->week >= RTC_WEEK_MAX)
        return FALSE;

    // Edit to the raw data type
    //raw->year  = RtcHEX2BCD( date->year );    // Change to the following code because for some reason the value is not stored
    *(u32 *)raw = RtcHEX2BCD(date->year);
    raw->month = RtcHEX2BCD(date->month);
    raw->day = RtcHEX2BCD(date->day);
    raw->week = (u32)RTC_GetDayOfWeek((RTCDate*)date);    // Always recalculate the day of week, don't use the day of week from the input data
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         RtcCheckTime

  Description:  Verifies if the time value can be set in the RTC without any problems.
                If there are no problems, edits the format to one that can be set in the RTC.

  Arguments:    date: Inputs the time to check
                raw: Outputs the data edited into a format that can be set in the RTC

  Returns:      BOOL:  Returns FALSE if there was a problem while checking.
 *---------------------------------------------------------------------------*/
static BOOL RtcCheckTime(const RTCTime *time, RTCRawTime *raw)
{
    // Check whether each member is in the allowable range
    if (time->hour >= 24)
        return FALSE;
    if (time->minute >= 60)
        return FALSE;
    if (time->second >= 60)
        return FALSE;

    // Edit to the raw data type
    if (time->hour >= 12)
    {
        raw->afternoon = 1;
    }
    else
    {
        raw->afternoon = 0;
    }
    raw->hour = RtcHEX2BCD(time->hour);
    raw->minute = RtcHEX2BCD(time->minute);
    raw->second = RtcHEX2BCD(time->second);

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         RtcGetResultCallback

  Description:  Called when asynchronous processing completes. Updates the processing results of internal variables.

  Arguments:    result: The processing results from the asynchronous function
                arg: Not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void RtcGetResultCallback(RTCResult result, void *arg)
{
#pragma unused( arg )

    rtcWork.commonResult = result;
}

#include    <nitro/code32.h>
/*---------------------------------------------------------------------------*
  Name:         RtcWaitBusy

  Description:  Wait while the RTC's asynchronous processing is locked.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
RtcWaitBusy( void )
{
    ldr     r12,    =rtcWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #RTC_LOCK_ON
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
