/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     internal_ex.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <twl/rtc/common/fifo_ex.h>
#include    <nitro/pxi.h>
#include	"private.h"

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static BOOL     RtcexSendPxiCommand(u32 command);

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawCounterAsync

  Description:  Asynchronously gets the unprocessed Up Counter register setting value.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL
RTCEXi_ReadRawCounterAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_COUNTER);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawFoutAsync

  Description:  Asynchronously gets the unprocessed FOUT1/FOUT2 register setting values.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawFoutAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_FOUT);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawFoutAsync

  Description:  Asynchronously writes unprocessed FOUT1/FOUT2 setting values to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawFoutAsync(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_FOUT);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawAlarmEx1Async

  Description:  Asynchronously gets the unprocessed Extended Alarm 1 register setting value.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawAlarmEx1Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawAlarmEx1Async

  Description:  Asynchronously writes the unprocessed Extended Alarm 1 register to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawAlarmEx1Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX1);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_ReadRawAlarmEx2Async

  Description:  Asynchronously gets the unprocessed Extended Alarm 2 register setting value.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                Data will be stored in OS_GetSystemWork()->real_time_clock.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_ReadRawAlarmEx2Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_READ_ALARM_EX2);
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_WriteRawAlarmEx2Async

  Description:  Asynchronously writes the unprocessed Extended Alarm 2 register to the device.
                The notification of a response from ARM7 is done by the PXI callback (tag:RTC).
                The data in OS_GetSystemWork()->real_time_clock will be written.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
BOOL RTCEXi_WriteRawAlarmEx2Async(void)
{
    return RtcexSendPxiCommand(RTC_PXI_COMMAND_WRITE_ALARM_EX2);
}

/*---------------------------------------------------------------------------*
  Name:         RtcexSendPxiCommand

  Description:  Send specified command to ARM7 via PXI.

  Arguments:    None.

  Returns:      BOOL     - Returns TRUE if a send for PXI completed, and FALSE if send by PXI failed.
                           
 *---------------------------------------------------------------------------*/
static BOOL RtcexSendPxiCommand(u32 command)
{
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_RTC,
                               ((command << RTC_PXI_COMMAND_SHIFT) & RTC_PXI_COMMAND_MASK), 0))
    {
        return FALSE;
    }
    return TRUE;
}
