/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - snd
  File:     sndex.c

  Copyright 2008-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2011-11-01#$
  $Rev: 11432 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include    <twl/os.h>
#include    <twl/snd/ARM9/sndex.h>
#include    <nitro/pxi.h>
#include    <twl/os/common/codecmode.h>


/*---------------------------------------------------------------------------*
    Macro definitions
 *---------------------------------------------------------------------------*/
#ifdef  SDK_DEBUG
#define     SNDEXi_Warning          OS_TWarning
#else
#define     SNDEXi_Warning(...)     ((void)0)
#endif

/*---------------------------------------------------------------------------*
    Structure definitions
 *---------------------------------------------------------------------------*/

/* Work structure for asynchronous processes */
typedef struct SNDEXWork
{
    SNDEXCallback   callback;
    void*           cbArg;
    void*           dest;
    u8              command;
    u8              padding[3];

} SNDEXWork;

/* Internal status */
typedef enum SNDEXState
{
    SNDEX_STATE_BEFORE_INIT       =   0,
    SNDEX_STATE_INITIALIZING      =   1,
    SNDEX_STATE_INITIALIZED       =   2,
    SNDEX_STATE_LOCKED            =   3,
    SNDEX_STATE_PLAY_SHUTTER      =   4,        // Shutter sound is playing (exclude all SNDEX functions other than PreProcessForShutterSound and PostProcessForShutterSound)
    SNDEX_STATE_POST_PROC_SHUTTER =   5,        // Currently postprocessing the shutter sound playback (exclude all SNDEX functions)
    SNDEX_STATE_MAX

} SNDEXState;

/* Set audio output device */
typedef enum SNDEXDevice
{
    SNDEX_DEVICE_AUTO       =   0,
    SNDEX_DEVICE_SPEAKER    =   1,
    SNDEX_DEVICE_HEADPHONE  =   2,
    SNDEX_DEVICE_BOTH       =   3,
    SNDEX_DEVICE_MAX

} SNDEXDevice;

/*---------------------------------------------------------------------------*
    Variable Definitions
 *---------------------------------------------------------------------------*/
static volatile SNDEXState   sndexState  =   SNDEX_STATE_BEFORE_INIT;
static SNDEXWork    sndexWork;
static SNDEXVolumeSwitchCallbackInfo SNDEXi_VolumeSwitchCallbackInfo = {NULL, NULL, NULL, 0};

PMSleepCallbackInfo sleepCallbackInfo;
PMExitCallbackInfo  exitCallbackInfo;
static volatile BOOL         isIirFilterSetting = FALSE;     // TRUE if already executing SetIirFilterAsync
static volatile BOOL         isLockSpi = FALSE;              // Whether SPI exclusion control is currently being applied inside SNDEX_SetIirFilter[Async]

static volatile BOOL         isPlayShutter = FALSE;          // TRUE if currently processing shutter sound playback
static volatile BOOL         isStoreVolume = FALSE;          // Whether a volume has been saved by SNDEXi_SetIgnoreHWVolume
static          u8           storeVolume   = 0;              // Value of the volume before it is temporarily changed by SNDEXi_SetIgnoreHWVolume

/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/
static void         CommonCallback  (PXIFifoTag tag, u32 data, BOOL err);
static void         SyncCallback    (SNDEXResult result, void* arg);
static BOOL         SendCommand     (u8 command, u8 param);
static BOOL         SendCommandEx   (u8 command, u16 param);
static SNDEXResult  CheckState       (void);
static void         ReplyCallback   (SNDEXResult result);
static void         SetSndexWork    (SNDEXCallback cb, void* cbarg, void* dst, u8 command);

static void         SndexSleepAndExitCallback    (void *arg);

static void         WaitIirFilterSetting   (void);
static void         WaitResetSoundCallback (SNDEXResult result, void* arg);
static void         ResetTempVolume        (void);

SNDEXResult SNDEXi_GetDeviceAsync       (SNDEXDevice* device, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetDevice            (SNDEXDevice* device);
SNDEXResult SNDEXi_SetDeviceAsync       (SNDEXDevice device, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetDevice            (SNDEXDevice device);

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_Init

  Description:  Initializes the extended sound features library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
SNDEXi_Init (void)
{
    OSIntrMode  e   =   OS_DisableInterrupts();

    /* Confirm that initialized */
    if (sndexState != SNDEX_STATE_BEFORE_INIT)
    {
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("%s: Already initialized.\n", __FUNCTION__);
        return;
    }
    sndexState  =   SNDEX_STATE_INITIALIZING;
    (void)OS_RestoreInterrupts(e);

    /* Variable initialization */
    SetSndexWork(NULL, NULL, NULL, 0);

    /* Stand by until the ARM7's SNDEX library is started */
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_SNDEX, PXI_PROC_ARM7))
    {
        SVC_WaitByLoop(10);
    }
    /* Set the PXI callback function */
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_SNDEX, CommonCallback);
    
    /* Set the postprocessing callback function for when executing a hardware reset or shutdown*/
    PM_SetExitCallbackInfo(&exitCallbackInfo, SndexSleepAndExitCallback, NULL);
    PMi_InsertPostExitCallbackEx(&exitCallbackInfo, PM_CALLBACK_PRIORITY_SNDEX);
    
    /* Register the pre-sleep callback */
    PM_SetSleepCallbackInfo(&sleepCallbackInfo, SndexSleepAndExitCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCallbackInfo, PM_CALLBACK_PRIORITY_SNDEX);

    /* Initialization complete */
    sndexState  =   SNDEX_STATE_INITIALIZED;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetMuteAsync

  Description:  Asynchronously gets the mute status of the audio output.
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    mute: Buffer used to get the mute status
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetMuteAsync (SNDEXMute* mute, SNDEXCallback callback, void* arg)
{
    /* No parameter restrictions */
    
    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)mute, SNDEX_PXI_COMMAND_GET_SMIX_MUTE);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetMute

  Description:  Gets the mute status of the audio output.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    mute: Buffer used to get the mute status

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetMute (SNDEXMute* mute)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_GetMuteAsync(mute, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetI2SFrequencyAsync

  Description:  Asynchronously gets the I2S frequency information.
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    freq: Buffer used to get the frequency information
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetI2SFrequencyAsync (SNDEXFrequency* freq, SNDEXCallback callback, void* arg)
{
    /* No parameter restrictions */

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)freq, SNDEX_PXI_COMMAND_GET_SMIX_FREQ);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetI2SFrequency

  Description:  Gets I2S frequency information.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    freq: Buffer used to get the frequency information

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetI2SFrequency(SNDEXFrequency* freq)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_GetI2SFrequencyAsync(freq, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDSPMixRateAsync

  Description:  Gets the audio output mix rate information for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    rate: Buffer used to get the mix rate information
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetDSPMixRateAsync (u8* rate, SNDEXCallback callback, void* arg)
{
    /* No parameter restrictions */

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)rate, SNDEX_PXI_COMMAND_GET_SMIX_DSP);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDSPMixRate

  Description:  Gets the audio output mix rate information for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    rate: Buffer used to get the mix rate information

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDSPMixRate (u8* rate)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_GetDSPMixRateAsync(rate, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeAsync

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 7.
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    volume: Buffer used to get the volume information
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function
                eightlv: Gets the volume as one of eight levels
                keep: Gets the amount of volume reserved for changes in volume. If there is no amount reserved, gets the current value.

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetVolumeAsync (u8* volume, SNDEXCallback callback, void* arg, BOOL eightlv, BOOL keep)
{
    /* No parameter restrictions */

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)volume, SNDEX_PXI_COMMAND_GET_VOLUME);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command,
        (u8)((keep << SNDEX_PXI_PARAMETER_SHIFT_VOL_KEEP) | (eightlv << SNDEX_PXI_PARAMETER_SHIFT_VOL_8LV))))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolume

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 7.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    volume: Buffer used to get the volume information
                eightlv: Gets the volume as one of eight levels
                keep: Gets the amount of volume reserved for changes in volume. If there is no amount reserved, gets the current value.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolume (u8* volume, BOOL eightlv, BOOL keep)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_GetVolumeAsync(volume, SyncCallback, (void*)(&msgQ), eightlv, keep);
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeExAsync

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    volume: Buffer used to get the volume information
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolumeAsync(volume, callback, arg, FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetVolumeEx

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    volume: Buffer used to get the volume information

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetVolumeEx (u8* volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolume(volume, FALSE, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetCurrentVolumeEx[Async]

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                If this is the synchronous function, calling it from inside an interrupt handler is prohibited.
                For asynchronous functions, the buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    volume: Buffer used to get the volume information
               [callback: Callback function that notifies the results when the asynchronous processing is completed]
               [arg: Parameter passed to the callback function]

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------
*/
SNDEXResult
SNDEXi_GetCurrentVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolumeAsync(volume, callback, arg, FALSE, FALSE);
}

SNDEXResult
SNDEXi_GetCurrentVolumeEx (u8* volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_GetVolume(volume, FALSE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDeviceAsync

  Description:  Gets the audio output device setting information.
                The buffer should not be used for other purposes until the asynchronous process completes, as results are written to the results buffer asynchronously from the function call.

  Arguments:    device: Buffer used to get the output device information
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------



*/
SNDEXResult
SNDEXi_GetDeviceAsync (SNDEXDevice* device, SNDEXCallback callback, void* arg)
{
    /* No parameter restrictions */

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)device, SNDEX_PXI_COMMAND_GET_SND_DEVICE);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_GetDevice

  Description:  Gets the audio output device setting information.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    device: Buffer used to get the output device information

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_GetDevice (SNDEXDevice* device)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_GetDeviceAsync(device, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetMuteAsync

  Description:  Changes the mute status of the audio output.

  Arguments:    mute: Mute setting status
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetMuteAsync (SNDEXMute mute, SNDEXCallback callback, void* arg)
{
    /* Parameter restriction check */
    if (mute >= SNDEX_MUTE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (mute: %d)\n", __FUNCTION__, mute);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_MUTE);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, (u8)mute))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetMute

  Description:  Changes the mute status of the audio output.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    mute: Mute setting status

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetMute (SNDEXMute mute)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_SetMuteAsync(mute, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetI2SFrequencyAsync

  Description:  Changes the I2S frequency.

  Arguments:    freq: Frequency
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg)
{
    /* Parameter restriction check */
    if (freq >= SNDEX_FREQUENCY_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (freq: %d)\n", __FUNCTION__, freq);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* Check CODEC operating mode */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_FREQ);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, (u8)freq))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetI2SFrequency

  Description:  Changes the I2S frequency.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    freq: Frequency

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetI2SFrequency (SNDEXFrequency freq)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_SetI2SFrequencyAsync(freq, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDSPMixRateAsync

  Description:  Changes the audio output mix rate for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).

  Arguments:    rate: Mix rate with a numeral from 0 to 8
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetDSPMixRateAsync (u8 rate, SNDEXCallback callback, void* arg)
{
    /* Check CODEC operating mode */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        SNDEXi_Warning("%s: Illegal state\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* Parameter restriction check */
    if (rate > SNDEX_DSP_MIX_RATE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (rate: %u)\n", __FUNCTION__, rate);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SMIX_DSP);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, (u8)rate))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDSPMixRate

  Description:  Changes the audio output mix rate for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    rate: Mix rate with a numeral from 0 to 8

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDSPMixRate (u8 rate)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_SetDSPMixRateAsync(rate, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeAsync

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 7.

  Arguments:    volume: Volume as a numeral from 0 to 7
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function
                eightlv: Sets the volume as one of eight levels

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetVolumeAsync (u8 volume, SNDEXCallback callback, void* arg, BOOL eightlv)
{
    /* Parameter restriction check */
    if (volume > (eightlv ? SNDEX_VOLUME_MAX : SNDEX_VOLUME_MAX_EX))
    {
        SNDEXi_Warning("%s: Invalid parameter (volume: %u)\n", __FUNCTION__, volume);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_VOLUME);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 
        (u8)((eightlv << SNDEX_PXI_PARAMETER_SHIFT_VOL_8LV) | (SNDEX_PXI_PARAMETER_MASK_VOL_VALUE & volume))))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolume

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 7.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    volume: Volume as a numeral from 0 to 7

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolume (u8 volume, BOOL eightlv)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_SetVolumeAsync(volume, SyncCallback, (void*)(&msgQ), eightlv);
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeExAsync

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 31.

  Arguments:    volume: Volume as a numeral from 0 to 31
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetVolumeExAsync (u8 volume, SNDEXCallback callback, void* arg)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_SetVolumeAsync(volume, callback, arg, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeEx

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 31.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    volume: Volume as a numeral from 0 to 31

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetVolumeEx (u8 volume)
{
    if (!OS_IsRunOnTwl())
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    return SNDEXi_SetVolume(volume, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDeviceAsync

  Description:  Changes the audio output device setting.

  Arguments:    device: Output device setting
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetDeviceAsync (SNDEXDevice device, SNDEXCallback callback, void* arg)
{
    /* Parameter restriction check */
    if (device >= SNDEX_DEVICE_MAX)
    {
        SNDEXi_Warning("%s: Invalid parameter (device: %d)\n", __FUNCTION__, device);
        return SNDEX_RESULT_INVALID_PARAM;
    }

    /* Check CODEC operating mode */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_SND_DEVICE);

    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, (u8)device))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetDevice

  Description:  Changes the audio output device setting.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    device: Output device setting

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetDevice (SNDEXDevice device)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_SetDeviceAsync(device, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilterAsync

  Description:  Asynchronously sets the IIR filter (Biquad) parameters.

  !! CAUTION!!: This uses multiple back-to-back transfers, as it is not possible to pass all parameters to ARM7 in one PXI transmission. Just sending all parameters in successive transmissions could run into problems where the previous parameter has not been fully processed when the next arrives, so this waits for processing to complete before making the next PXI transmission.

  Arguments:    target: 
                pParam:

  Returns:      None.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_SetIirFilterAsync(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg)
{
    /* target range check */
    if ( target < 0 || target >= SNDEX_IIR_FILTER_TARGET_AVAILABLE_MAX )
    {
        return SNDEX_RESULT_ILLEGAL_TARGET;
    }

    // Return an error if already processing SetIirFilterAsync
    if (isIirFilterSetting)
    {
        return SNDEX_RESULT_EXCLUSIVE;
    }
    isIirFilterSetting = TRUE;
    
    /* Check CODEC operating mode */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        SNDEXi_Warning("%s: Illegal state\n", __FUNCTION__);
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            isIirFilterSetting = FALSE;
            return result;
        }
    }
    
  /* Target */
    /* Register asynchronous process */
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_TARGET);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, (u16)target))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    // Wait for the callback by idling in an empty loop so that there is time for the CheckState function's Warning suppression
    OS_SpinWait(67 * 1000); //Approximately 1 ms
    
    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }
    
  /* Filter parameter */
    // n0
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N0);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n0))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //Approximately 1 ms

    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }

    // n1
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N1);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n1))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //Approximately 1 ms
    
    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }
    
    // n2
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N2);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->n2))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //Approximately 1 ms
    
    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }
    
    // d1
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D1);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->d1))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //Approximately 1 ms
    
    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }
    
    // d2
    SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D2);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, pParam->d2))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    OS_SpinWait(67 * 1000); //Approximately 1 ms
    
    // Wait until the previous PXI transfer's processing ends
    while ( CheckState() != SNDEX_RESULT_SUCCESS )
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
    }
    
  /* Set the target and the filter parameters */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_SET_IIRFILTER);
    /* Issue command to ARM7 */
    if (FALSE == SendCommandEx(sndexWork.command, 0))
    {
        isIirFilterSetting = FALSE;
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetIirFilter

  Description:  Sets the IIR filter (Biquad) parameters.

  Arguments:    target: 
                pParam:

  Returns:      None.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_SetIirFilter(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* target range check */
    if ( target < 0 || target >= SNDEX_IIR_FILTER_TARGET_AVAILABLE_MAX )
    {
        return SNDEX_RESULT_ILLEGAL_TARGET;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);
    
    result = SNDEXi_SetIirFilterAsync(target, pParam, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    
    // Assume that the SPI_Lock was ended by SetIirFilter, and clear the flag
    isLockSpi = FALSE;
    
    isIirFilterSetting = FALSE;
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphoneAsync

  Description:  Gets whether headphones are connected.

  Arguments:    hp: Buffer used to get headphone connection status
                callback: Callback function that notifies of the results when the asynchronous processing is completed.
                arg: Parameter passed to the callback function

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------


*/
SNDEXResult
SNDEXi_IsConnectedHeadphoneAsync(SNDEXHeadphone *hp, SNDEXCallback callback, void* arg)
{
    /* Confirm status */
    {
        SNDEXResult result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }

    /* Register asynchronous process */
    SetSndexWork(callback, arg, (void*)hp, SNDEX_PXI_COMMAND_HP_CONNECT);
    
    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphone

  Description:  Gets whether headphones are connected.B

  Arguments:    hp: Buffer used to get headphone connection status

  Returns:      SNDEXResult: Returns processing results. When returning SNDEX_RESULT_SUCCESS, the function registered to the callback is notified of the actual processing results after the asynchronous process is complete.
 *---------------------------------------------------------------------------

*/
SNDEXResult
SNDEXi_IsConnectedHeadphone(SNDEXHeadphone *hp)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];

    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }

    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);

    /* Execute the asynchronous function and idle the callback */
    result  =   SNDEXi_IsConnectedHeadphoneAsync(hp, SyncCallback, (void*)(&msgQ));
    if (result == SNDEX_RESULT_SUCCESS)
    {
        (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetVolumeSwitchCallback

  Description:  Sets the callback function that is called when the volume button is pressed.

  Arguments:    callback: Callback function called when the volume button is pressed
                arg: Parameter passed to the callback function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SNDEXi_SetVolumeSwitchCallback(SNDEXCallback callback, void* arg)
{
    SNDEXi_VolumeSwitchCallbackInfo.callback = callback;
    SNDEXi_VolumeSwitchCallbackInfo.cbArg = arg;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_SetIgnoreHWVolume

  Description:  If you need to play a sound (such as the clock's alarm tone) at a specified volume regardless of the value set for the system volume at the time, you must change the volume without user input.
                This function saves the volume before changing it.

  Arguments:    volume: Value to change the volume to

  Returns:      SNDEXResult: Returns processing results. This is the result of an SNDEX function called internally.
 *---------------------------------------------------------------------------

*/
SNDEXResult SNDEXi_SetIgnoreHWVolume(u8 volume, BOOL eightlv)
{
    SNDEXResult result;
    
    // If the ResetIgnoreHWVolume function was not run after the previous SetIgnoreHWVolume function was called, the volume value is not saved.
    // 
    if (!isStoreVolume)
    {
        result = SNDEXi_GetVolumeEx(&storeVolume);
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }
    
    // Change the volume to the specified value
    result = eightlv ? SNDEX_SetVolume(volume) : SNDEXi_SetVolumeEx(volume);
    if (result != SNDEX_RESULT_SUCCESS)
    {
        return result;
    }
    
    isStoreVolume = TRUE;
    return result;      // SNDEX_RESULT_SUCCESS
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_ResetIgnoreHWVolume

  Description:  Restores the volume to its value before changed by SNDEX_SetIgnoreHWVolume.

  Arguments:    None.
  
  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
SNDEXResult SNDEXi_ResetIgnoreHWVolume(void)
{
    SNDEXResult result;
    if ((result = SNDEXi_SetVolumeEx(storeVolume)) != SNDEX_RESULT_SUCCESS)
    {
        return result;
    }
    
    isStoreVolume = FALSE;
    return result;          // SNDEX_RESULT_SUCCESS
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_PreProcessForShutterSound

  Description:  Performs all the preprocessing necessary for shutter sound playback.
                This is a synchronous function, so calling from inside an interrupt handler is prohibited.

  Arguments:    None.

  Returns:      Returns SNDEX_RESULT_SUCCESS if successful.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_PreProcessForShutterSound  (void)
{
    SNDEXResult     result;
    OSMessageQueue  msgQ;
    OSMessage       msg[1];
    
    /* Check CODEC operating mode */
    if (OSi_IsCodecTwlMode() == FALSE)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* Calling this from an exception handler is prohibited */
    if (OS_GetCurrentThread() == NULL)
    {
        SNDEXi_Warning("%s: Syncronous API could not process in exception handler.\n", __FUNCTION__);
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* Skip CheckState if the SNDEXi_PreProcessForShutterSound function has already been called. */
    if (sndexState != SNDEX_STATE_PLAY_SHUTTER)
    {
        /* Confirm status */
        result  =   CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return result;
        }
    }
    else if (sndexState == SNDEX_STATE_POST_PROC_SHUTTER)
    {
        return SNDEX_RESULT_EXCLUSIVE;
    }
    
    /* Initialize the message queue used for synchronization */
    OS_InitMessageQueue(&msgQ, msg, 1);
    
    
    /* Register process */
    SetSndexWork(SyncCallback, (void*)(&msgQ), NULL, SNDEX_PXI_COMMAND_PRE_PROC_SHUTTER);
    
    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }

    /* Wait for reply */
    (void)OS_ReceiveMessage(&msgQ, (OSMessage*)(&result), OS_MESSAGE_BLOCK);

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SNDEXi_PostProcessCallbackForShutterSound

  Description:  Executes postprocessing for shutter sound playback.
                This is executed asynchronously because it is executed inside an interrupt.

  Arguments:    callback: Callback function called when the postprocessing completes
                arg: Parameter passed to the callback function

  Returns:      Returns SNDEX_RESULT_SUCCESS if successful.
 *---------------------------------------------------------------------------*/
SNDEXResult
SNDEXi_PostProcessForShutterSound (SNDEXCallback callback, void* arg)
{
    /* Check CODEC operating mode */
    // When this function is called, it is presumed to be after execution of SNDEXi_PreProcessCallbackForShutterSound
    if (OSi_IsCodecTwlMode() == FALSE || sndexState != SNDEX_STATE_PLAY_SHUTTER)
    {
        return SNDEX_RESULT_ILLEGAL_STATE;
    }
    
    /* Transition the state to postprocessing (maximum priority) */
    sndexState = SNDEX_STATE_POST_PROC_SHUTTER;
    
    /* Register asynchronous process */
    SetSndexWork(callback, arg, NULL, SNDEX_PXI_COMMAND_POST_PROC_SHUTTER);
    
    /* Issue command to ARM7 */
    if (FALSE == SendCommand(sndexWork.command, 0))
    {
        return SNDEX_RESULT_PXI_SEND_ERROR;
    }
    
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CommonCallback

  Description:  PXI FIFO reception callback function for processes related to the extended sound features.

  Arguments:    tag: PXI message type
                data: Message content from ARM7
                err: PXI transfer error flag

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
CommonCallback (PXIFifoTag tag, u32 data, BOOL err)
{
    u8      command =   (u8)((data & SNDEX_PXI_COMMAND_MASK) >> SNDEX_PXI_COMMAND_SHIFT);
    u8      result  =   (u8)((data & SNDEX_PXI_RESULT_MASK) >> SNDEX_PXI_RESULT_SHIFT);
    u8      param   =   (u8)((data & SNDEX_PXI_PARAMETER_MASK) >> SNDEX_PXI_PARAMETER_SHIFT);
    
    /* If the command received from ARM7 is a notification that the system volume switch was pressed, try locking */
    if (command == SNDEX_PXI_COMMAND_PRESS_VOLSWITCH)
    {
        SNDEXResult result = CheckState();
        if (result != SNDEX_RESULT_SUCCESS)
        {
            return;
        }
        // Set the sndexWork structure
        SetSndexWork(NULL, NULL, NULL, SNDEX_PXI_COMMAND_PRESS_VOLSWITCH);
    }
    
    /* Confirm status */
    if ((tag != PXI_FIFO_TAG_SNDEX) || (err == TRUE)
            || (sndexState != SNDEX_STATE_LOCKED && sndexState != SNDEX_STATE_PLAY_SHUTTER && sndexState != SNDEX_STATE_POST_PROC_SHUTTER)
            || (sndexWork.command != command))
    {
        /* Logically, execution will never be able to pass through here, but output a warning just in case */
        OS_TWarning("SNDEX: Library state is inconsistent.\n");
        ReplyCallback(SNDEX_RESULT_FATAL_ERROR);
        return;
    }

    /* Parse the process results */
    switch (result)
    {
    case SNDEX_PXI_RESULT_SUCCESS:
        /* Perform completion processing in accordance with the command */
        switch (command)
        {
        case SNDEX_PXI_COMMAND_GET_SMIX_MUTE:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXMute*)sndexWork.dest)   = (SNDEXMute)param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SMIX_FREQ:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXFrequency*)sndexWork.dest)  =   (SNDEXFrequency)param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SMIX_DSP:
        case SNDEX_PXI_COMMAND_GET_VOLUME:
            if (sndexWork.dest != NULL)
            {
                *((u8*)sndexWork.dest)  =   param;
            }
            break;
        case SNDEX_PXI_COMMAND_GET_SND_DEVICE:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXDevice*)sndexWork.dest) =   (SNDEXDevice)param;
            }
            break;
        case SNDEX_PXI_COMMAND_PRESS_VOLSWITCH:
            /* Notification from ARM7 that the volume button was pressed */
            if (SNDEXi_VolumeSwitchCallbackInfo.callback != NULL)
            {
                (SNDEXi_VolumeSwitchCallbackInfo.callback)((SNDEXResult)result, SNDEXi_VolumeSwitchCallbackInfo.cbArg);
            }
            break;
        case SNDEX_PXI_COMMAND_HP_CONNECT:
            if (sndexWork.dest != NULL)
            {
                *((SNDEXHeadphone*)sndexWork.dest) =   (SNDEXHeadphone)param;
            }
            break;
        }
        ReplyCallback(SNDEX_RESULT_SUCCESS);
        break;
    case SNDEX_PXI_RESULT_INVALID_PARAM:
        ReplyCallback(SNDEX_RESULT_INVALID_PARAM);
        break;
    case SNDEX_PXI_RESULT_EXCLUSIVE:
        ReplyCallback(SNDEX_RESULT_EXCLUSIVE);
        break;
    case SNDEX_PXI_RESULT_ILLEGAL_STATE:
        ReplyCallback(SNDEX_RESULT_ILLEGAL_STATE);
        break;
    case SNDEX_PXI_RESULT_DEVICE_ERROR:
        if (command == SNDEX_PXI_COMMAND_GET_VOLUME)
        {
            if (sndexWork.dest != NULL)
            {
                *((u8*)sndexWork.dest)  =   SNDEX_VOLUME_MIN;
            }
        }
        ReplyCallback(SNDEX_RESULT_DEVICE_ERROR);
        break;
    case SNDEX_PXI_RESULT_INVALID_COMMAND:
    default:
        ReplyCallback(SNDEX_RESULT_FATAL_ERROR);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SyncCallback

  Description:  Shared callback function for synchronous function control.

  Arguments:    result: Result of asynchronous processing
                arg: Message being blocked by the synchronous function while it waits for reception.
                            Pointer to queue control structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SyncCallback (SNDEXResult result, void* arg)
{
    SDK_NULL_ASSERT(arg);

    if (FALSE == OS_SendMessage((OSMessageQueue*)arg, (OSMessage)result, OS_MESSAGE_NOBLOCK))
    {
        /* Logically, execution will never be able to pass through here, but output a warning just in case */
        OS_TWarning("SNDEX: Temporary message queue is full.\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         SendCommand

  Description:  Issues an extended sound feature operation command to ARM7 via PXI.

  Arguments:    command: Command type
                param: Parameter associated with the command

  Returns:      BOOL: Returns TRUE if PXI transmission was successful.
 *---------------------------------------------------------------------------*/
static BOOL
SendCommand (u8 command, u8 param)
{
    OSIntrMode  e   =   OS_DisableInterrupts();
    u32     packet  =   (u32)(((command << SNDEX_PXI_COMMAND_SHIFT) & SNDEX_PXI_COMMAND_MASK)
                        | ((param << SNDEX_PXI_PARAMETER_SHIFT) & SNDEX_PXI_PARAMETER_MASK));

    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_SNDEX, packet, 0))
    {
        /* Cancel mutex if send failed */
        sndexState  =   SNDEX_STATE_INITIALIZED;
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Failed to send PXI command.\n");
        return FALSE;
    }
    
    (void)OS_RestoreInterrupts(e);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SendCommand

  Description:  Issues an extended sound feature operation command to ARM7 via PXI.

  Arguments:    command: Command type
                param: Parameter associated with the command (16 bits because of the IirFilter parameter)

  Returns:      BOOL: Returns TRUE if PXI transmission was successful.
 *---------------------------------------------------------------------------*/
static BOOL
SendCommandEx (u8 command, u16 param)
{
    OSIntrMode  e   =   OS_DisableInterrupts();
    u32     packet  =   (u32)(((command << SNDEX_PXI_COMMAND_SHIFT) & SNDEX_PXI_COMMAND_MASK)
                        | ((param << SNDEX_PXI_PARAMETER_SHIFT) & SNDEX_PXI_PARAMETER_MASK_IIR));

    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_SNDEX, packet, 0))
    {
        /* Cancel mutex if send failed */
        sndexState  =  SNDEX_STATE_INITIALIZED;
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Failed to send PXI command.\n");
        return FALSE;
    }
    if( command == SNDEX_PXI_COMMAND_SET_IIRFILTER )
    {
        isLockSpi = TRUE;
    }
    
    (void)OS_RestoreInterrupts(e);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CheckState

  Description:  Checks whether the internal state is one where an asynchronous process can start, and if so, locks the SNDEX library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------
*/
static SNDEXResult
CheckState (void)
{
    OSIntrMode  e   =   OS_DisableInterrupts();

    /* Check internal state */
    switch (sndexState)
    {
    case SNDEX_STATE_BEFORE_INIT:
    case SNDEX_STATE_INITIALIZING:
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Library is not initialized yet.\n");
        return SNDEX_RESULT_BEFORE_INIT;
    case SNDEX_STATE_LOCKED:
    case SNDEX_STATE_PLAY_SHUTTER:
    case SNDEX_STATE_POST_PROC_SHUTTER:
        (void)OS_RestoreInterrupts(e);
        SNDEXi_Warning("SNDEX: Another request is in progress.\n");
        return SNDEX_RESULT_EXCLUSIVE;
    }

    /* Transition to state where exclusion control is in effect */
    sndexState  =   SNDEX_STATE_LOCKED;
    (void)OS_RestoreInterrupts(e);
    return SNDEX_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReplyCallback

  Description:  Sends notification of the completion of asynchronous processing. Updates the internal state and also unlocks the exclusion state.

  Arguments:    result: Asynchronous processing result to notify to the callback function

  Returns:      None.
 *---------------------------------------------------------------------------
*/
static void
ReplyCallback (SNDEXResult result)
{
    OSIntrMode      e           =   OS_DisableInterrupts();
    SNDEXCallback   callback    =   sndexWork.callback;
    void*           cbArg       =   sndexWork.cbArg;
    u8              command     =   sndexWork.command;
    
    // Assume that the SPI_Lock was ended by SetIirFilterAsync, and lower the flag
    if (sndexWork.command == SNDEX_PXI_COMMAND_SET_IIRFILTER)
    {
        isLockSpi = FALSE;
        isIirFilterSetting = FALSE;
    }
    
    /* Reset state */
    SetSndexWork(NULL, NULL, NULL, 0);
    // If currently preprocessing the shutter sound playback, do not unlock until postprocessing begins
    if (command == SNDEX_PXI_COMMAND_PRE_PROC_SHUTTER)
    {
        // Anything other than the SNDEXi_PostProcessForShutterSound function, which performs shutter sound playback postprocessing, is considered locked and is excluded.
        // 
        sndexState          =   SNDEX_STATE_PLAY_SHUTTER;
    }
    else
    {
        sndexState          =   SNDEX_STATE_INITIALIZED;
    }
    (void)OS_RestoreInterrupts(e);

    /* Call the registered callback function */
    if (callback != NULL)
    {
        callback(result, cbArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetSndexWork

  Description:  Sets parameters for sndexWork.

  Arguments:    cb: Pointer to the callback function
                cbarg: Argument to pass to 'cb'
                dst: Pointer to a variable to store the command's processing results
                command: Command to send to ARM7

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SetSndexWork    (SNDEXCallback cb, void* cbarg, void* dst, u8 command)
{
    sndexWork.callback = cb;
    sndexWork.cbArg = cbarg;
    sndexWork.dest = dst;
    sndexWork.command = command;
}

/*---------------------------------------------------------------------------*
  Name:         SndexSleepAndExitCallback

  Description:  This callback function is invoked at hardware reset, shutdown, or transition to Sleep Mode.

  Arguments:    arg

  Returns:      None.
 *---------------------------------------------------------------------------
*/
static void
SndexSleepAndExitCallback (void *arg)
{
#pragma unused( arg )
    // Always wait for IIR filter processing to finish first, because while it is running, other SNDEX functions may be exclusively locked in the SNDEX library.
    // 
    WaitIirFilterSetting();
    
    ResetTempVolume();
}

/*---------------------------------------------------------------------------*
  Name:         WaitIirFilterSetting

  Description:  Waits for SNDEX_SetIirFilter[Async] to end.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
WaitIirFilterSetting (void)
{
    // Continue waiting until the SPI_Lock in SNDEX_SetIirFilter[Async] ends
    while (isLockSpi)
    {
        OS_SpinWait(67 * 1000); //Approximately 1 ms
        PXIi_HandlerRecvFifoNotEmpty();
    }
}

static void 
WaitResetSoundCallback(SNDEXResult result, void* arg)
{
    static u32 i = 0;    // Retry up to 5 times
#pragma unused(arg)
    if (result != SNDEX_RESULT_SUCCESS && i < 5)
    {
        (void)SNDEXi_SetVolumeExAsync(storeVolume, WaitResetSoundCallback, NULL);
        i++;
        return;
    }
    isStoreVolume = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ResetTempVolume

  Description:  Reverts to the volume that was temporarily saved by SNDEXi_SetIgnoreHWVolume.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
ResetTempVolume (void)
{
    if (isStoreVolume)
    {
        static i = 0;    // Retry up to 5 times
        // Revert to the volume that was temporarily saved by SNDEXi_SetIgnoreHWVolume
        // Repeat until the asynchronous function runs successfully
        // The callback will handle retries having to do with success or failure of the volume change
        while( SNDEX_RESULT_SUCCESS != SNDEXi_SetVolumeExAsync(storeVolume, WaitResetSoundCallback, NULL) && i < 5)
        {
            i++;
        }
        while (isStoreVolume)
        {
            OS_SpinWait(67 * 1000); //Approximately 1 ms
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}

