/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - include
  File:     api.h

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

#ifndef NITRO_RTC_ARM9_API_H_
#define NITRO_RTC_ARM9_API_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/rtc/common/type.h>
#include    <nitro/rtc/common/fifo.h>
#include    <nitro/pxi.h>


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/

// Day of the Week Definitions
typedef enum RTCWeek
{
    RTC_WEEK_SUNDAY = 0,               // Sunday
    RTC_WEEK_MONDAY,                   // Monday
    RTC_WEEK_TUESDAY,                  // Tuesday
    RTC_WEEK_WEDNESDAY,                // Wednesday
    RTC_WEEK_THURSDAY,                 // Thursday
    RTC_WEEK_FRIDAY,                   // Friday
    RTC_WEEK_SATURDAY,                 // Saturday
    RTC_WEEK_MAX
}
RTCWeek;

// Alarm Channel Definitions
typedef enum RTCAlarmChan
{
    RTC_ALARM_CHAN_1 = 0,              // Interrupt channel 1
    RTC_ALARM_CHAN_2,                  // Interrupt channel 2
    RTC_ALARM_CHAN_MAX
}
RTCAlarmChan;

// Alarm Status Definitions
typedef enum RTCAlarmStatus
{
    RTC_ALARM_STATUS_OFF = 0,          // Interrupts disabled status
    RTC_ALARM_STATUS_ON,               // Interrupts enabled status
    RTC_ALARM_STATUS_MAX
}
RTCAlarmStatus;

// Alarm Enable Flag Definitions
#define     RTC_ALARM_ENABLE_NONE       0x0000  // No enabled flag
#define     RTC_ALARM_ENABLE_WEEK       0x0001  // Use day of week setting in interrupt determination
#define     RTC_ALARM_ENABLE_HOUR       0x0002  // Use hour setting in interrupt determination
#define     RTC_ALARM_ENABLE_MINUTE     0x0004  // Use minute setting in interrupt determination
#define     RTC_ALARM_ENABLE_ALL        ( RTC_ALARM_ENABLE_WEEK | RTC_ALARM_ENABLE_HOUR | RTC_ALARM_ENABLE_MINUTE )

// Processing Result Definitions
typedef enum RTCResult
{
    RTC_RESULT_SUCCESS = 0,
    RTC_RESULT_BUSY,
    RTC_RESULT_ILLEGAL_PARAMETER,
    RTC_RESULT_SEND_ERROR,
    RTC_RESULT_INVALID_COMMAND,
    RTC_RESULT_ILLEGAL_STATUS,
    RTC_RESULT_FATAL_ERROR,
    RTC_RESULT_MAX
}
RTCResult;


/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Callback Function Type Definitions
typedef void (*RTCCallback) (RTCResult result, void *arg);
typedef void (*RTCInterrupt) (void);

// Date structure
typedef struct RTCDate
{
    u32     year;                      // Year ( 0 to 99 )
    u32     month;                     // Month ( 1 to 12 )
    u32     day;                       // Day ( 1 to 31 )
    RTCWeek week;                      // Day of the week

}
RTCDate;

// Time structure
typedef struct RTCTime
{
    u32     hour;                      // Hour ( 0 to 23 )
    u32     minute;                    // Minute ( 0 to 59 )
    u32     second;                    // Second ( 0 to 59 )

}
RTCTime;

// Time structure with an added millisecond field
typedef struct RTCTimeEx
{
    u32     hour;                      // Hour ( 0 to 23 )
    u32     minute;                    // Minute ( 0 to 59 )
    u32     second;                    // Second ( 0 to 59 )
    u32     millisecond;               // Millisecond (0 to 999)
}
RTCTimeEx;

// Alarm parameter structure
typedef struct RTCAlarmParam
{
    RTCWeek week;                      // Day of the week
    u32     hour;                      // Hour ( 0 to 23 )
    u32     minute;                    // Minute ( 0 to 59 )
    u32     enable;                    // Alarm enable flag ( RTC_ALARM_ENABLE_* )

}
RTCAlarmParam;


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         RTC_Init

  Description:  Initializes the RTC library.
       Notice:  A power-on check is performed when initializing the component side.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    RTC_Init(void);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDate

  Description:  Reads date data from the RTC.

  Arguments:    date      - Specifies the buffer for storing date data.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDate(RTCDate *date);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetTime

  Description:  Reads time data from the RTC.

  Arguments:    time      - Specifies the buffer for storing time data.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetTime(RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTime

  Description:  Read date and time data from RTC.

  Arguments:    date      - Specifies the buffer for storing date data.
                time      - Specifies the buffer for storing time data.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTime(RTCDate *date, RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDate

  Description:  Writes date data to the RTC.

  Arguments:    date      - Specifies the buffer where date data is stored.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDate(const RTCDate *date);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetTime

  Description:  Writes time data to the RTC.

  Arguments:    time      - Specifies the buffer where time data is stored.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetTime(const RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateTime

  Description:  Writes date and time data to the RTC.

  Arguments:    date      - Specifies the buffer where date data is stored.
                time      - Specifies the buffer where time data is stored.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDateTime(const RTCDate *date, const RTCTime *time);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmStatus

  Description:  Reads alarm ON/OFF status from the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                status    - Specifies a buffer for storing alarm status.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmStatus(RTCAlarmChan chan, RTCAlarmStatus *status);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmParam

  Description:  Reads alarm setting values from the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                param     - Specifies a buffer for storing alarm setting values.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmParam(RTCAlarmChan chan, RTCAlarmParam *param);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmStatus

  Description:  Writes alarm status to the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                status    - Specifies the buffer where alarm status is stored.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmStatus(RTCAlarmChan chan, const RTCAlarmStatus *status);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmParam

  Description:  Writes alarm setting values to the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                param     - Specifies the buffer where alarm setting values are stored.

  Returns:      RTCResult - Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmParam(RTCAlarmChan chan, const RTCAlarmParam *param);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateAsync

  Description:  Asynchronously reads date data from the RTC.

  Arguments:    date      - Specifies the buffer for storing date data.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateAsync(RTCDate *date, RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetTimeAsync

  Description:  Asynchronously reads time data from the RTC.

  Arguments:    time      - Specifies the buffer for storing time data.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetTimeAsync(RTCTime *time, RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetDateTimeAsync

  Description:  Asynchronously reads date and time data from the RTC.

  Arguments:    date      - Specifies the buffer for storing date data.
                time      - Specifies the buffer for storing time data.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetDateTimeAsync(RTCDate *date, RTCTime *time, RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateAsync

  Description:  Asynchronously writes date data to the RTC.

  Arguments:    date      - Specifies the buffer where date data is stored.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDateAsync(const RTCDate *date, RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetTimeAsync

  Description:  Asynchronously writes time data to the RTC.

  Arguments:    time      - Specifies the buffer where time data is stored.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetTimeAsync(const RTCTime *time, RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetDateTimeAsync

  Description:  Asynchronously writes date and time data to the RTC.

  Arguments:    date      - Specifies the buffer where date data is stored.
                time      - Specifies the buffer where time data is stored.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetDateTimeAsync(const RTCDate *date, const RTCTime *time, RTCCallback callback,
                               void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmStatusAsync

  Description:  Asynchronously reads alarm ON/OFF status from the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                status    - Specifies a buffer for storing alarm status.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmStatusAsync(RTCAlarmChan chan, RTCAlarmStatus *status, RTCCallback callback,
                                  void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_GetAlarmParamAsync

  Description:  Asynchronously reads alarm setting values from the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                param     - Specifies a buffer for storing alarm setting values.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_GetAlarmParamAsync(RTCAlarmChan chan, RTCAlarmParam *param, RTCCallback callback,
                                 void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmInterrupt

  Description:  Sets the callback function for when an alarm interrupt is generated.

  Arguments:    interrupt - Specifies a callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    RTC_SetAlarmInterrupt(RTCInterrupt interrupt);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmStatusAsync

  Description:  Asynchronously writes alarm status to the RTC.

  Arguments:    chan      - Specifies an alarm channel.
                status    - Specifies the buffer where alarm status is stored.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmStatusAsync(RTCAlarmChan chan, const RTCAlarmStatus *status,
                                  RTCCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         RTC_SetAlarmParamAsync

  Description:  Asynchronously writes alarm settings to the RTC.
       Notice:  Write will fail if the RTC alarm status is not ON, because the device side will not accept write.
                  

  Arguments:    chan      - Specifies an alarm channel.
                param     - Specifies the buffer where alarm setting values are stored.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      RTCResult - Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult RTC_SetAlarmParamAsync(RTCAlarmChan chan, const RTCAlarmParam *param,
                                 RTCCallback callback, void *arg);


/*---------------------------------------------------------------------------*
    Definitions of Private Functions
 *---------------------------------------------------------------------------*/
BOOL    RTCi_ResetAsync(void);
BOOL    RTCi_SetHourFormatAsync(void);
BOOL    RTCi_ReadRawDateTimeAsync(void);
BOOL    RTCi_WriteRawDateTimeAsync(void);
BOOL    RTCi_ReadRawDateAsync(void);
BOOL    RTCi_WriteRawDateAsync(void);
BOOL    RTCi_ReadRawTimeAsync(void);
BOOL    RTCi_WriteRawTimeAsync(void);
BOOL    RTCi_ReadRawPulseAsync(void);
BOOL    RTCi_WriteRawPulseAsync(void);
BOOL    RTCi_ReadRawAlarm1Async(void);
BOOL    RTCi_WriteRawAlarm1Async(void);
BOOL    RTCi_ReadRawAlarm2Async(void);
BOOL    RTCi_WriteRawAlarm2Async(void);
BOOL    RTCi_ReadRawStatus1Async(void);
BOOL    RTCi_WriteRawStatus1Async(void);
BOOL    RTCi_ReadRawStatus2Async(void);
BOOL    RTCi_WriteRawStatus2Async(void);
BOOL    RTCi_ReadRawAdjustAsync(void);
BOOL    RTCi_WriteRawAdjustAsync(void);
BOOL    RTCi_ReadRawFreeAsync(void);
BOOL    RTCi_WriteRawFreeAsync(void);
RTCResult RTCi_SetRegStatus2Async(const RTCRawStatus2 *status2, RTCCallback callback, void *arg);
RTCResult RTCi_SetRegAdjustAsync(const RTCRawAdjust *adjust, RTCCallback callback, void *arg);
RTCResult RTCi_SetRegAdjust(const RTCRawAdjust *Adjust);
RTCResult RTCi_SetRegStatus2(const RTCRawStatus2 *status2);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_API_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
