/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - include
  File:     api.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SPI_ARM9_MIC_H_
#define NITRO_SPI_ARM9_MIC_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/spi/common/type.h>
#include    <nitro/pxi.h>


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
// Processing result definition
typedef enum MICResult
{
    MIC_RESULT_SUCCESS = 0,            // Success
    MIC_RESULT_BUSY,                   // Exclusion control in effect
    MIC_RESULT_ILLEGAL_PARAMETER,      // Illegal parameter
    MIC_RESULT_SEND_ERROR,             // Failed transmission by PXI
    MIC_RESULT_INVALID_COMMAND,        // Unknown command
    MIC_RESULT_ILLEGAL_STATUS,         // Status does not permit execution
    MIC_RESULT_FATAL_ERROR,            // Errors other than those above
    MIC_RESULT_MAX
}
MICResult;

// Sampling type definitions
typedef enum MICSamplingType
{
    MIC_SAMPLING_TYPE_8BIT = 0,        //  8-bit sampling
    MIC_SAMPLING_TYPE_12BIT,           // 12-bit sampling
    MIC_SAMPLING_TYPE_SIGNED_8BIT,     // Signed 8-bit sampling
    MIC_SAMPLING_TYPE_SIGNED_12BIT,    // Signed 12-bit sampling
    MIC_SAMPLING_TYPE_12BIT_FILTER_OFF,
    MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF,
    MIC_SAMPLING_TYPE_MAX
}
MICSamplingType;

// Typical sampling periods are defined in ARM7 clock ticks.
typedef enum MICSamplingRate
{
    MIC_SAMPLING_RATE_8K = (HW_CPU_CLOCK_ARM7 / 8000),  // Approx. 8.0 kHz
    MIC_SAMPLING_RATE_11K = (HW_CPU_CLOCK_ARM7 / 11025),        // Approx. 11.025 kHz
    MIC_SAMPLING_RATE_16K = (HW_CPU_CLOCK_ARM7 / 16000),        // Approx. 16.0 kHz
    MIC_SAMPLING_RATE_22K = (HW_CPU_CLOCK_ARM7 / 22050),        // Approx. 22.05 kHz
    MIC_SAMPLING_RATE_32K = (HW_CPU_CLOCK_ARM7 / 32000),        // Approx. 32.0 kHz
    MIC_SAMPLING_RATE_LIMIT = 1024
}
MICSamplingRate;

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Callback function type definitions
typedef void (*MICCallback) (MICResult result, void *arg);

// Auto-sampling setting definitions
typedef struct MICAutoParam
{
    MICSamplingType type;              // Sampling type
    void   *buffer;                    // Pointer to result storage buffer
    u32     size;                      // Buffer size
    u32     rate;                      // Sampling period (ARM7 clock count)
    BOOL    loop_enable;               // Enable/disable the loop when buffer is full
    MICCallback full_callback;         // Callback when buffer is full
    void   *full_arg;                  // Argument to specify for the above callbacks

}
MICAutoParam;


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  Initializes microphone library

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MIC_Init(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSamplingAsync

  Description:  Performs a single asynchronous sample of the microphone.

  Arguments:    type:      - Specifies the sampling type.
                buf:       - Specifies the buffer that stores the sampling data.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSamplingAsync(MICSamplingType type, void *buf, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSamplingAsync

  Description:  Asynchronously starts microphone auto-sampling

  Arguments:    param:     - Pointer to a structure that specifies auto-sampling settings
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSamplingAsync(const MICAutoParam *param, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSamplingAsync

  Description:  Asynchronously stops microphone auto-sampling

  Arguments:    callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSamplingAsync(MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSamplingAsync

  Description:  Asynchronously adjusts the sampling rate used in microphone auto-sampling
                

  Arguments:    rate:       - Specifies the sampling rate.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSamplingAsync(u32 rate, MICCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_GetLastSamplingAddress

  Description:  Gets the address where the most recent mic sampling result is stored

  Arguments:    None.

  Returns:      void*:   Returns the storage address of the sampling result.
                        Returns NULL if nothing has been sampled yet.
 *---------------------------------------------------------------------------*/
void   *MIC_GetLastSamplingAddress(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_DoSampling

  Description:  Samples the microphone once.

  Arguments:    type:      - Specifies the sampling type.
                buf:       - Specifies the buffer that stores the sampling data.

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult MIC_DoSampling(MICSamplingType type, void *buf);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartAutoSampling

  Description:  Starts auto-sampling with the microphone.

  Arguments:    param:     - Pointer to a structure that specifies auto-sampling settings

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult MIC_StartAutoSampling(const MICAutoParam *param);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopAutoSampling

  Description:  Stops auto-sampling with the microphone.
                If a loop was not specified at the start of auto-sampling, sampling will stop automatically when the buffer is full.
                

  Arguments:    None.

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult MIC_StopAutoSampling(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustAutoSampling

  Description:  Adjusts the microphone auto-sampling rate.

  Arguments:    rate:       - Specifies the sampling rate.

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult MIC_AdjustAutoSampling(u32 rate);

#ifdef  SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSamplingAsync

  Description:  Asynchronously starts rate-limited microphone auto-sampling.
                Although sampling at accurate frequencies with a low CPU load is possible because it is done in hardware, this is limited to sampling rates supported by the hardware.
                This is an asynchronous function, so the actual processing result will be passed when the callback is invoked.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to start auto-sampling.
                
                
                

  Arguments:    param:     - Pointer to a structure that specifies auto-sampling settings.
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult   MIC_StartLimitedSamplingAsync(const MICAutoParam* param, MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSamplingAsync

  Description:  Asynchronously stops rate-limited microphone auto-sampling.
                This is an asynchronous function, so the actual processing result will be passed when the callback is invoked.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to stop auto-sampling.
                
                

  Arguments:    callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult   MIC_StopLimitedSamplingAsync(MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSamplingAsync

  Description:  Asynchronously adjusts the rate for rate-limited microphone auto-sampling.
                This is an asynchronous function, so the actual processing result will be passed when the callback is invoked.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to adjust the auto-sampling rate.
                
                

  Arguments:    rate     - Specifies the sampling interval in ARM7 CPU clock cycles
                callback     - Specifies the function to be called when the asynchronous process is completed.
                arg       - Specifies the argument used when calling the callback function.

  Returns:      MICResult:    - Returns the result of starting the asynchronous device operation.
 *---------------------------------------------------------------------------*/
MICResult   MIC_AdjustLimitedSamplingAsync(u32 rate, MICCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         MIC_StartLimitedSampling

  Description:  Starts rate-limited microphone auto-sampling.
                Although sampling at accurate frequencies with a low CPU load is possible because it is done in hardware, this is limited to sampling rates supported by the hardware.
                This is a synchronous function, so calls to it from interrupt handlers are prohibited.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to start auto-sampling.
                
                
                

  Arguments:    param:     - Pointer to a structure that specifies auto-sampling settings.

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult   MIC_StartLimitedSampling(const MICAutoParam* param);

/*---------------------------------------------------------------------------*
  Name:         MIC_StopLimitedSampling

  Description:  Stops rate-limited microphone auto-sampling.
                This is a synchronous function, so calls to it from interrupt handlers are prohibited.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to stop auto-sampling.
                

  Arguments:    None.

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult   MIC_StopLimitedSampling(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_AdjustLimitedSampling

  Description:  Adjusts the rate for rate-limited microphone auto-sampling.
                This is a synchronous function, so calls to it from interrupt handlers are prohibited.
                When called on a NITRO system, this will be replaced by an existing function that uses the CPU to adjust the auto-sampling rate.
                
                

  Arguments:    rate     - Specifies the sampling interval in ARM7 CPU clock cycles

  Returns:      MICResult:    - Returns the result of the device operation process.
 *---------------------------------------------------------------------------*/
MICResult   MIC_AdjustLimitedSampling(u32 rate);

#endif

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_RTC_ARM9_API_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
