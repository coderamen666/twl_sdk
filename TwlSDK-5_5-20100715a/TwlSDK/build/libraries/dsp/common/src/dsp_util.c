/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_util.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-04-16#$
  $Rev: 11320 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>
#include <twl/dsp/common/audio.h>

#include "dsp_process.h"

/*---------------------------------------------------------------------------*/
/* Constants */

// Sound playback priority
#define DSP_SOUND_PRIORITY_SHUTTER          0
#define DSP_SOUND_PRIORITY_NORMAL           16
#define DSP_SOUND_PRIORITY_NONE             32

// For WAVE file parsing
#define MAKE_FOURCC(cc1, cc2, cc3, cc4) (u32)((cc1) | (cc2 << 8) | (cc3 << 16) | (cc4 << 24))
#define MAKE_TAG_DATA(ca) (u32)((*(ca)) | (*(ca+1) << 8) | (*(ca+2) << 16) | (*(ca+3) << 24))
#define FOURCC_RIFF MAKE_FOURCC('R', 'I', 'F', 'F')
#define FOURCC_WAVE MAKE_FOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt  MAKE_FOURCC('f', 'm', 't', ' ')
#define FOURCC_data MAKE_FOURCC('d', 'a', 't', 'a')

// Wait (in ms) inserted before playback of shutter sound
#define DSP_WAIT_SHUTTER                    60

/*---------------------------------------------------------------------------*/
/* Variables */

// DSP sound playback flag
static BOOL DSPiSoundPlaying = FALSE;
static OSAlarm DSPiSoundAlarm;

// Current DSP sound playback priority
static int DSPiSoundPriority = DSP_SOUND_PRIORITY_NONE;

// DSP sampling control information
typedef struct DSPAudioCaptureInfo
{
    DSPAddr bufferAddress;
    DSPWord bufferLength;
    DSPWord currentPosition;
}
DSPAudioCaptureInfo;
static DSPAudioCaptureInfo  DSPiAudioCapture;
extern DSPAddr              DSPiAudioCaptureAddress;
static DSPAddr              DSPiReadPosition = 0;

// Local buffer on the ARM9 side
static u32  DSPiLocalRingLength = 0;
static u8  *DSPiLocalRingBuffer = NULL;
static int  DSPiLocalRingOffset = 0;
static DSPAddr DSPiAudioCaptureAddress = 0;

static BOOL DSPiPlayingShutter = FALSE;

// Function that returns the sound setting when shutter sound playback is completed
static void DSPiShutterPostProcessCallback( SNDEXResult result, void* arg )
{
#pragma unused(arg)
    if(result == SNDEX_RESULT_EXCLUSIVE)
    {
        // Function that returns the shutter sound setting always succeeds
        OS_TPanic("SNDEXi_PostProcessForShutterSound SNDEX_RESULT_EXCLUSIVE ... DSP_PlayShutterSound\n");
    }
    if(result != SNDEX_RESULT_SUCCESS)
    {
        // Function that returns the shutter sound setting always succeeds
        OS_TPanic("SNDEXi_PostProcessForShutterSound Error ... DSP_PlayShutterSound\n");
    }
    DSPiPlayingShutter = FALSE;
}

/*---------------------------------------------------------------------------*/
/* Functions */

// Function generated after DSP plays a sound and ring buffer waits for empty playback
static void sound_handler(void* arg)
{
#pragma unused(arg)
    DSPiSoundPlaying = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PipeCallbackForSound

  Description:  Sound playback completed callback.

  Arguments:    userdata: Optional user-defined argument
                port: Port number
                peer: Transmission direction

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PipeCallbackForSound(void *userdata, int port, int peer)
{
    (void)userdata;
    if (peer == DSP_PIPE_INPUT)
    {
        // Receive command response from DSP
        DSPAudioDriverResponse  response;
        DSPPipe                 pipe[1];
        (void)DSP_LoadPipe(pipe, port, peer);
        if (DSP_GetPipeReadableSize(pipe) >= sizeof(response))
        {
            DSP_ReadPipe(pipe, &response, sizeof(response));
            response.ctrl = DSP_32BIT_TO_DSP(response.ctrl);
            response.result = DSP_32BIT_TO_DSP(response.result);
            // Audio output command
            if ((response.ctrl & DSP_AUDIO_DRIVER_TARGET_MASK) == DSP_AUDIO_DRIVER_TARGET_OUTPUT)
            {
                // Stop command
                if ((response.ctrl & DSP_AUDIO_DRIVER_CONTROL_MASK) == DSP_AUDIO_DRIVER_CONTROL_STOP)
                {
                    //DSPiSoundPlaying = FALSE;
                    OS_CreateAlarm(&DSPiSoundAlarm);
                    OS_SetAlarm(&DSPiSoundAlarm, OS_MilliSecondsToTicks(30), sound_handler, NULL);
                    if(DSPiSoundPriority == DSP_SOUND_PRIORITY_SHUTTER)
                    {
                          // Function that returns the shutter sound setting always succeeds
                          (void)SNDEXi_PostProcessForShutterSound(DSPiShutterPostProcessCallback, 0);
                    }
                    DSPiSoundPriority = DSP_SOUND_PRIORITY_NONE;
                }
            }
            // Audio input command
            if ((response.ctrl & DSP_AUDIO_DRIVER_TARGET_MASK) == DSP_AUDIO_DRIVER_TARGET_INPUT)
            {
                // Start command
                if ((response.ctrl & DSP_AUDIO_DRIVER_CONTROL_MASK) == DSP_AUDIO_DRIVER_CONTROL_START)
                {
                    DSPiAudioCaptureAddress = (DSPAddr)response.result;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PlaySoundEx

  Description:  Plays sound directly from DSP sound output.
  
  Arguments:    src: Sampling data that is the source data
                           It is necessary to use a 4-byte boundary alignment with PCM16bit32768kHz
                len: Sampling data byte size
                ctrl: Combination with DSP_AUDIO_DRIVER_MODE_*
                priority: Playback priority
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PlaySoundEx(const void *src, u32 len, u32 ctrl, int priority)
{
    // Ignore if the component has not started anything up
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context)
    {
        // If priority is lower than the sound that is currently in playback, do not playback
        if (DSPiSoundPriority < priority)
        {
            OS_TWarning("still now playing high-priority sound.\n");
        }
        else
        {
            DSPiSoundPriority = priority;
            ctrl |= DSP_AUDIO_DRIVER_TARGET_OUTPUT;
            ctrl |= DSP_AUDIO_DRIVER_CONTROL_START;
            // Change from byte size to half-word size
            len >>= 1;
            // Set the callback for completion notification
            DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
            DSPiSoundPlaying = TRUE;
            {
                DSPAudioDriverCommand   command;
                command.ctrl = DSP_32BIT_TO_DSP(ctrl);
                command.buf = DSP_32BIT_TO_DSP(src);
                command.len = DSP_32BIT_TO_DSP(len);
                command.opt = DSP_32BIT_TO_DSP(0);
                DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlaySound

  Description:  Plays sound directly from DSP sound output.
  
  Arguments:    src: Sampling data that is the source data
                         It is necessary to use a 4-byte boundary alignment with PCM16bit32768kHz
                len: Sampling data byte size
                stereo: If stereo, TRUE. If monaural, FALSE

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_PlaySoundCore(const void *src, u32 len, BOOL stereo)
{
    u32     ctrl = (stereo != FALSE) ? DSP_AUDIO_DRIVER_MODE_STEREO : DSP_AUDIO_DRIVER_MODE_MONAURAL;
    DSPi_PlaySoundEx(src, len, ctrl, DSP_SOUND_PRIORITY_NORMAL);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_PlayShutterSound

  Description:  Plays sound directly from DSP sound output.
  
  Arguments:    src: Sampling data that is the source data
                         It is necessary to use a 4-byte boundary alignment with PCM16bit32768kHz
                len: Sampling data byte size

  
  Returns:      If SNDEX function is successful, returns TRUE
 *---------------------------------------------------------------------------*/
#if 0 // Do not apply a thorough WAVE check up to here
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len)
{
    u8* wave_data = (u8*)src;
    u32 cur = 0;
    u32 tag;
    u32 wave_len;
    u32 raw_len;
    BOOL    fFmt = FALSE, fData = FALSE;
    
    static SNDEXFrequency freq;
    u32 sampling;
    u32 chunkSize;

    // Check whether the given data is a WAVE file
    if(len < cur+12)
        return FALSE;
    tag = MAKE_TAG_DATA(wave_data+cur);
    if(tag != FOURCC_RIFF)
        return FALSE;
    cur+=4;

    wave_len = MAKE_TAG_DATA(wave_data+cur);
    cur+=4;

    tag = MAKE_TAG_DATA(wave_data+cur);
    if(tag != FOURCC_WAVE)
        return FALSE;
    cur+=4;

    while (wave_len > 0)
    {
        if(len < cur+8)
            return FALSE;
        tag = MAKE_TAG_DATA(wave_data+cur);
        cur+=4;
        chunkSize = MAKE_TAG_DATA(wave_data+cur);
        cur+=4;
        
        if(len < cur+chunkSize)
            return FALSE;
        
        switch (tag)
        {
        case FOURCC_fmt:
            // Check the sampling rate from the format information
            // If not playing back the shutter sound, get the frequency
            if(!DSPi_IsShutterSoundPlayingCore())
            {
                if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
                    return FALSE;
            }
            sampling = MAKE_TAG_DATA(wave_data+cur+4);
            cur+=chunkSize;
            if( ((freq == SNDEX_FREQUENCY_32730)&&(sampling != 32730))||((freq == SNDEX_FREQUENCY_47610)&&(sampling != 47610)) )
                return FALSE;
            fFmt = TRUE;
            break;
        case FOURCC_data:
            raw_len = chunkSize;
            fData = TRUE;
            break;
        default:
            cur+=chunkSize;
            break;
        }
        if(fFmt && fData)
            break;
        wave_len -= chunkSize;
    }
    if(!(fFmt && fData))
        return FALSE;

    // When headphones are connected, turn off the system speaker
    // Insert an approximately 60-msec wait before playback of the shutter sound to stabilize output
    // This wait should be inserted in any situation to unify the sense of operation
    OS_SpinWait(67 * DSP_WAIT_SHUTTER * 1000);	// Approximately 60 msec
    
    // Change the sound output mode in the function
    while(SNDEXi_PreProcessForShutterSound() != SNDEX_RESULT_SUCCESS)
    {
        OS_Sleep(1); // Retry until successful
    }

    {
        u32     ctrl = DSP_AUDIO_DRIVER_MODE_MONAURAL;
        // Reduce the shutter sound on the left and right 50%
        ctrl |= DSP_AUDIO_DRIVER_MODE_HALFVOL;
        DSPi_PlaySoundEx((wave_data+cur), raw_len, ctrl, DSP_SOUND_PRIORITY_SHUTTER);
        DSPiPlayingShutter = TRUE;
    }

    return TRUE;
}
#else // The purpose of the check is only for not making the shutter sound with a different frequency
BOOL DSPi_PlayShutterSoundCore(const void *src, u32 len)
{
#pragma unused(len)
    u32 cur;
    u32 sampling;
    u32 raw_len;
    u8* wave_data = (u8*)src;
    static SNDEXFrequency freq;

    if(len < 44)
        return FALSE;
    
    if(MAKE_TAG_DATA(wave_data) != FOURCC_RIFF)
        return FALSE;

    if(MAKE_TAG_DATA(wave_data+8) != FOURCC_WAVE)
        return FALSE;

    cur = 24;
    sampling = MAKE_TAG_DATA(wave_data+cur);
    
    // Check the sampling rate from the format information
    // If not playing back the shutter sound, get the frequency
    if(!DSPi_IsShutterSoundPlayingCore())
    {
        if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            return FALSE;
    }
    if( ((freq == SNDEX_FREQUENCY_32730)&&(sampling != 32730))||((freq == SNDEX_FREQUENCY_47610)&&(sampling != 47610)) )
        return FALSE;
    
    cur += 16;
    raw_len = MAKE_TAG_DATA(wave_data+cur);
    cur += 4;

    if(len < cur+raw_len)
        return FALSE;
       
    // When headphones are connected, turn off the system speaker
    // Insert an approximately 60-msec wait before playback of the shutter sound to stabilize output
    // This wait should be inserted in any situation to unify the sense of operation
    OS_SpinWait(67 * DSP_WAIT_SHUTTER * 1000);	// Approximately 60 msec

    // Change the sound output mode in the function
    while(SNDEXi_PreProcessForShutterSound() != SNDEX_RESULT_SUCCESS)
    {
        OS_Sleep(1); // Retry until successful
    }

    {
        u32     ctrl = DSP_AUDIO_DRIVER_MODE_MONAURAL;
        // Reduce the shutter sound on the left and right 50%
        ctrl |= DSP_AUDIO_DRIVER_MODE_HALFVOL;
        DSPi_PlaySoundEx((wave_data+cur), raw_len, ctrl, DSP_SOUND_PRIORITY_SHUTTER);
        DSPiPlayingShutter = TRUE;
    }

    return TRUE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSound

  Description:  Stops playback from DSP sound output.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StopSoundCore(void)
{
    // Ignore if the component has not started anything up
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context && DSPiSoundPlaying)
    {
        int     ctrl = 0;
        ctrl |= DSP_AUDIO_DRIVER_TARGET_OUTPUT;
        ctrl |= DSP_AUDIO_DRIVER_CONTROL_STOP;
        {
            DSPAudioDriverCommand   command;
            command.ctrl = DSP_32BIT_TO_DSP(ctrl);
            command.buf = DSP_32BIT_TO_DSP(0);
            command.len = DSP_32BIT_TO_DSP(0);
            command.opt = DSP_32BIT_TO_DSP(0);
            DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsSoundPlaying

  Description:  Determines whether DSP sound output is currently in playback.
  
  Arguments:    None.
  
  Returns:      TRUE if DSP sound output is currently in playback.
 *---------------------------------------------------------------------------*/
BOOL DSPi_IsSoundPlayingCore(void)
{
    return DSPiSoundPlaying;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_IsShutterSoundPlaying

  Description:  Determines whether DSP sound output is playing back the current shutter sound.
  
  Arguments:    None.
  
  Returns:      TRUE if DSP sound output is playing back the current shutter sound.
 *---------------------------------------------------------------------------*/
BOOL DSPi_IsShutterSoundPlayingCore(void)
{
    return (DSPiSoundPlaying | DSPiPlayingShutter);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StartSampling

  Description:  Samples microphone sound directly from DSP sound input.
  
  Arguments:    buffer: Ring buffer used in sampling
                         It is necessary to align the boundary to 16-bit integer multiples
                length: Ring buffer size
                         It is necessary to align the boundary to 16-bit integer multiples
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StartSamplingCore(void *buffer, u32 length)
{
    SDK_ALIGN2_ASSERT(buffer);
    SDK_ALIGN2_ASSERT(length);
    {
        // Ignore if the component has not started anything up
        DSPProcessContext  *context = DSP_FindProcess(NULL);
        if (context)
        {
            int     ctrl = 0;
            ctrl |= DSP_AUDIO_DRIVER_TARGET_INPUT;
            ctrl |= DSP_AUDIO_DRIVER_CONTROL_START;
            DSPiLocalRingLength = length;
            DSPiLocalRingBuffer = (u8 *)buffer;
            DSPiLocalRingOffset = 0;
            DSPiAudioCaptureAddress = 0;
            DSPiReadPosition = 0;
            // Set the callback for completion notification
            DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
            {
                DSPAudioDriverCommand   command;
                command.ctrl = DSP_32BIT_TO_DSP(ctrl);
                command.buf = DSP_32BIT_TO_DSP(0);
                command.len = DSP_32BIT_TO_DSP(0);
                command.opt = DSP_32BIT_TO_DSP(0);
                DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_StopSampling

  Description:  Stops sampling from DSP sound input.
  
  Arguments:    None.

  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_StopSamplingCore(void)
{
    // Ignore if the component has not started anything up
    DSPProcessContext  *context = DSP_FindProcess(NULL);
    if (context)
    {
        int     ctrl = 0;
        ctrl |= DSP_AUDIO_DRIVER_TARGET_INPUT;
        ctrl |= DSP_AUDIO_DRIVER_CONTROL_STOP;
        // Set the callback for completion notification
        DSP_SetPipeCallback(DSP_PIPE_AUDIO, DSPi_PipeCallbackForSound, NULL);
        {
            DSPAudioDriverCommand   command;
            command.ctrl = DSP_32BIT_TO_DSP(ctrl);
            command.buf = DSP_32BIT_TO_DSP(0);
            command.len = DSP_32BIT_TO_DSP(0);
            command.opt = DSP_32BIT_TO_DSP(0);
            DSP_WriteProcessPipe(context, DSP_PIPE_AUDIO, &command, sizeof(command));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncSamplingBuffer

  Description:  Reads only the updated portions from the ring buffer in DSP to the ARM9 processor.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_SyncSamplingBufferCore(void)
{
    // If not receiving the monitoring address of DSP, do nothing
    if (DSPiAudioCaptureAddress != 0)
    {
        // Get the capture information in DSP and read if not read
        DSP_LoadData(DSP_ADDR_TO_ARM(DSPiAudioCaptureAddress), &DSPiAudioCapture, sizeof(DSPiAudioCapture));
        if (DSPiAudioCapture.currentPosition != DSPiReadPosition)
        {
            // Because the sizes of the ring buffers are different on the DSP and ARM9 processor, be careful of the end as you copy them
            // 
            int     cur = DSPiAudioCapture.currentPosition;
            int     end = (DSPiReadPosition > cur) ? DSPiAudioCapture.bufferLength : cur;
            int     len = end - DSPiReadPosition;
            while (len > 0)
            {
                int     segment = (int)MATH_MIN(len, DSP_WORD_TO_DSP32(DSPiLocalRingLength - DSPiLocalRingOffset));
                DSP_LoadData(DSP_ADDR_TO_ARM(DSPiAudioCapture.bufferAddress + DSPiReadPosition),
                             &DSPiLocalRingBuffer[DSPiLocalRingOffset], DSP_WORD_TO_ARM(segment));
                len -= segment;
                DSPiReadPosition += segment;
                if (DSPiReadPosition >= DSPiAudioCapture.bufferLength)
                {
                    DSPiReadPosition -= DSPiAudioCapture.bufferLength;
                }
                DSPiLocalRingOffset += (int)DSP_WORD_TO_ARM32(segment);
                if (DSPiLocalRingOffset >= DSPiLocalRingLength)
                {
                    DSPiLocalRingOffset -= DSPiLocalRingLength;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetLastSamplingAddress

  Description:  Gets the latest sampling position of the local ring buffer loaded to ARM9.
                

  Arguments:    None.

  Returns:      Latest sampling position.
 *---------------------------------------------------------------------------*/
const u8* DSPi_GetLastSamplingAddressCore(void)
{
    int     offset = DSPiLocalRingOffset - (int)sizeof(u16);
    if (offset < 0)
    {
        offset += DSPiLocalRingLength;
    }
    return &DSPiLocalRingBuffer[offset];
}

