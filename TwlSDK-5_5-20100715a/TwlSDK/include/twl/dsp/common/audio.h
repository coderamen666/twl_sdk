/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dspcomponents - include - dsp
  File:     audio.h

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-25#$
  $Rev: 8651 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#ifndef TWLSDK_DSP_AUDIO_H__
#define TWLSDK_DSP_AUDIO_H__


#include <twl/dsp/common/byteaccess.h>


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Declarations */

// Audio component structures
typedef struct DSPAudioCodecCommand
{
    DSPByte32       ctrl;
    DSPAddrInARM    src;
    DSPAddrInARM    dst;
    DSPByte32       len;
}
DSPAudioCodecCommand;

typedef struct DSPAudioDriverCommand
{
    DSPByte32       ctrl;
    DSPAddrInARM    buf;
    DSPByte32       len;
    DSPByte32       opt;
}
DSPAudioDriverCommand;

typedef struct DSPAudioDriverResponse
{
    DSPByte32       ctrl;
    DSPByte32       result;
}
DSPAudioDriverResponse;


/*---------------------------------------------------------------------------*/
/* Constants */

// G.711 codec types
typedef DSPByte32 DSPAudioCodecMode;
#define DSP_AUDIO_CODEC_TYPE_MASK       (DSPAudioCodecMode)0x0F00
#define DSP_AUDIO_CODEC_MODE_MASK       (DSPAudioCodecMode)0x00FF
#define DSP_AUDIO_CODEC_TYPE_ENCODE     (DSPAudioCodecMode)0x0100
#define DSP_AUDIO_CODEC_TYPE_DECODE     (DSPAudioCodecMode)0x0200
#define DSP_AUDIO_CODEC_MODE_G711_ALAW  (DSPAudioCodecMode)0x0001
#define DSP_AUDIO_CODEC_MODE_G711_ULAW  (DSPAudioCodecMode)0x0002

// Driver control commands
#define DSP_AUDIO_DRIVER_TARGET_MASK    (DSPByte32)0xF000
#define DSP_AUDIO_DRIVER_CONTROL_MASK   (DSPByte32)0x0F00
#define DSP_AUDIO_DRIVER_MODE_MASK      (DSPByte32)0x00FF
#define DSP_AUDIO_DRIVER_TARGET_OUTPUT  (DSPByte32)0x1000
#define DSP_AUDIO_DRIVER_TARGET_INPUT   (DSPByte32)0x2000
#define DSP_AUDIO_DRIVER_TARGET_CACHE   (DSPByte32)0x3000
#define DSP_AUDIO_DRIVER_CONTROL_START  (DSPByte32)0x0100
#define DSP_AUDIO_DRIVER_CONTROL_STOP   (DSPByte32)0x0200
#define DSP_AUDIO_DRIVER_CONTROL_LOAD   (DSPByte32)0x0300
#define DSP_AUDIO_DRIVER_CONTROL_STORE  (DSPByte32)0x0400
#define DSP_AUDIO_DRIVER_MODE_MONAURAL  (DSPByte32)0x0000
#define DSP_AUDIO_DRIVER_MODE_STEREO    (DSPByte32)0x0001
#define DSP_AUDIO_DRIVER_MODE_HALFVOL   (DSPByte32)0x0002


/*---------------------------------------------------------------------------*/
/* Functions */

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
 * Internal core functions placed on LTDMAIN
 *---------------------------------------------------------------------------*/

void DSPi_OpenStaticComponentAudioCore(FSFile *file);
BOOL DSPi_LoadAudioCore(FSFile *file, int slotB, int slotC);
void DSPi_UnloadAudioCore(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_OpenStaticComponentAudio

  Description:  Opens the memory file for an audio component.
                Although you will no longer have to make file system preparations in advance, this will be linked as static memory and thus increase the program size.
                
  
  Arguments:    file: The FSFile structure to open the memory file.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_OpenStaticComponentAudio(FSFile *file)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_OpenStaticComponentAudioCore(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadAudio

  Description:  Loads an audio component into the DSP.
  
  Arguments:    file: The audio component file.
                slotB: WRAM-B that was allowed to be used for code memory.
                slotC: WRAM-C that was allowed to be used for data memory.
  
  Returns:      TRUE if the audio component is loaded properly in the DSP.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_LoadAudio(FSFile *file, int slotB, int slotC)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_LoadAudioCore(file, slotB, slotC);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadAudio

  Description:  Unloads the audio component from the DSP.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_UnloadAudio(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_UnloadAudioCore();
    }
}


#else


#endif // SDK_TWL


#ifdef __cplusplus
}
#endif


#endif // TWLSDK_DSP_AUDIO_H__
