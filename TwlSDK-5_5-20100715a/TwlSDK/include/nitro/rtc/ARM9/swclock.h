/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - include
  File:     swclock.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date$
  $Rev$
  $Author: tokunaga_eiji $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_RTC_ARM9_SWCLOCK_H_
#define NITRO_RTC_ARM9_SWCLOCK_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include    <nitro/types.h>

/*---------------------------------------------------------------------------*
  Name:         RTC_InitSWClock

  Description:  Initializes the software clock.
                Synchronizes the value of the software clock with the current value of the RTC, and adds a sleep recovery callback function to re-synchronize the software clock with the RTC.
                This must be called before the RTC_GetDateTimeExFromSWClock and RTC_SyncSWClock functions.
                
                

  Arguments:    None.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_InitSWClock(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSWClockTick

  Description:  Returns the time duration from 2000/01/01 00:00 until now as a converted tick value that is the sum of the current tick value and the RTC check converted tick value at boot time.
                

  Arguments:    None.

  Returns:      OSTick - The converted tick value for the time duration from 2000/01/01 00:00 until now.
 *---------------------------------------------------------------------------*/
OSTick RTC_GetSWClockTick(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetSyncSWClockResult

  Description:  Returns the result of calling RTC_GetDateTime the last time the software clock was synchronized.
                

  Arguments:    None.

  Returns:      RTCResult - The result of calling RTC_GetDateTime when synchronization was performed.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetLastSyncSWClockResult(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeExFromSWClock

  Description:  Loads date and time data (in milliseconds) from the software clock that uses CPU ticks.
                

  Arguments:    date      - Specifies the buffer for storing date data.
                time      - Specifies the buffer for storing time data.

  Returns:      RTCResult - Returns the RTCResult that was saved during the last synchronization.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeExFromSWClock(RTCDate *date, RTCTimeEx *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_SyncSWClock

  Description:  Synchronizes the software clock that uses CPU ticks with the RTC.

  Arguments:    None.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SyncSWClock(void);

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_SWCLOCK_H_ */


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
