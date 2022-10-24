/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mic
  File:     micex.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/os.h>
#include    <nitro/spi.h>
#include    "micex.h"

#include    <twl/spi/common/mic_common.h>

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void     MicexSyncCallback(MICResult result, void* arg);
static BOOL     MicexStartLimitedSampling(void* buf, u32 size, u32 rate, u8 flags);
static BOOL     MicexStopLimitedSampling(void);
static BOOL     MicexAdjustLimitedSampling(u32 rate);

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StartLimitedSampling
  Description:  Starts sampling rate limited microphone auto-sampling.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
  Arguments:    param:     - Pointer to a structure that specifies auto sampling settings
  Returns:      MICResult: Returns synchronous processing results.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StartLimitedSampling(const MICAutoParam* param)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from within an interrupt handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* Initialize the message queue for getting responses  */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback  */
    result  =   MICEXi_StartLimitedSamplingAsync(param, MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StartLimitedSamplingAsync
  Description:  Starts sampling rate limited microphone auto-sampling.
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
  Arguments:    param:     Pointer to a structure that specifies auto-sampling settings.
                callback: Specifies the callback function to call when asynchronous processing is completed.
                arg: Specifies parameter passed to the callback function.
  Returns:      MICResult: Returns the results of starting the asynchronous process.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();
    u8          flags;

    SDK_NULL_ASSERT(callback);
    SDK_NULL_ASSERT(param->buffer);

    /* Confirms parameters */
    {
        /* Buffer size */
        if (param->size <= 0)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Illegal desination buffer size. (%d)\n", __FUNCTION__, param->size);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* 32-byte buffer alignment */
        if (((u32)(param->buffer) % HW_CACHE_LINE_SIZE != 0) || ((param->size % HW_CACHE_LINE_SIZE) != 0))
        {
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Destination buffer (%p - %p) is not aligned on %d bytes boundary.\n",
                    __FUNCTION__, param->buffer, (void*)((u32)param->buffer + param->size), HW_CACHE_LINE_SIZE);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* AD conversion bit width */
        switch (param->type)
        {
        case MIC_SAMPLING_TYPE_8BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_8BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_12BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_8BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_S8BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT:
            flags   =   SPI_MIC_SAMPLING_TYPE_S12BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
            flags   =   (SPI_MIC_SAMPLING_TYPE_12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
            flags   =   (SPI_MIC_SAMPLING_TYPE_S12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        default:
#ifdef  SDK_DEBUG
            OS_TWarning("%s: Illegal sampling type. (%d)\n", __FUNCTION__, param->type);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* sampling period */
        switch (param->rate)
        {
        case MIC_SAMPLING_RATE_32730:   case MIC_SAMPLING_RATE_16360:
        case MIC_SAMPLING_RATE_10910:   case MIC_SAMPLING_RATE_8180:
        case MIC_SAMPLING_RATE_47610:   case MIC_SAMPLING_RATE_23810:
        case MIC_SAMPLING_RATE_15870:   case MIC_SAMPLING_RATE_11900:
            break;
        default:
#ifdef  SDK_TWL
            OS_TWarning("%s: Illegal sampling rate. (%d)\n", __FUNCTION__, param->rate);
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        /* Enable/disable loop */
        if (param->loop_enable)
        {
            flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_LOOP_MASK) | SPI_MIC_SAMPLING_TYPE_LOOP_ON);
        }
        else
        {
            flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_LOOP_MASK) | SPI_MIC_SAMPLING_TYPE_LOOP_OFF);
        }
        /* Correction flag for expansions  */
        flags   =   (u8)((flags & ~SPI_MIC_SAMPLING_TYPE_CORRECT_MASK) | SPI_MIC_SAMPLING_TYPE_CORRECT_OFF);
    }

    /* Check exclusion lock  */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* Send command for restricted auto-sampling start */
    w->callback =   callback;
    w->callbackArg  =   arg;
    w->full =   param->full_callback;
    w->fullArg  =   param->full_arg;
    if (MicexStartLimitedSampling(param->buffer, param->size, param->rate, flags))
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StopLimitedSampling
  Description:  Stops sampling rate limited microphone auto-sampling.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
  Arguments:    None.
  Returns:      MICResult: Returns synchronous processing results.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StopLimitedSampling(void)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from within an interrupt handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* Initialize the message queue for getting responses  */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback  */
    result  =   MICEXi_StopLimitedSamplingAsync(MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_StopLimitedSamplingAsync
  Description:  Stops sampling rate limited microphone auto-sampling.
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
  Arguments:    callback: Specifies the callback function to call when asynchronous processing is completed.
                arg: Specifies parameter passed to the callback function.
  Returns:      MICResult: Returns the results of starting the asynchronous process.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_StopLimitedSamplingAsync(MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();

    SDK_NULL_ASSERT(callback);

    /* Check exclusion lock  */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* Send command to stop restricted auto-sampling */
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (MicexStopLimitedSampling())
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_AdjustLimitedSampling
  Description:  Adjusts the sampling intervals used in sampling rate limited microphone auto-sampling.
                
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
  Arguments:    rate: Specifies the sampling interval in ARM7 CPU clock units.
  Returns:      MICResult: Returns synchronous processing results.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_AdjustLimitedSampling(u32 rate)
{
    MICResult       result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from within an interrupt handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Could not process in exception handler.\n", __FUNCTION__);
#endif
        return MIC_RESULT_ILLEGAL_STATUS;
    }

    /* Initialize the message queue for getting responses  */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback  */
    result  =   MICEXi_AdjustLimitedSamplingAsync(rate, MicexSyncCallback, (void*)(&msgQ));
    if (result == MIC_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MICEXi_AdjustLimitedSamplingAsync
  Description:  Adjusts the sampling intervals used in sampling rate limited microphone auto-sampling.
                
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
  Arguments:    rate: Specifies the sampling interval in ARM7 CPU clock units.
                callback: Specifies the callback function to call when asynchronous processing is completed.
                arg: Specifies parameter passed to the callback function.
  Returns:      MICResult: Returns the results of starting the asynchronous process.
 *---------------------------------------------------------------------------*/
MICResult
MICEXi_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg)
{
    OSIntrMode  e;
    MICWork*    w   =   MICi_GetSysWork();

    SDK_NULL_ASSERT(callback);

    /* Confirms parameters */
    switch (rate)
    {
    case MIC_SAMPLING_RATE_32730:   case MIC_SAMPLING_RATE_16360:
    case MIC_SAMPLING_RATE_10910:   case MIC_SAMPLING_RATE_8180:
    case MIC_SAMPLING_RATE_47610:   case MIC_SAMPLING_RATE_23810:
    case MIC_SAMPLING_RATE_15870:   case MIC_SAMPLING_RATE_11900:
        break;
    default:
#ifdef  SDK_DEBUG
        OS_TWarning("%s: Illegal sampling rate. (%d)\n", __FUNCTION__, rate);
#endif
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    /* Check exclusion lock  */
    e   =   OS_DisableInterrupts();
    if (w->lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(e);
        return MIC_RESULT_BUSY;
    }
    w->lock =   MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(e);

    /* Send command to adjust restricted auto-sampling */
    w->callback =   callback;
    w->callbackArg  =   arg;
    if (MicexAdjustLimitedSampling(rate))
    {
        return MIC_RESULT_SUCCESS;
    }
    w->lock =   MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MicexSyncCallback
  Description:  Shared callback function for synchronous functions. Makes inactive threads recover to an operable state by sending them the asynchronous processing results. 
                
  Arguments:    result: Passes the microphone operation asynchronous processing results.
                arg: Passes a pointer to the message queue which the thread which calls synchronous functions is waiting to receive.
                            
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
MicexSyncCallback(MICResult result, void* arg)
{
    SDK_NULL_ASSERT(arg);

    (void)OS_SendMessage((OSMessageQueue*)arg, (OSMessage)result, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         MicexStartLimitedSampling
  Description:  Issues start command for restricted auto-sampling to ARM7.
  Arguments:    buf: Address of buffer storing sampled data.
                size: Specifies the buffer size in bytes.
                rate: Specifies the sampling interval in ARM7 CPU clock units.
                flags - Specifies the AD conversion bit width, enabling/disabling of loops, and enabling/disabling of correction
  Returns:      BOOL: Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                            
 *---------------------------------------------------------------------------*/
static BOOL
MicexStartLimitedSampling(void* buf, u32 size, u32 rate, u8 flags)
{
    /* PXI Packet [0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_ON << 8) | (u32)flags, 0))
    {
        return FALSE;
    }

    /* PXI Packet [1] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (1 << SPI_PXI_INDEX_SHIFT) | ((u32)buf >> 16), 0))
    {
        return FALSE;
    }

    /* PXI Packet [2] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (2 << SPI_PXI_INDEX_SHIFT) | ((u32)buf & 0x0000ffff), 0))
    {
        return FALSE;
    }

    /* PXI Packet [3] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (3 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    /* PXI Packet [4] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (4 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    /* PXI Packet [5] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (5 << SPI_PXI_INDEX_SHIFT) | (rate >> 16), 0))
    {
        return FALSE;
    }

    /* PXI Packet [6] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_END_BIT | (6 << SPI_PXI_INDEX_SHIFT) | (rate & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicexStopLimitedSampling
  Description:  Issues stop command for restricted auto-sampling to ARM7.
  Arguments:    None.
  Returns:      BOOL: Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                            
 *---------------------------------------------------------------------------*/
static BOOL
MicexStopLimitedSampling(void)
{
    /* PXI Packet [0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | SPI_PXI_END_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_OFF << 8), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicexAdjustLimitedSampling
  Description:  Issues change command for restricted auto-sampling to ARM7.
  Arguments:    rate: Specifies the sampling interval in ARM7 CPU clock units.
  Returns:      BOOL: Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                            
 *---------------------------------------------------------------------------*/
static BOOL
MicexAdjustLimitedSampling(u32 rate)
{
    /* PXI Packet [0] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_START_BIT | (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_LTDAUTO_ADJUST << 8), 0))
    {
        return FALSE;
    }

    /* PXI Packet [1] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            (1 << SPI_PXI_INDEX_SHIFT) | (rate >> 16), 0))
    {
        return FALSE;
    }

    /* PXI Packet [2] */
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
            SPI_PXI_END_BIT | (2 << SPI_PXI_INDEX_SHIFT) | (rate & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}
