/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     swclock.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date: 2008-01-11#$
  $Rev: 9449 $
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#include <nitro/os.h>
#include <nitro/rtc.h>
#include <nitro/spi/ARM9/pm.h>

/*---------------------------------------------------------------------------*
    Static Variable Definitions
 *---------------------------------------------------------------------------*/
static u16       rtcSWClockInitialized;       // Tick initialized verify flag
static OSTick    rtcSWClockBootTicks;         // Boot-time tick-converted RTC value
static RTCResult rtcLastResultOfSyncSWClock;  // RTCResult at final synchronization
static PMSleepCallbackInfo rtcSWClockSleepCbInfo; // Callback information when recovering from sleep 

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/

static void RtcGetDateTimeExFromSWClock(RTCDate * date, RTCTimeEx *time);
static void RtcSleepCallbackForSyncSWClock(void* args);

/*---------------------------------------------------------------------------*
  Name:         RTC_InitSWClock

  Description:  Initializes the software clock. Synchronizes the software clock value with the current RTC value, and adds a function to the sleep recovery callbacks that resynchronizes the RTC value and software clock. 
                
                
                It is necessary to call this function before calling the RTC_GetDateTimeExFromSWClock or RTC_SyncSWClock functions. 
                

  Arguments:    None.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_InitSWClock(void)
{
    SDK_ASSERT(OS_IsTickAvailable());
    
    // If already initialized, do nothing and return RTC_RESULT_SUCCESS 
    if(rtcSWClockInitialized)
    {
        return RTC_RESULT_SUCCESS;
    }
    
    // Synchronize the software clock value with the current RTC.
    (void) RTC_SyncSWClock();

    // Register sleep recovery callbacks.
    PM_SetSleepCallbackInfo(&rtcSWClockSleepCbInfo, RtcSleepCallbackForSyncSWClock, NULL);
    PM_AppendPostSleepCallback(&rtcSWClockSleepCbInfo);
    
    rtcSWClockInitialized = TRUE;

    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSWClockTicks

  Description:  Totals the current tick value and the boot-time tick-converted RTC value, and returns the length of time between January 1 2000 00:00:00 and the current time as a tick conversion value. 
                

  Arguments:    None.

  Returns:      OSTick: Tick conversion time from January 1 2000 00:00:00 to the current time
                         If the most recent synchronization failed, this is 0.
 *---------------------------------------------------------------------------*/
OSTick RTC_GetSWClockTick(void)
{
    if(rtcLastResultOfSyncSWClock == RTC_RESULT_SUCCESS)
    {
        return OS_GetTick() + rtcSWClockBootTicks;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTC_GetLastSyncSWClockResult

  Description:  Returns the result of the RTC_GetDateTime called during the most recent software clock synchronization.
                

  Arguments:    None.

  Returns:      RTCResult: Result of the RTC_GetDateTime called at synchronization
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetLastSyncSWClockResult(void)
{
    return rtcLastResultOfSyncSWClock;
}


/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeExFromSWClock

  Description:  Reads date and time data up to the millisecond from the software clock (which uses CPU ticks)..
                

  Arguments:    date: Specifies the buffer for storing date data.
                time: Specifies the buffer for storing time data.

  Returns:      RTCResult: Returns the RTCResult that was saved during the most recent synchronization
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeExFromSWClock(RTCDate *date, RTCTimeEx *time)
{
    SDK_NULL_ASSERT(date);
    SDK_NULL_ASSERT(time);

    RtcGetDateTimeExFromSWClock(date, time);
    
    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RTC_SyncSWClock

  Description:  Synchronizes the software clock (which uses CPU ticks) to the RTC.
                The result of the RTC_GetDateTime called when synchronizing is saved and returned as the return value of RTC_GetDateTimeExFromSWClock. 
                

  Arguments:    None.

  Returns:      RTCResult: Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SyncSWClock(void)
{   
    RTCDate currentDate;
    RTCTime currentTime;
    
    // Save the result of the RTC_GetDateTime.
    rtcLastResultOfSyncSWClock = RTC_GetDateTime(&currentDate, &currentTime);
    // Convert the current RTC to ticks, take the difference compared to the current CPU tick, and save the boot-time tick-converted RTC value.    
    rtcSWClockBootTicks = OS_SecondsToTicks(RTC_ConvertDateTimeToSecond(&currentDate, &currentTime)) - OS_GetTick();

    return rtcLastResultOfSyncSWClock;
}

/*---------------------------------------------------------------------------*
  Name:         RtcGetDateTimeExFromSWClock

  Description:  Calculates date and time data from the tick difference saved when the current ticks and software clock were synchronized to RTC.
                

  Arguments:    date: Buffer for storing RTCDate
                time: Buffer for storing RTCTimeEx

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void RtcGetDateTimeExFromSWClock(RTCDate * date, RTCTimeEx *time)
{
    OSTick currentTicks;
    s64 currentSWClockSeconds;

    currentTicks = RTC_GetSWClockTick();
    currentSWClockSeconds = (s64) OS_TicksToSeconds(currentTicks);
        
    // Convert the current time in seconds (not including milliseconds) into RTCDate and RTCTime.
    RTC_ConvertSecondToDateTime(date, (RTCTime*)time, currentSWClockSeconds);

    // Add milliseconds separately 
    time->millisecond = (u32) (OS_TicksToMilliSeconds(currentTicks) % 1000);  
}

/*---------------------------------------------------------------------------*
  Name:         RtcSleepCallbackForSyncSWClock

  Description:  Sleep callback registered when RTC_InitSWClock is called.
                Synchronizes the software clock to the RTC when recovering from sleep.

  Arguments:    args: Unused argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define RTC_SWCLOCK_SYNC_RETRY_INTERVAL   1   // Units are milliseconds
static void RtcSleepCallbackForSyncSWClock(void* args)
{
#pragma unused(args)
    for (;;)
    {
        (void) RTC_SyncSWClock();

        // If the RTC is being processed by another thread or by the ARM7, or if the PXI FIFO is FULL, wait RTC_SWCLOCK_SYNC_RETRY_INTERVAL milliseconds and retry.
        // 
        if(rtcLastResultOfSyncSWClock != RTC_RESULT_BUSY &&
           rtcLastResultOfSyncSWClock != RTC_RESULT_SEND_ERROR)
        {
            break;
        }

        OS_TWarning("RTC_SyncSWClock() failed at sleep callback. Retry... \n");
        OS_Sleep(RTC_SWCLOCK_SYNC_RETRY_INTERVAL);
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
