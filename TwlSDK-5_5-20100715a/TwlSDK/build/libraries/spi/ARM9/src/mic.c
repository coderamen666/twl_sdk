/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - libraries
  File:     mic.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/spi.h>
#include    <nitro/os/common/systemWork.h>

#ifdef  SDK_TWL
#include    <twl/os/common/codecmode.h>
#include    "micex.h"
#endif

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
#ifndef SDK_TWL
// Lock definition for exclusive processing of asynchronous functions
typedef enum MICLock
{
    MIC_LOCK_OFF = 0,                  // Unlock status
    MIC_LOCK_ON,                       // Lock status
    MIC_LOCK_MAX
}
MICLock;

// Work structure
typedef struct MICWork
{
    MICLock lock;                      // Exclusive lock
    MICCallback callback;              // For saving an asynchronous callback function
    void   *callbackArg;               // For saving arguments to the callback function
    MICResult commonResult;            // For saving asynchronous function processing results
    MICCallback full;                  // For saving the sampling completion callback
    void   *fullArg;                   // For saving arguments to the completion callback function
    void   *dst_buf;                   // For saving a storage area for individual sampling results

}
MICWork;
#endif

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 micInitialized;             // Initialized verify flag
static MICWork micWork;                // Structure that combines work variables


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void MicCommonCallback(PXIFifoTag tag, u32 data, BOOL err);
static BOOL MicDoSampling(u16 type);
static BOOL MicStartAutoSampling(void *buf, u32 size, u32 span, u8 flags);
static BOOL MicStopAutoSampling(void);
static BOOL MicAdjustAutoSampling(u32 span);
static void MicGetResultCallback(MICResult result, void *arg);
static void MicWaitBusy(void);


/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  Initializes microphone library

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MIC_Init(void)
{
    // Verification of non-initialization
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // Work variable initialization
    micWork.lock = MIC_LOCK_OFF;
    micWork.callback = NULL;

    // Wait until ARM7 MIC library starts
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_MIC, PXI_PROC_ARM7))
    {
    }

    // Clear the shared area storage address for the latest sampling result
    OS_GetSystemWork()->mic_last_address = 0;

    // Set the PXI callback function.
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_MIC, MicCommonCallback);
}

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSamplingAsync

  Description:  Performs a single asynchronous sample of the microphone.

  Arguments:    type:      - Specifies the sampling type.
                buf:       - Specifies the buffer that stores the sampling data.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSamplingAsync(MICSamplingType type, void *buf, MICCallback callback, void *arg)
{
    OSIntrMode enabled;
    u16     wtype;

    SDK_NULL_ASSERT(buf);
    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (type >= MIC_SAMPLING_TYPE_MAX)
    {
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_8BIT;
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_12BIT;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_S8BIT;
        break;
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
        wtype = SPI_MIC_SAMPLING_TYPE_S12BIT;
        break;
    default:
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Sends the sampling run command
    micWork.callback = callback;
    micWork.callbackArg = arg;
    micWork.dst_buf = buf;
    if (MicDoSampling(wtype))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSampling

  Description:  Samples the microphone once.

  Arguments:    type:      - Specifies the sampling type.
                buf:       - Specifies the buffer that stores the sampling data.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSampling(MICSamplingType type, void *buf)
{
    micWork.commonResult = MIC_DoSamplingAsync(type, buf, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSamplingAsync

  Description:  Asynchronously starts microphone auto sampling

  Arguments:    param:     Pointer to a structure that specifies auto-sampling settings.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSamplingAsync(const MICAutoParam *param, MICCallback callback, void *arg)
{
    OSIntrMode enabled;
    u8      flags;

    SDK_NULL_ASSERT(callback);
    SDK_NULL_ASSERT(param->buffer);

    // Confirms parameters
    {
        // 32-byte buffer alignment
        if ((u32)(param->buffer) & 0x01f)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("Parameter param->buffer must be 32-byte aligned.\n");
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // Buffer size alignment
        if (param->size & 0x01f)
        {
#ifdef  SDK_DEBUG
            OS_TWarning("Parameter param->size must be a multiple of 32-byte.\n");
#endif
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // Buffer size
        if (param->size <= 0)
        {
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // sampling period
        if (param->rate < MIC_SAMPLING_RATE_LIMIT)
        {
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // AD conversion bit width
        switch (param->type)
        {
        case MIC_SAMPLING_TYPE_8BIT:
            flags = SPI_MIC_SAMPLING_TYPE_8BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT:
            flags = SPI_MIC_SAMPLING_TYPE_12BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_8BIT:
            flags = SPI_MIC_SAMPLING_TYPE_S8BIT;
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT:
            flags = SPI_MIC_SAMPLING_TYPE_S12BIT;
            break;
        case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
            flags = (SPI_MIC_SAMPLING_TYPE_12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
            flags = (SPI_MIC_SAMPLING_TYPE_S12BIT | SPI_MIC_SAMPLING_TYPE_FILTER_OFF);
            break;
        default:
            return MIC_RESULT_ILLEGAL_PARAMETER;
        }
        // Enable/disable loop
        if (param->loop_enable)
        {
            flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_LOOP_ON);
        }
        else
        {
            flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_LOOP_OFF);
        }
        // Correction flag for expansion is currently fixed to be OFF
        flags = (u8)(flags | SPI_MIC_SAMPLING_TYPE_CORRECT_OFF);
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send auto-sampling start command
    micWork.callback = callback;
    micWork.callbackArg = arg;
    micWork.full = param->full_callback;
    micWork.fullArg = param->full_arg;
    if (MicStartAutoSampling(param->buffer, param->size, param->rate, flags))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSampling

  Description:  Starts auto-sampling with the microphone.

  Arguments:    param:     Pointer to a structure that specifies auto-sampling settings.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSampling(const MICAutoParam *param)
{
    micWork.commonResult = MIC_StartAutoSamplingAsync(param, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSamplingAsync

  Description:  Asynchronously stops microphone auto-sampling

  Arguments:    callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSamplingAsync(MICCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    // Check lock
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send auto-sampling stop command
    micWork.callback = callback;
    micWork.callbackArg = arg;
    if (MicStopAutoSampling())
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSampling

  Description:  Stops auto-sampling with the microphone.
                If a loop was not specified at the start of auto-sampling, sampling will stop automatically when the buffer is full.
                

  Arguments:    None.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSampling(void)
{
    micWork.commonResult = MIC_StopAutoSamplingAsync(MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSamplingAsync

  Description:  Asynchronously adjusts the sampling rate used in microphone auto sampling
                

  Arguments:    rate:       - Specifies the sampling rate.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSamplingAsync(u32 rate, MICCallback callback, void *arg)
{
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    // Confirms parameters
    if (rate < MIC_SAMPLING_RATE_LIMIT)
    {
        return MIC_RESULT_ILLEGAL_PARAMETER;
    }

    // Check lock
    enabled = OS_DisableInterrupts();
    if (micWork.lock != MIC_LOCK_OFF)
    {
        (void)OS_RestoreInterrupts(enabled);
        return MIC_RESULT_BUSY;
    }
    micWork.lock = MIC_LOCK_ON;
    (void)OS_RestoreInterrupts(enabled);

    // Send the auto sampling adjustment command
    micWork.callback = callback;
    micWork.callbackArg = arg;
    if (MicAdjustAutoSampling(rate))
    {
        return MIC_RESULT_SUCCESS;
    }
    micWork.lock = MIC_LOCK_OFF;
    return MIC_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSampling

  Description:  Adjusts the sampling rate of the microphone auto-sampling

  Arguments:    rate:       - Specifies the sampling rate.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSampling(u32 rate)
{
    micWork.commonResult = MIC_AdjustAutoSamplingAsync(rate, MicGetResultCallback, NULL);
    if (micWork.commonResult == MIC_RESULT_SUCCESS)
    {
        MicWaitBusy();
    }
    return micWork.commonResult;
}

/*---------------------------------------------------------------------------*
  Name:         MIC_GetLastSamplingAddress

  Description:  Gets the address where the most recent mic sampling result is stored

  Arguments:    None.

  Returns:      void*:   Returns the storage address of the sampling result.
                        Returns NULL if nothing has been sampled yet.
 *---------------------------------------------------------------------------*/
void   *MIC_GetLastSamplingAddress(void)
{
    return (void *)(OS_GetSystemWork()->mic_last_address);
}

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSamplingAsync

  Description:  Asynchronously starts sampling rate limited microphone auto-sampling.
                Sampling on precise frequencies is possible with a low burden on the CPU because sampling is performed by the hardware, but the sampling rate is limited to the rates supported by the hardware. 
                
                
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
                If this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling start function.
                

  Arguments:    param:     Pointer to a structure that specifies auto-sampling settings.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult
MIC_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StartLimitedSamplingAsync(param, callback, arg) :
            MIC_StartAutoSamplingAsync(param, callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSamplingAsync

  Description:  Asynchronously stops sampling rate limited microphone auto-sampling.
                
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
                f this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling stop function.
                

  Arguments:    callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult
MIC_StopLimitedSamplingAsync(MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StopLimitedSamplingAsync(callback, arg) :
            MIC_StopAutoSamplingAsync(callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSamplingAsync

  Description:  Asynchronously adjusts the sampling rate used in sampling rate limited microphone auto-sampling.
                
                This is an asynchronous function, so the actual processing results are passed when the callback is called. 
                If this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling adjustment function.
                

  Arguments:    rate: Specifies the sampling interval in ARM7 CPU clock units.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult
MIC_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_AdjustLimitedSamplingAsync(rate, callback, arg) :
            MIC_AdjustAutoSamplingAsync(rate, callback, arg));
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSampling

  Description:  Starts sampling rate limited microphone auto-sampling.
                Sampling on precise frequencies is possible with a low burden on the CPU because sampling is performed by the hardware, but the sampling rate is limited to the rates supported by the hardware. 
                
                
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
                If this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling start function.
                

  Arguments:    param:     Pointer to a structure that specifies auto-sampling settings.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult
MIC_StartLimitedSampling(const MICAutoParam* param)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StartLimitedSampling(param) :
            MIC_StartAutoSampling(param));
}

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSampling

  Description:  Stops sampling rate limited microphone auto-sampling.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
                f this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling stop function.
                

  Arguments:    None.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult
MIC_StopLimitedSampling(void)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_StopLimitedSampling() :
            MIC_StopAutoSampling());
}

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSampling

  Description:  Adjusts the sampling rate used in sampling rate limited microphone auto-sampling.
                
                This is a synchronous function, so calling from within an interrupt handler is prohibited.
                If this function is called on NITRO, it is replaced by the preexisting CPU-based auto-sampling adjustment function.
                

  Arguments:    rate: Specifies the sampling interval in ARM7 CPU clock units.

  Returns:      MICResult:    - Returns the result of the device operation process
 *---------------------------------------------------------------------------*/
MICResult
MIC_AdjustLimitedSampling(u32 rate)
{
    return ((OSi_IsCodecTwlMode() == TRUE) ?
            MICEXi_AdjustLimitedSampling(rate) :
            MIC_AdjustAutoSampling(rate));
}
#endif

/*---------------------------------------------------------------------------*
  Name:         MicCommonCallback

  Description:  Common callback function for asynchronous MIC functions

  Arguments:    tag:   PXI tag that shows the message type.
                data:  Message from ARM7.
                err:   PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MicCommonCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    u16     command;
    u16     pxiresult;
    MICResult result;
    MICCallback cb;

    // Verify PXI communication error
    if (err)
    {
        if (micWork.lock != MIC_LOCK_OFF)
        {
            micWork.lock = MIC_LOCK_OFF;
        }
        if (micWork.callback)
        {
            cb = micWork.callback;
            micWork.callback = NULL;
            cb(MIC_RESULT_FATAL_ERROR, micWork.callbackArg);
        }
    }

    // Analyze received data
    command = (u16)((data & SPI_PXI_RESULT_COMMAND_MASK) >> SPI_PXI_RESULT_COMMAND_SHIFT);
    pxiresult = (u16)((data & SPI_PXI_RESULT_DATA_MASK) >> SPI_PXI_RESULT_DATA_SHIFT);

    // Check the result of processing 
    switch (pxiresult)
    {
    case SPI_PXI_RESULT_SUCCESS:
        result = MIC_RESULT_SUCCESS;
        break;
    case SPI_PXI_RESULT_INVALID_COMMAND:
        result = MIC_RESULT_INVALID_COMMAND;
        break;
    case SPI_PXI_RESULT_INVALID_PARAMETER:
        result = MIC_RESULT_ILLEGAL_PARAMETER;
        break;
    case SPI_PXI_RESULT_ILLEGAL_STATUS:
        result = MIC_RESULT_ILLEGAL_STATUS;
        break;
    case SPI_PXI_RESULT_EXCLUSIVE:
        result = MIC_RESULT_BUSY;
        break;
    default:
        result = MIC_RESULT_FATAL_ERROR;
    }

    // Verify processing target command
    if (command == SPI_PXI_COMMAND_MIC_BUFFER_FULL)
    {
        // Callback buffer full notification
        if (micWork.full)
        {
            micWork.full(result, micWork.fullArg);
        }
    }
    else
    {
        if (command == SPI_PXI_COMMAND_MIC_SAMPLING)
        {
            // Store sampling result in buffer
            if (micWork.dst_buf)
            {
                *(u16 *)(micWork.dst_buf) = OS_GetSystemWork()->mic_sampling_data;
            }
        }
        // Open exclusive lock
        if (micWork.lock != MIC_LOCK_OFF)
        {
            micWork.lock = MIC_LOCK_OFF;
        }
        // Callback processing result
        if (micWork.callback)
        {
            cb = micWork.callback;
            micWork.callback = NULL;
            cb(result, micWork.callbackArg);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MicDoSampling

  Description:  Send command to ARM7 for sampling microphone once

  Arguments:    type - Sampling type
                ( 0: 8-bit , 1: 12-bit , 2: signed 8-bit, 3: signed 12-bit )

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static BOOL MicDoSampling(u16 type)
{
    // PXI Packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_SAMPLING << 8) | (u32)type, 0))
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicStartAutoSampling

  Description:  Send start command for microphone auto-sampling to ARM7

  Arguments:    buf: Address of buffer storing sampled data.
                size     - Buffer size. Specified in byte units.
                span     - sampling interval (specify with ARM7 CPU clock)
                        Because of the characteristics of the timer, only numbers of 16 bits x 1, 64, 256, or 1024 can be accurately set. 
                        Bits at the end are truncated.
                flags - Specifies the AD conversion bit width, enabling/disabling of loops, and enabling/disabling of correction

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static BOOL MicStartAutoSampling(void *buf, u32 size, u32 span, u8 flags)
{
    // PXI Packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_AUTO_ON << 8) | (u32)flags, 0))
    {
        return FALSE;
    }

    // PXI Packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (1 << SPI_PXI_INDEX_SHIFT) | ((u32)buf >> 16), 0))
    {
        return FALSE;
    }

    // PXI Packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (2 << SPI_PXI_INDEX_SHIFT) | ((u32)buf & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // PXI Packet [3]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (3 << SPI_PXI_INDEX_SHIFT) | (size >> 16), 0))
    {
        return FALSE;
    }

    // PXI Packet [4]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               (4 << SPI_PXI_INDEX_SHIFT) | (size & 0x0000ffff), 0))
    {
        return FALSE;
    }

    // PXI Packet [5]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (5 << SPI_PXI_INDEX_SHIFT) | (span >> 16), 0))
    {
        return FALSE;
    }

    // PXI Packet [6]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_END_BIT |
                               (6 << SPI_PXI_INDEX_SHIFT) | (span & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicStopAutoSampling

  Description:  Send stop command for microphone auto-sampling to ARM7

  Arguments:    None.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static BOOL MicStopAutoSampling(void)
{
    // PXI Packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               SPI_PXI_END_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) | (SPI_PXI_COMMAND_MIC_AUTO_OFF << 8), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicAdjustAutoSampling

  Description:  Sends microphone auto-sampling change command to ARM7.

  Arguments:    span: Specifies the sampling interval in ARM7 CPU clock units.

  Returns:      BOOL - Returns TRUE when the command was successfully sent via PXI and FALSE on failure.
                       
 *---------------------------------------------------------------------------*/
static BOOL MicAdjustAutoSampling(u32 span)
{
    // PXI Packet [0]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_START_BIT |
                               (0 << SPI_PXI_INDEX_SHIFT) |
                               (SPI_PXI_COMMAND_MIC_AUTO_ADJUST << 8), 0))
    {
        return FALSE;
    }

    // PXI Packet [1]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC, (1 << SPI_PXI_INDEX_SHIFT) | (span >> 16), 0))
    {
        return FALSE;
    }

    // PXI Packet [2]
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_MIC,
                               SPI_PXI_END_BIT |
                               (2 << SPI_PXI_INDEX_SHIFT) | (span & 0x0000ffff), 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MicGetResultCallback

  Description:  Called when asynchronous processing completes. Updates the processing results of internal variables.

  Arguments:    result  -   The processing results from async function.
                arg    - Not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MicGetResultCallback(MICResult result, void *arg)
{
#pragma unused( arg )

    micWork.commonResult = result;
}

#include    <nitro/code32.h>
/*---------------------------------------------------------------------------*
  Name:         MicWaitBusy

  Description:  Wait while asynchronous processing for MIC is locked

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
MicWaitBusy( void )
{
    ldr     r12,    =micWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #MIC_LOCK_ON
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         MICi_GetSysWork

  Description:  Gets the MIC library control structure.

  Arguments:    None.

  Returns:      MICWork*: Returns a pointer to the structure.
 *---------------------------------------------------------------------------*/
MICWork*
MICi_GetSysWork(void)
{
    return &micWork;
}
#endif

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
