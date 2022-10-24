/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_util.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-26#$
  $Rev: 9412 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_UTIL_H_
#define TWL_DSP_UTIL_H_


#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Constants */

#define DSP_SLOT_B_COMPONENT_G711           1
#define DSP_SLOT_C_COMPONENT_G711           2

#define DSP_SLOT_B_COMPONENT_JPEGENCODER    3
#define DSP_SLOT_C_COMPONENT_JPEGENCODER    4

#define DSP_SLOT_B_COMPONENT_JPEGDECODER    2
#define DSP_SLOT_C_COMPONENT_JPEGDECODER    4

#define DSP_SLOT_B_COMPONENT_GRAPHICS       1
#define DSP_SLOT_C_COMPONENT_GRAPHICS       4

#define DSP_SLOT_B_COMPONENT_AACDECODER     2
#define DSP_SLOT_C_COMPONENT_AACDECODER     4

/*---------------------------------------------------------------------------*/
/* Functions */

void DSPi_PlaySoundCore(const void *src, u32 len, BOOL stereo);
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len);
void DSPi_StopSoundCore(void);
BOOL DSPi_IsSoundPlayingCore(void);
BOOL DSPi_IsShutterSoundPlayingCore(void);
void DSPi_StartSamplingCore(void *buffer, u32 length);
void DSPi_StopSamplingCore(void);
void DSPi_SyncSamplingBufferCore(void);
const u8* DSPi_GetLastSamplingAddressCore(void);


/*---------------------------------------------------------------------------*
  Name:         DSP_PlaySound

  Description:  Plays sound directly from the DSP audio output.
  
  Arguments:    src: The original sampling data.
                         This must be PCM 16-bit data at 32,768 kHz, aligned at 4-byte boundaries.
                len: Number of bytes of sampling data.
                stereo: TRUE for stereo and FALSE for monaural.

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_PlaySound(const void *src, u32 len, BOOL stereo)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_PlaySoundCore(src, len, stereo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlayShutterSound

  Description:  Plays the shutter sound from the DSP audio output.
  
  Arguments:    src: Shutter sound data, in WAVE format, associated with the SDK.
                len: Number of bytes in the shutter sound data.

  
  Returns:      Returns TRUE if the SDNEX function succeeded.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_PlayShutterSound(const void *src, u32 len)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_PlayShutterSoundCore(src, len);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsSoundPlaying

  Description:  Determines whether DSP audio output is currently being played back.
  
  Arguments:    None.
  
  Returns:      TRUE if DSP audio output is currently being played back.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_IsSoundPlaying(void)
{
    BOOL    retval = FALSE;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_IsSoundPlayingCore();
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsShutterSoundPlaying

  Description:  Determines whether the shutter sound is currently being played back from the DSP audio output.
  
  Arguments:    None.
  
  Returns:      TRUE if the shutter sound is currently being played back from the DSP audio output.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_IsShutterSoundPlaying(void)
{
    BOOL    retval = FALSE;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_IsShutterSoundPlayingCore();
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSound

  Description:  Plays sound directly from the DSP audio output.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StopSound(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StopSoundCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartSampling

  Description:  Starts DSP microphone sampling.

  Arguments:    buffer: The ring buffer to use for sampling.
                         This must be aligned to boundaries at integer multiples of 16 bits.
                length: The ring buffer size.
                         This must be aligned to boundaries at integer multiples of 16 bits.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StartSampling(void *buffer, u32 length)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StartSamplingCore(buffer, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSampling

  Description:  Stops DSP microphone sampling.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_StopSampling(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_StopSamplingCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncSamplingBuffer

  Description:  Loads only the updated parts of the internal DSP ring buffer into the ARM9.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_SyncSamplingBuffer(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_SyncSamplingBufferCore();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetLastSamplingAddress

  Description:  Gets the most recent sample position in the local ring buffer that was loaded to the ARM9.
                

  Arguments:    None.

  Returns:      The most recent sample position.
 *---------------------------------------------------------------------------*/
SDK_INLINE const u8* DSP_GetLastSamplingAddress(void)
{
    const u8   *retval = NULL;
    if (OS_IsRunOnTwl())
    {
        retval = DSPi_GetLastSamplingAddressCore();
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * The following are internal functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPostStartProcess

  Description:  This is a hook that occurs immediately after a DSP process image is loaded.
                This is required for DSP component developers to start the debugger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_HookPostStartProcess(void);


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TWL_DSP_UTIL_H_ */
