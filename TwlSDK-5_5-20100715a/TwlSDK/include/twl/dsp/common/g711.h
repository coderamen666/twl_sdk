/*---------------------------------------------------------------------------*
  Project:  TwlSDK - dspcomponents - include - dsp - audio
  File:     g711.h

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
#ifndef DSP_AUDIO_G711_H__
#define DSP_AUDIO_G711_H__


#include <twl/dsp/common/pipe.h>
#include <twl/dsp/common/audio.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
 * Internal core functions placed on LTDMAIN
 *---------------------------------------------------------------------------*/

void DSPi_OpenStaticComponentG711Core(FSFile *file);
BOOL DSPi_LoadG711Core(FSFile *file, int slotB, int slotC);
void DSPi_UnloadG711Core(void);
void DSPi_EncodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
void DSPi_DecodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
BOOL DSPi_TryWaitForG711Core(void);
void DSPi_WaitForG711Core(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_OpenStaticComponentG711

  Description:  Opens the memory file for a G.711 component.
                Although you will no longer have to make file system preparations in advance, this will be linked as static memory and thus increase the program size.
                
  
  Arguments:    file: The FSFile structure to open the memory file.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_OpenStaticComponentG711(FSFile *file)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_OpenStaticComponentG711Core(file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadG711

  Description:  Loads a component that supports the G.711 encoding method into the DSP.
  
  Arguments:    file: G.711 component file.
                slotB: WRAM-B that was allowed to be used for code memory.
                slotC: WRAM-C that was allowed to be used for data memory.
  
  Returns:      TRUE if the G.711 component is loaded properly in the DSP.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_LoadG711(FSFile *file, int slotB, int slotC)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_LoadG711Core(file, slotB, slotC);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_UnloadG711

  Description:  Unloads a component that supports the G.711 encoding method from the DSP.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_UnloadG711(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_UnloadG711Core();
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EncodeG711

  Description:  G.711 encoding.
  
  Arguments:    dst: Destination buffer for data conversion. (8-bit A-law/u-law)
                src: Buffer with the data to convert. (16-bit PCM)
                len: Number of samples to convert.
                mode: Codec type.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_EncodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_EncodeG711Core(dst, src, len, mode);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DecodeG711

  Description:  G.711 decoding.
  
  Arguments:    dst: Destination buffer for data conversion. (16-bit PCM)
                src: Buffer with the data to convert. (8-bit A-law/u-law)
                len: Number of samples to convert.
                mode: Codec type.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_DecodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_DecodeG711Core(dst, src, len, mode);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSP_TryWaitForG711

  Description:  Determines whether the last issued command has completed.
  
  Arguments:    None.
  
  Returns:      TRUE if the last issued command has completed.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSP_TryWaitForG711(void)
{
    if (OS_IsRunOnTwl())
    {
        return DSPi_TryWaitForG711Core();
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_WaitForG711

  Description:  Waits for the last issued command to complete.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void DSP_WaitForG711(void)
{
    if (OS_IsRunOnTwl())
    {
        DSPi_WaitForG711Core();
    }
}

#else

void DSP_EncodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);
void DSP_DecodeG711(void *dst, const void *src, u32 len, DSPAudioCodecMode mode);

#endif // SDK_TWL


#ifdef __cplusplus
}
#endif


#endif // DSP_AUDIO_G711_H__
