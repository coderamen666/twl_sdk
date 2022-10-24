/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     camera_api.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-20#$
  $Rev: 9373 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_CAMERA_API_H_
#define TWL_CAMERA_CAMERA_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// Processing Result Definitions
typedef enum CAMERAResult
{
    CAMERA_RESULT_SUCCESS = 0,
    CAMERA_RESULT_SUCCESS_TRUE = 0,
    CAMERA_RESULT_SUCCESS_FALSE,
    CAMERA_RESULT_BUSY,
    CAMERA_RESULT_ILLEGAL_PARAMETER,
    CAMERA_RESULT_SEND_ERROR,
    CAMERA_RESULT_INVALID_COMMAND,
    CAMERA_RESULT_ILLEGAL_STATUS,
    CAMERA_RESULT_FATAL_ERROR,
    CAMERA_RESULT_MAX
}
CAMERAResult;

// Callbacks
typedef void (*CAMERACallback)(CAMERAResult result, void *arg);
typedef void (*CAMERAIntrCallback)(CAMERAResult result);

// Core APIs
CAMERAResult CAMERA_InitCore(void);
void CAMERA_EndCore(void);
CAMERAResult CAMERA_StartCore(CAMERASelect camera);
CAMERAResult CAMERA_StopCore(void);
CAMERAResult CAMERA_I2CInitAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CInitCore(CAMERASelect camera);
CAMERAResult CAMERA_I2CActivateAsyncCore(CAMERASelect camera, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CActivateCore(CAMERASelect camera);
CAMERAResult CAMERA_I2CContextSwitchAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CContextSwitchCore(CAMERASelect camera, CAMERAContext context);
CAMERAResult CAMERA_I2CSizeExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERASize size, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CSizeExCore(CAMERASelect camera, CAMERAContext context, CAMERASize size);
CAMERAResult CAMERA_I2CFrameRateAsyncCore(CAMERASelect camera, CAMERAFrameRate rate, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CFrameRateCore(CAMERASelect camera, CAMERAFrameRate rate);
CAMERAResult CAMERA_I2CEffectExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CEffectExCore(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect);
CAMERAResult CAMERA_I2CFlipExAsyncCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CFlipExCore(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip);
CAMERAResult CAMERA_I2CPhotoModeAsyncCore(CAMERASelect camera, CAMERAPhotoMode mode, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CPhotoModeCore(CAMERASelect camera, CAMERAPhotoMode mode);
CAMERAResult CAMERA_I2CWhiteBalanceAsyncCore(CAMERASelect camera, CAMERAWhiteBalance wb, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CWhiteBalanceCore(CAMERASelect camera, CAMERAWhiteBalance wb);
CAMERAResult CAMERA_I2CExposureAsyncCore(CAMERASelect camera, int exposure, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CExposureCore(CAMERASelect camera, int exposure);
CAMERAResult CAMERA_I2CSharpnessAsyncCore(CAMERASelect camera, int sharpness, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CSharpnessCore(CAMERASelect camera, int sharpness);
CAMERAResult CAMERAi_I2CTestPatternAsyncCore(CAMERASelect camera, CAMERATestPattern pattern, CAMERACallback callback, void *arg);
CAMERAResult CAMERAi_I2CTestPatternCore(CAMERASelect camera, CAMERATestPattern pattern);
CAMERAResult CAMERA_I2CAutoExposureAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CAutoExposureCore(CAMERASelect camera, BOOL on);
CAMERAResult CAMERA_I2CAutoWhiteBalanceAsyncCore(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_I2CAutoWhiteBalanceCore(CAMERASelect camera, BOOL on);
CAMERAResult CAMERA_SetLEDAsyncCore(BOOL isBlink, CAMERACallback callback, void *arg);
CAMERAResult CAMERA_SetLEDCore(BOOL isBlink);
CAMERAResult CAMERA_SwitchOffLEDAsyncCore(CAMERACallback callback, void *arg);
CAMERAResult CAMERA_SwitchOffLEDCore(void);

void CAMERA_SetVsyncCallbackCore(CAMERAIntrCallback callback);
void CAMERA_SetBufferErrorCallbackCore(CAMERAIntrCallback callback);
void CAMERA_SetRebootCallbackCore(CAMERAIntrCallback callback);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  Initializes the CAMERA library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Init(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERAResult result;
        result = CAMERA_InitCore();
        if (result == CAMERA_RESULT_SUCCESS)
        {
            return CAMERA_I2CInitCore(CAMERA_SELECT_BOTH);
        }
        else
        {
            return result;
        }
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_End

  Description:  Shuts down the CAMERA library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_End(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_EndCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  This high-level function starts capturing. This can also be used for switching.
                Synchronous version only.

  Arguments:    camera: One of the CAMERASelect values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Start(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_StartCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  This high-level function stops capturing.
                Synchronous version only.

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_Stop(void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_StopCore();
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  Initializes camera registers via I2C.
                async version.

  Arguments:    camera: One of the CAMERASelect values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CInitAsync(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CInitAsyncCore(camera, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  Initializes camera registers via I2C.
                sync version.

  Arguments:    camera: One of the CAMERASelect values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CInit(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CInitCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  Activates the specified camera (go to standby mode if NONE is specified).
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values (BOTH is not valid)
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CActivateAsync(CAMERASelect camera, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CActivateAsyncCore(camera, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  Activates the specified camera (go to standby mode if NONE is specified).
                sync version.

  Arguments:    camera: One of the CAMERASelect values (BOTH is not valid)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CActivate(CAMERASelect camera)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CActivateCore(camera);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CContextSwitchAsync

  Description:  Resizes the camera output image.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CContextSwitchAsync(CAMERASelect camera, CAMERAContext context, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CContextSwitchAsyncCore(camera, context, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CContextSwitch

  Description:  Resizes the camera output image.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CContextSwitch(CAMERASelect camera, CAMERAContext context)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CContextSwitchCore(camera, context);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeExAsync

  Description:  Sets the camera frame size.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                size: One of the CAMERASize values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeExAsync(CAMERASelect camera, CAMERAContext context, CAMERASize size, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExAsyncCore(camera, context, size, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeAsync

  Description:  Sets the camera frame size for all contexts.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                size: One of the CAMERASize values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeAsync(CAMERASelect camera, CAMERASize size, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExAsyncCore(camera, CAMERA_CONTEXT_BOTH, size, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSizeEx

  Description:  Sets the camera frame size.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                size: One of the CAMERASize values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSizeEx(CAMERASelect camera, CAMERAContext context, CAMERASize size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExCore(camera, context, size);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSize

  Description:  Sets the camera frame size for all contexts.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                size: One of the CAMERASize values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSize(CAMERASelect camera, CAMERASize size)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSizeExCore(camera, CAMERA_CONTEXT_BOTH, size);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  Sets the camera frame rate.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                rate: One of the CAMERAFrameRate values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFrameRateAsync(CAMERASelect camera, CAMERAFrameRate rate, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFrameRateAsyncCore(camera, rate, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  Sets the camera frame rate.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                rate: One of the CAMERAFrameRate values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFrameRate(CAMERASelect camera, CAMERAFrameRate rate)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFrameRateCore(camera, rate);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectExAsync

  Description:  Sets camera effects.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                effect: One of the CAMERAEffect values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectExAsync(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExAsyncCore(camera, context, effect, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectAsync

  Description:  Sets the camera effect for all contexts.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                effect: One of the CAMERAEffect values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectAsync(CAMERASelect camera, CAMERAEffect effect, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExAsyncCore(camera, CAMERA_CONTEXT_BOTH, effect, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  Sets camera effects.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                effect: One of the CAMERAEffect values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffectEx(CAMERASelect camera, CAMERAContext context, CAMERAEffect effect)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExCore(camera, context, effect);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  Sets the camera effect for all contexts.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                effect: One of the CAMERAEffect values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CEffect(CAMERASelect camera, CAMERAEffect effect)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CEffectExCore(camera, CAMERA_CONTEXT_BOTH, effect);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipExAsync

  Description:  Sets the camera's flip/mirror.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                flip: One of the CAMERAFlip values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipExAsync(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExAsyncCore(camera, context, flip, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipAsync

  Description:  Sets the camera flip/mirror for all contexts.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                flip: One of the CAMERAFlip values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipAsync(CAMERASelect camera, CAMERAFlip flip, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExAsyncCore(camera, CAMERA_CONTEXT_BOTH, flip, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipEx

  Description:  Sets the camera's flip/mirror.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                context: One of the CAMERAContext values (A or B)
                flip: One of the CAMERAFlip values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlipEx(CAMERASelect camera, CAMERAContext context, CAMERAFlip flip)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExCore(camera, context, flip);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  Sets the camera flip/mirror for all contexts.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                flip: One of the CAMERAFlip values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CFlip(CAMERASelect camera, CAMERAFlip flip)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CFlipExCore(camera, CAMERA_CONTEXT_BOTH, flip);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoModeAsync

  Description:  Sets the camera's photo mode.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                mode: One of the CAMERAPhotoMode values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CPhotoModeAsync(CAMERASelect camera, CAMERAPhotoMode mode, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CPhotoModeAsyncCore(camera, mode, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPhotoMode

  Description:  Sets the camera's photo mode.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                mode: One of the CAMERAPhotoMode values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CPhotoMode(CAMERASelect camera, CAMERAPhotoMode mode)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CPhotoModeCore(camera, mode);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalanceAsync

  Description:  Sets the camera's white balance.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                wb: One of the CAMERAWhiteBalance values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CWhiteBalanceAsync(CAMERASelect camera, CAMERAWhiteBalance wb, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CWhiteBalanceAsyncCore(camera, wb, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CWhiteBalance

  Description:  Sets the camera's white balance.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                wb: One of the CAMERAWhiteBalance values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CWhiteBalance(CAMERASelect camera, CAMERAWhiteBalance wb)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CWhiteBalanceCore(camera, wb);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposureAsync

  Description:  Sets the camera's exposure.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                exposure: -5 to +5
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CExposureAsync(CAMERASelect camera, int exposure, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CExposureAsyncCore(camera, exposure, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CExposure

  Description:  Sets the camera's exposure.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                exposure: -5 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CExposure(CAMERASelect camera, int exposure)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CExposureCore(camera, exposure);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpnessAsync

  Description:  Sets the camera's sharpness.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                sharpness: -3 to +5
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSharpnessAsync(CAMERASelect camera, int sharpness, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSharpnessAsyncCore(camera, sharpness, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSharpness

  Description:  Sets the camera's sharpness.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                sharpness: -3 to +5
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CSharpness(CAMERASelect camera, int sharpness)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CSharpnessCore(camera, sharpness);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPatternAsync

  Description:  Sets the camera's test pattern.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                pattern: One of the CAMERATestPattern values
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERAi_I2CTestPatternAsync(CAMERASelect camera, CAMERATestPattern pattern, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERAi_I2CTestPatternAsyncCore(camera, pattern, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CTestPattern

  Description:  Sets the camera's test pattern.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                pattern: One of the CAMERATestPattern values

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERAi_I2CTestPattern(CAMERASelect camera, CAMERATestPattern pattern)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERAi_I2CTestPatternCore(camera, pattern);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposureAsync

  Description:  Enables and disables camera auto-exposure.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                on: TRUE if AE will be enabled
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoExposureAsync(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoExposureAsyncCore(camera, on, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoExposure

  Description:  Enables and disables camera auto-exposure.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                on: TRUE if AE will be enabled
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoExposure(CAMERASelect camera, BOOL on)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoExposureCore(camera, on);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalanceAsync

  Description:  Enables and disables camera auto-white balance.
                Asynchronous version.

  Arguments:    camera: One of the CAMERASelect values
                on: TRUE if AWB will be enabled
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoWhiteBalanceAsync(CAMERASelect camera, BOOL on, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoWhiteBalanceAsyncCore(camera, on, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CAutoWhiteBalance

  Description:  Enables and disables camera auto-white balance.
                sync version.

  Arguments:    camera: One of the CAMERASelect values
                on: TRUE if AWB will be enabled
  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_I2CAutoWhiteBalance(CAMERASelect camera, BOOL on)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_I2CAutoWhiteBalanceCore(camera, on);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetLEDAsync

  Description:  Configures whether the camera indicator LED (outside) will blink.
                This only works when the outer camera is active.
                By default, it will not blink when I2CActivate(OUT/BOTH) is called.
                Asynchronous version.

  Arguments:    isBlink: TRUE to cause it to blink and FALSE to leave it lit
                callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SetLEDAsync(BOOL isBlink, CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SetLEDAsyncCore(isBlink, callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetLED

  Description:  Configures whether the camera indicator LED (outside) will blink.
                This only works when the outer camera is active.
                By default, it will not blink when I2CActivate(OUT/BOTH) is called.
                sync version.

  Arguments:    isBlink: TRUE to cause it to blink and FALSE to leave it lit

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SetLED(BOOL isBlink)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SetLEDCore(isBlink);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SwitchOffLEDAsync

  Description:  Turns off the camera indicator LED once.
                This results in the same behavior as calling CAMERA_SetLED(TRUE) and CAMERA_SetLED(FALSE) in succession.
                

  Arguments:    callback: Specifies the function to be called when the asynchronous process has completed.
                arg: Specifies the arguments to the callback function when it is invoked.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SwitchOffLEDAsync(CAMERACallback callback, void *arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SwitchOffLEDAsyncCore(callback, arg);
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SwitchOffLED

  Description:  Turns off the camera indicator LED once.
                This results in the same behavior as calling CAMERA_SetLED(TRUE) and CAMERA_SetLED(FALSE) in succession.
                

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
SDK_INLINE CAMERAResult CAMERA_SwitchOffLED()
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return CAMERA_SwitchOffLEDCore();
    }
    return CAMERA_RESULT_FATAL_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsynCallback

  Description:  Registers a callback function to invoke when a camera VSync interrupt occurs.

  Arguments:    callback: The callback function to register

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetVsyncCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetVsyncCallbackCore(callback);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorCallback

  Description:  Registers a callback function to invoke when a camera buffer error interrupt occurs.

  Arguments:    callback: The callback function to register

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetBufferErrorCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetBufferErrorCallbackCore(callback);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetRebootCallback

  Description:  Registers a callback function to invoke when the process of recovering from a camera malfunction is complete.

  Arguments:    callback: The callback function to register

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CAMERA_SetRebootCallback(CAMERAIntrCallback callback)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        CAMERA_SetRebootCallbackCore(callback);
    }
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CAMERA_API_H_ */
