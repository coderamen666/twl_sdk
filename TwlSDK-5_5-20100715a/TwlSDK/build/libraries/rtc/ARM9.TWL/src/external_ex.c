/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - libraries
  File:     external_ex.c

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

#include    <nitro/os.h>
#include    <nitro/rtc.h>
#include    <twl/rtc/common/type_ex.h>
#include    <twl/rtc/ARM9/api_ex.h>
#include    "private.h"

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void     RtcexGetResultCallback(RTCResult result, void* arg);
static void     RtcexWaitBusy(u32* lock);

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetCounterAsync

  Description:  Asynchronously reads the Up Counter value from the RTC.

  Arguments:    count: Specifies the buffer to store the Up Counter value.
                callback: Specifies the function to be called when the asynchronous process is completed.
                arg: Specifies the argument used when calling the callback function.

  Returns:      RTCResult: Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetCounterAsync(u32* count, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(count);
    SDK_NULL_ASSERT(callback);

    /* Check lock */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* Send the Up Counter read command */
    w->sequence =   RTC_SEQ_GET_COUNTER;
    w->index    =   0;
    w->buffer[0]    =   (void*)count;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_ReadRawCounterAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetCounter

  Description:  Reads the Up Counter value from the RTC.

  Arguments:    count: Specifies the buffer to store the Up Counter value.

  Returns:      RTCResult: Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetCounter(u32* count)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_GetCounterAsync(count, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetFoutAsync

  Description:  Asynchronously reads FOUT setting values from the RTC.

  Arguments:    fout: Specifies a buffer for storing FOUT setting values.
                callback: Specifies the function to be called when the asynchronous process is completed.
                arg: Specifies the argument used when calling the callback function.

  Returns:      RTCResult: Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetFoutAsync(u16* fout, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(fout);
    SDK_NULL_ASSERT(callback);

    /* Check lock */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* Send the FOUT setting value read command */
    w->sequence =   RTC_SEQ_GET_FOUT;
    w->index    =   0;
    w->buffer[0]    =   (void*)fout;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_ReadRawFoutAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_GetFout

  Description:  Asynchronously reads FOUT setting values from the RTC.

  Arguments:    fout: Specifies a buffer for storing FOUT setting values.

  Returns:      RTCResult: Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_GetFout(u16* fout)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_GetFoutAsync(fout, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_SetFoutAsync

  Description:  Asynchronously writes FOUT settings to the RTC.

  Arguments:    fout: Specifies the buffer storing FOUT setting values.
                callback: Specifies the function to be called when the asynchronous process is completed.
                arg: Specifies the argument used when calling the callback function.

  Returns:      RTCResult: Returns the result of the process that starts the asynchronous device operation.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_SetFoutAsync(const u16* fout, RTCCallback callback, void* arg)
{
    OSIntrMode  enabled;
    RTCWork*    w   =   RTCi_GetSysWork();

    SDK_NULL_ASSERT(fout);
    SDK_NULL_ASSERT(callback);

    /* Check lock */
    enabled =   OS_DisableInterrupts();
    if (w->lock != RTC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return RTC_RESULT_BUSY;
    }
    w->lock =   RTC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    /* Edit the data to set. */
    ((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.fout.fout =   *fout;

    /* Send the FOUT setting value write command */
    w->sequence =   RTC_SEQ_SET_FOUT;
    w->index    =   0;
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (RTCEXi_WriteRawFoutAsync() == TRUE)
    {
        return RTC_RESULT_SUCCESS;
    }
    else
    {
        w->lock =   RTC_LOCK_OFF;
        return RTC_RESULT_SEND_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_SetFout

  Description:  Writes FOUT settings values to the RTC.

  Arguments:    fout: Specifies the buffer storing FOUT setting values.

  Returns:      RTCResult: Returns the device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_SetFout(const u16* fout)
{
    RTCWork*    w   =   RTCi_GetSysWork();

    w->commonResult =   RTCEXi_SetFoutAsync(fout, RtcexGetResultCallback, NULL);
    if (w->commonResult == RTC_RESULT_SUCCESS)
    {
        RtcexWaitBusy(&(w->lock));
    }
    return w->commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         RTCEXi_CommonCallback

  Description:  Shared callback function corresponding to additional commands used by asynchronous RTC functions.

  Arguments:    tag:   PXI tag that shows the message type.
                data:  Message from ARM7.
                err:   PXI transfer error flag.

  Returns:      RTCResult: Returns the asynchronous device operation processing result.
 *---------------------------------------------------------------------------*/
RTCResult
RTCEXi_CommonCallback(void)
{
    RTCWork*    w   =   RTCi_GetSysWork();
    RTCResult   result  =   RTC_RESULT_SUCCESS;

    /* Various types of postprocessing according to the internal status  */
    switch(w->sequence)
    {
    /* Sequence to get the Up Counter value  */
    case RTC_SEQ_GET_COUNTER:
        {
            u32*    pDst    =   (u32*)(w->buffer[0]);
            RTCRawCounter*  pSrc    =
                &(((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.counter);
            /* Change the endianness */
            *pDst   =   (u32)(pSrc->bytes[0] | (pSrc->bytes[1] << 8) | (pSrc->bytes[2] << 16));
        }
        break;
    /* Sequence to get the FOUT setting value */
    case RTC_SEQ_GET_FOUT:
        {
            u16*    pDst    =   (u16*)(w->buffer[0]);
            RTCRawFout* pSrc    =
                &(((RTCRawDataEx*)(OS_GetSystemWork()->real_time_clock))->a.fout);
            *pDst   =   (u16)(pSrc->fout);
        }
        break;
    /* Sequence to change the FOUT setting value  */
    case RTC_SEQ_SET_FOUT:
        /* No processing in particular. */
        break;
    default:
        /* Other unknown sequences  */
        w->index    =   0;
        result  =   RTC_RESULT_INVALID_COMMAND;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         RtcexGetResultCallback

  Description:  Called when asynchronous processing completes. Updates the processing results of internal variables.

  Arguments:    result: The processing results from the asynchronous function.
                arg: Not used.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
RtcexGetResultCallback(RTCResult result, void* arg)
{
#pragma unused(arg)

    RTCi_GetSysWork()->commonResult =   result;
}

#include    <nitro/code32.h>
/*---------------------------------------------------------------------------*
  Name:         RtcWaitBusy

  Description:  Wait while the RTC's asynchronous processing is locked.

  Arguments:    r0: Pointer to the lock control member in the library's shared work buffer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
RtcexWaitBusy(u32* lock)
{
loop:
    ldr     r1, [r0]
    cmp     r1, #RTC_LOCK_ON
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
