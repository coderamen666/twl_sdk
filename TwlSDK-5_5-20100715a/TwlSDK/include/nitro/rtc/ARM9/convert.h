/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - include
  File:     convert.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_RTC_ARM9_CONVERT_H_
#define NITRO_RTC_ARM9_CONVERT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro/types.h>

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         RTC_ConvertDateToDay

  Description:  Converts date data from RTCDate type to the total number of days since the year 2000.

  Arguments:    date      - Pointer to date data.

  Returns:      The total number of days since Jan. 1, 2000.
 *---------------------------------------------------------------------------*/
s32     RTC_ConvertDateToDay(const RTCDate *date);

/*---------------------------------------------------------------------------*
  Name:         RTCi_ConvertTimeToSecond

  Description:  Converts time data in RTCTime type to the total number of seconds since 0:00.

  Arguments:    time      - Pointer to the time data.

  Returns:      Total number of seconds since 0:00.
 *---------------------------------------------------------------------------*/
s32     RTCi_ConvertTimeToSecond(const RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_ConvertDateTimeToSecond

  Description:  Converts date and time data in RTCDate and RTCTime types to total number of seconds.

  Arguments:    date      - Pointer to date data.
                time      - Pointer to the time data.

  Returns:      Total number of seconds since 0:00, Jan. 1, 2000, local time.
 *---------------------------------------------------------------------------*/
s64     RTC_ConvertDateTimeToSecond(const RTCDate *date, const RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_ConvertDayToDate

  Description:  Converts the total number of days since the year 2000 to date data in RTCDate type.

  Arguments:    date      - Pointer to save destination date data.
                day       - The total number of days since Jan 1, 2000.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    RTC_ConvertDayToDate(RTCDate *date, s32 day);

/*---------------------------------------------------------------------------*
  Name:         RTCi_ConvertSecondToTime

  Description:  Converts the total number of seconds since 0:00 to time data in RTCTime type.

  Arguments:    time      - Pointer to save destination date data.
                sec       -Total number of seconds since 0:00.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    RTCi_ConvertSecondToTime(RTCTime *time, s32 sec);

/*---------------------------------------------------------------------------*
  Name:         RTC_ConvertDateTimeToSecond

  Description:  Converts total number of seconds to date and time data in RTCDate and RTCTime types.

  Arguments:    date      - Pointer to save destination date data.
                time      - Pointer to the save destination time data.
                sec       - Total number of seconds since 0:00 Jan 1, 2000.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    RTC_ConvertSecondToDateTime(RTCDate *date, RTCTime *time, s64 sec);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDayOfWeek

  Description:  Returns the day of the week based on RTCDate type data.
                (This is found based on a calculation rather than using the day of the week inside RTCDate.)

  Arguments:    date      - Pointer to date data.

  Returns:      Day of week      RTC_WEEK_xxxx
 *---------------------------------------------------------------------------*/
RTCWeek RTC_GetDayOfWeek(RTCDate *date);

/*===========================================================================*/

#ifdef __cplusplus
}          /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_CONVERT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
