/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     internal.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author:$
 *---------------------------------------------------------------------------*/

#include	<nitro/rtc/ARM9/api.h>
#include	<nitro/pxi.h>


/*---------------------------------------------------------------------------*
	Internal Function Definitions
 *---------------------------------------------------------------------------*/
static BOOL RtcSendPxiCommand(u32 command);


/*---------------------------------------------------------------------------*
  Name:         RTCi_ResetAsync

  Description:  Asynchronously reset RTC.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ResetAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_RESET);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_SetHourFormatAsync

  Description:  Asynchronously change the time format.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Sets arguments with status1 in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_SetHourFormatAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_SET_HOUR_FORMAT);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawDateTimeAsync

  Description:  Asynchronously obtain unprocessed date/time data.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawDateTimeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_DATETIME);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawDateTimeAsync

  Description:  Asynchronously write unprocessed date/time data to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawDateTimeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_DATETIME);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawDateAsync

  Description:  Asynchronously obtain unprocessed date data.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawDateAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_DATE);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawDateAsync

  Description:  Asynchronously write unprocessed date data to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawDateAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_DATE);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawTimeAsync

  Description:  Asynchronously obtain unprocessed time data.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawTimeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_TIME);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawTimeAsync

  Description:  Asynchronously write unprocessed time data to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawTimeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_TIME);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawPulseAsync

  Description:  Asynchronously obtain steady interrupt setting values for unprocessed frequencies.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawPulseAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_PULSE);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawPulseAsync

  Description:  Asynchronously write steady interrupt setting values for unprocessed frequencies to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawPulseAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_PULSE);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawAlarm1Async

  Description:  Asynchronously obtains unprocessed alarm 1 interrupt setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawAlarm1Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawAlarm1Async

  Description:  Asynchronously writes unprocessed alarm 1 interrupt setting values to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawAlarm1Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawAlarm2Async

  Description:  Asynchronously obtains unprocessed alarm 2 interrupt setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawAlarm2Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawAlarm2Async

  Description:  Asynchronously writes unprocessed alarm 2 interrupt setting values to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawAlarm2Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawStatus1Async

  Description:  Asynchronously obtains unprocessed status 1 register setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawStatus1Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_STATUS1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawStatus1Async

  Description:  Asynchronously writes unprocessed status 1 register setting values to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawStatus1Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_STATUS1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawStatus2Async

  Description:  Asynchronously obtains unprocessed status 2 register setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawStatus2Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_STATUS2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawStatus2Async

  Description:  Asynchronously writes unprocessed status 2 register setting values to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawStatus2Async(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_STATUS2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawAdjustAsync

  Description:  Asynchronously obtain unprocessed clock adjustment register setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawAdjustAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_ADJUST);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawAdjustAsync

  Description:  Asynchronously write unprocessed clock adjustment register setting values to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawAdjustAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_ADJUST);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_ReadRawFreeAsync

  Description:  Asynchronously obtain unprocessed free register setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_ReadRawFreeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_READ_FREE);
}

/*---------------------------------------------------------------------------*
  Name:         RTCi_WriteRawFreeAsync

  Description:  Asynchronously write unprocessed free register setting values to device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCi_WriteRawFreeAsync(void)
{
    return RtcSendPxiCommand(RTC_PXI_COMMAND_WRITE_FREE);
}

/*---------------------------------------------------------------------------*
  Name:         RtcSendPxiCommand

  Description:  Send specified command to ARM7 via PXI.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
static BOOL RtcSendPxiCommand(u32 command)
{
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_RTC,
                               ((command << RTC_PXI_COMMAND_SHIFT) & RTC_PXI_COMMAND_MASK), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
