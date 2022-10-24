/*---------------------------------------------------------------------------*
  Project:  TwlSDK - snd - include
  File:     sndex.h

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2011-11-01#$
  $Rev: 11432 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifndef TWL_SND_ARM9_SNDEX_H_
#define TWL_SND_ARM9_SNDEX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include <twl/snd/common/sndex_common.h>

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
/* Processing result */
typedef enum SNDEXResult
{
    SNDEX_RESULT_SUCCESS        =   0,
    SNDEX_RESULT_BEFORE_INIT    =   1,
    SNDEX_RESULT_INVALID_PARAM  =   2,
    SNDEX_RESULT_EXCLUSIVE      =   3,
    SNDEX_RESULT_ILLEGAL_STATE  =   4,
    SNDEX_RESULT_PXI_SEND_ERROR =   5,
    SNDEX_RESULT_DEVICE_ERROR   =   6,
    SNDEX_RESULT_FATAL_ERROR    =   7,
    SNDEX_RESULT_ILLEGAL_TARGET =   8,
    SNDEX_RESULT_MAX

} SNDEXResult;

/* Mute settings */
typedef enum SNDEXMute
{
    SNDEX_MUTE_OFF  =   0,
    SNDEX_MUTE_ON   =   1,
    SNDEX_MUTE_MAX

} SNDEXMute;

/* I2S frequency settings */
typedef enum SNDEXFrequency
{
    SNDEX_FREQUENCY_32730   =   0,
    SNDEX_FREQUENCY_47610   =   1,
    SNDEX_FREQUENCY_MAX

} SNDEXFrequency;

/* IIR filter parameter structure */
typedef struct _SNDEXIirFilterParam
{
    u16 n0;
    u16 n1;
    u16 n2;
    u16 d1;
    u16 d2;
} SNDEXIirFilterParam;

/* Enumerated type for the IIR filter target */
typedef enum _SNDEXIirFilterTarget
{
    SNDEX_IIR_FILTER_ADC_1 = 0,
    SNDEX_IIR_FILTER_ADC_2,
    SNDEX_IIR_FILTER_ADC_3,
    SNDEX_IIR_FILTER_ADC_4,
    SNDEX_IIR_FILTER_ADC_5,
    SNDEX_IIR_FILTER_TARGET_AVAILABLE_MAX,
    SNDEX_IIR_FILTER_RESERVED_1 = SNDEX_IIR_FILTER_TARGET_AVAILABLE_MAX,
    SNDEX_IIR_FILTER_RESERVED_2,
    SNDEX_IIR_FILTER_RESERVED_3,
    SNDEX_IIR_FILTER_RESERVED_4,
    SNDEX_IIR_FILTER_RESERVED_5,
    SNDEX_IIR_FILTER_RESERVED_6,
    SNDEX_IIR_FILTER_RESERVED_7,
    SNDEX_IIR_FILTER_RESERVED_8,
    SNDEX_IIR_FILTER_RESERVED_9,
    SNDEX_IIR_FILTER_RESERVED_10,
    SNDEX_IIR_FILTER_RESERVED_11,
    SNDEX_IIR_FILTER_RESERVED_12,
    SNDEX_IIR_FILTER_RESERVED_13,
    SNDEX_IIR_FILTER_RESERVED_14,
    SNDEX_IIR_FILTER_RESERVED_15,
    SNDEX_IIR_FILTER_TARGET_MAX
} SNDEXIirFilterTarget;

/* Whether there are headphones connected */
typedef enum SNDEXHeadphone
{
    SNDEX_HEADPHONE_UNCONNECT   =   0,
    SNDEX_HEADPHONE_CONNECT     =   1,
    SNDEX_HEADPHONE_MAX
} SNDEXHeadphone;

/* DSP mix rate settings */
#define SNDEX_DSP_MIX_RATE_MIN  0
#define SNDEX_DSP_MIX_RATE_MAX  8

/* Volume settings */
#define SNDEX_VOLUME_MIN        0
#define SNDEX_VOLUME_MAX        7
#define SNDEX_VOLUME_MAX_EX     31

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
/* Callback function type definitions */
typedef void    (*SNDEXCallback)    (SNDEXResult result, void* arg);

#define SNDEXVolumeSwitchCallbackInfo SNDEXWork

/*---------------------------------------------------------------------------*
    Definitions of Private Functions
 *---------------------------------------------------------------------------*/
void    SNDEXi_Init (void);

SNDEXResult SNDEXi_GetMuteAsync            (SNDEXMute* mute, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetMute                 (SNDEXMute* mute);
SNDEXResult SNDEXi_GetI2SFrequencyAsync    (SNDEXFrequency* freq, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetI2SFrequency         (SNDEXFrequency* freq);
SNDEXResult SNDEXi_GetDSPMixRateAsync      (u8* rate, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetDSPMixRate           (u8* rate);
SNDEXResult SNDEXi_GetVolumeAsync          (u8* volume, SNDEXCallback callback, void* arg, BOOL eightlv, BOOL keep);
SNDEXResult SNDEXi_GetVolume               (u8* volume, BOOL eightlv, BOOL keep);
SNDEXResult SNDEXi_GetVolumeExAsync        (u8* volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetVolumeEx             (u8* volume);
SNDEXResult SNDEXi_GetCurrentVolumeExAsync (u8* volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_GetCurrentVolumeEx      (u8* volume);

SNDEXResult SNDEXi_SetMuteAsync         (SNDEXMute mute, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetMute              (SNDEXMute mute);
SNDEXResult SNDEXi_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetI2SFrequency      (SNDEXFrequency freq);
SNDEXResult SNDEXi_SetDSPMixRateAsync   (u8 rate, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetDSPMixRate        (u8 rate);
SNDEXResult SNDEXi_SetVolumeAsync       (u8 volume, SNDEXCallback callback, void* arg, BOOL eightlv);
SNDEXResult SNDEXi_SetVolume            (u8 volume, BOOL eightlv);
SNDEXResult SNDEXi_SetVolumeExAsync     (u8 volume, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetVolumeEx          (u8 volume);

SNDEXResult SNDEXi_SetIirFilterAsync    (SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_SetIirFilter         (SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam);

SNDEXResult SNDEXi_IsConnectedHeadphoneAsync (SNDEXHeadphone *hp, SNDEXCallback callback, void* arg);
SNDEXResult SNDEXi_IsConnectedHeadphone      (SNDEXHeadphone *hp);

void        SNDEXi_SetVolumeSwitchCallback   (SNDEXCallback callback, void* arg);

SNDEXResult SNDEXi_SetIgnoreHWVolume       (u8 volume, BOOL eightlv);
SNDEXResult SNDEXi_ResetIgnoreHWVolume     (void);

SNDEXResult SNDEXi_PreProcessForShutterSound  (void);
SNDEXResult SNDEXi_PostProcessForShutterSound (SNDEXCallback callback, void* arg);

/*---------------------------------------------------------------------------*
  Name:         SNDEX_Init

  Description:  Initializes the extended sound features library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void
SNDEX_Init (void)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEXi_Init();
    }
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetMuteAsync

  Description:  Asynchronously gets the mute status of the audio output.
                Because the writing of results to the results buffer is asynchronous compared to the function call, the buffer should not be used for other purposes until the asynchronous process completes.
                

  Arguments:    mute: Specifies the buffer used to get the mute status.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetMuteAsync (SNDEXMute* mute, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetMuteAsync(mute, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetMute

  Description:  Gets the mute status of the audio output.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    mute: Specifies the buffer used to get the mute status.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetMute (SNDEXMute* mute)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetMute(mute) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetI2SFrequencyAsync

  Description:  Asynchronously gets the I2S frequency information.
                Because the writing of results to the results buffer is asynchronous compared to the function call, the buffer should not be used for other purposes until the asynchronous process completes.
                

  Arguments:    freq: Specifies the buffer used to get the frequency information.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetI2SFrequencyAsync (SNDEXFrequency* freq, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetI2SFrequencyAsync(freq, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetI2SFrequency

  Description:  Gets I2S frequency information.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    freq: Specifies the buffer used to get the frequency information.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetI2SFrequency (SNDEXFrequency* freq)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetI2SFrequency(freq) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetDSPMixRateAsync

  Description:  Gets the audio output mix rate information for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                Because the writing of results to the results buffer is asynchronous compared to the function call, the buffer should not be used for other purposes until the asynchronous process completes.
                

  Arguments:    rate: Specifies the buffer used to get the mix rate information.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetDSPMixRateAsync (u8* rate, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetDSPMixRateAsync(rate, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetDSPMixRate

  Description:  Gets the audio output mix rate information for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    rate: Specifies the buffer used to get the mix rate information.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetDSPMixRate (u8* rate)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetDSPMixRate(rate) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetVolumeAsync

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                Because the writing of results to the results buffer is asynchronous compared to the function call, the buffer should not be used for other purposes until the asynchronous process completes.
                

  Arguments:    volume: Specifies the buffer used to get the volume information.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetVolumeAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolumeAsync(volume, callback, arg, TRUE, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetCurrentVolumeAsync

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                Because the writing of results to the results buffer is asynchronous compared to the function call, the buffer should not be used for other purposes until the asynchronous process completes.
                

  Arguments:    volume: Specifies the buffer used to get the volume information.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                However, the volume available with this function is the value that has actually been set, not the retained value that is updated by the SNDEX_SetVolume[Async] functions.
                                
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetCurrentVolumeAsync (u8* volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolumeAsync(volume, callback, arg, TRUE, FALSE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetVolume

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    volume: Specifies the buffer used to get the volume information.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetVolume (u8* volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolume(volume, TRUE, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_GetCurrentVolume

  Description:  Gets the audio output volume information. Minimum volume: 0, maximum: 31.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    volume: Specifies the buffer used to get the volume information.

  Returns:      SNDEXResult: Returns processing results.
                                However, the volume available with this function is the value that has actually been set, not the retained value that is updated by the SNDEX_SetVolume[Async] functions.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_GetCurrentVolume (u8* volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_GetVolume(volume, TRUE, FALSE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetMuteAsync

  Description:  Changes the mute status of the audio output.

  Arguments:    mute: Specifies mute setting status.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetMuteAsync (SNDEXMute mute, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetMuteAsync(mute, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetMute

  Description:  Changes the mute status of the audio output.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    mute: Specifies mute setting status.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetMute (SNDEXMute mute)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetMute(mute) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetI2SFrequencyAsync

  Description:  Changes the I2S frequency.

  Arguments:    freq: Specifies the frequency.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetI2SFrequencyAsync (SNDEXFrequency freq, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetI2SFrequencyAsync(freq, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetI2SFrequency

  Description:  Changes the I2S frequency.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    freq: Specifies the frequency.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetI2SFrequency (SNDEXFrequency freq)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetI2SFrequency(freq) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetDSPMixRateAsync

  Description:  Changes the audio output mix rate for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).

  Arguments:    rate: Specifies the mix rate with a numeral from 0 to 8.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetDSPMixRateAsync (u8 rate, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetDSPMixRateAsync(rate, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetDSPMixRate

  Description:  Changes the audio output mix rate for the CPU and DSP.
                Minimum mix rate: 0 (DSP 100%), maximum: 8 (CPU 100%).
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    rate: Specifies the mix rate with a numeral from 0 to 8.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetDSPMixRate (u8 rate)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetDSPMixRate(rate) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolumeAsync

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 31.

  Arguments:    volume: Specifies the volume as a numeral from 0 to 31.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetVolumeAsync (u8 volume, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetVolumeAsync(volume, callback, arg, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolume

  Description:  Changes the audio output volume. Minimum volume: 0, maximum: 31.
                This is a synchronous function, so calling from within an interrupt handler is prohibited.

  Arguments:    volume: Specifies the volume as a numeral from 0 to 31.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetVolume (u8 volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetVolume(volume, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilterAsync

  Description:  Asynchronously sets the IIR filter (Biquad) parameters.

  Arguments:    target: An SNDEXIirFilterTarget enumerated type indicating where to apply IIR filters
                pParam: A SNDEXIirFilterParam structure with IIR filter coefficients
                callback: The callback function that will be invoked after the IIR filters are configured
                arg: Arguments to pass to the callback function

  Returns:      SNDEX_RESULT_SUCCESS on normal completion.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIirFilterAsync(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIirFilterAsync(target, pParam, callback, arg) : SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIirFilter

  Description:  Sets the IIR filter (Biquad) parameters.

  Arguments:    target: An SNDEXIirFilterTarget enumerated type indicating where to apply IIR filters
                pParam: A SNDEXIirFilterParam structure with IIR filter coefficients

  Returns:      SNDEX_RESULT_SUCCESS on normal completion.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIirFilter(SNDEXIirFilterTarget target, const SNDEXIirFilterParam* pParam)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIirFilter(target, pParam) : SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphoneAsync

  Description:  Gets whether headphones are connected.

  Arguments:    hp: Specifies the buffer used to get headphone connection status.
                callback: Specifies the callback function that notifies of the results when the asynchronous process is completed.
                                
                arg: Specifies parameter passed to the callback function.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_IsConnectedHeadphoneAsync (SNDEXHeadphone *hp, SNDEXCallback callback, void* arg)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_IsConnectedHeadphoneAsync(hp, callback, arg) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_IsConnectedHeadphone

  Description:  Gets whether headphones are connected.

  Arguments:    hp: Specifies the buffer used to get headphone connection status.

  Returns:      SNDEXResult: Returns processing results.
                                When returning SNDEX_RESULT_SUCCESS, the actual processing results are notified to the function registered to the callback after the asynchronous process is complete.
                                
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_IsConnectedHeadphone (SNDEXHeadphone *hp)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_IsConnectedHeadphone(hp) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetVolumeSwitchCallback

  Description:  Sets the callback function that is called when the volume button is pressed.

  Arguments:    callback: Specifies the callback function called when the volume button is pressed.
                arg: Specifies parameter passed to the callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void
SNDEX_SetVolumeSwitchCallback (SNDEXCallback callback, void* arg)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEXi_SetVolumeSwitchCallback(callback, arg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_SetIgnoreHWVolume

  Description:  If you need to play a sound (such as the clock's alarm tone) at a specified volume regardless of the value set for the system volume at the time, you must change the volume without user input.
                This function saves the volume before changing it.
                A termination callback function is registered to restore the original volume when the system is reset or shut down at a time that is unintended by the application.
                By calling SNDEX_ResetIgnoreHWVolume, the original volume will be restored and the registered termination callback will be removed.
                
                
                
                

  Arguments:    volume: The value to change the volume to.

  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_SetIgnoreHWVolume (u8 volume)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_SetIgnoreHWVolume(volume, TRUE) :
            SNDEX_RESULT_ILLEGAL_STATE);
}

/*---------------------------------------------------------------------------*
  Name:         SNDEX_ResetIgnoreHWVolume

  Description:  Restores the volume to its value before changed by SNDEX_SetIgnoreHWVolume.
                This removes the registered termination callback at the same time.

  Arguments:    None.
  
  Returns:      SNDEXResult: Returns processing results.
 *---------------------------------------------------------------------------*/
static inline SNDEXResult
SNDEX_ResetIgnoreHWVolume (void)
{
    return (SNDEXResult)((OS_IsRunOnTwl() == TRUE) ?
            SNDEXi_ResetIgnoreHWVolume() :
            SNDEX_RESULT_ILLEGAL_STATE);
}
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   // extern "C"
#endif
#endif  // TWL_SND_ARM9_SNDEX_H_
