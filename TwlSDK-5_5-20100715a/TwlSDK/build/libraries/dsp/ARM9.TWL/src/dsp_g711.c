/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_g711.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-21#$
  $Rev: 9387 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>

#include <twl/dsp/common/pipe.h>
#include <twl/dsp/common/g711.h>

#include "dsp_process.h"


/*---------------------------------------------------------------------------*/
/* Variables */

static BOOL DSPiG711Available = FALSE;
static u32  DSPiG711CommandSend = 0;
static u32  DSPiG711CommandRecv = 0;
static DSPProcessContext DSPiProcessG711[1];


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         DSPi_OpenStaticComponentG711Core

  Description:  Open the memory file for the G.711 component.
                It is no longer necessary to prepare a file system in advance. Because it is linked as static memory, the program size increases.
                
  
  Arguments:    file: FSFile structure that opens the memory file
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_OpenStaticComponentG711Core(FSFile *file)
{
    extern const u8 DSPiFirmware_audio[];
    (void)DSPi_CreateMemoryFile(file, DSPiFirmware_audio);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_SendCodecG711

  Description:  Issues command to the the G.711 codec.
  
  Arguments:    dst: Conversion destination buffer. (Pack stream for A-law or u-law)
                src: Conversion source buffer. (PCM 8kHz * 8bits)
                len: Conversion sample count
                flags: Bit set for codec and conversion direction
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_SendCodecG711(void *dst, const void *src, u32 len, DSPAudioCodecMode flags)
{
    DSPAudioCodecCommand    command;
    command.ctrl = DSP_32BIT_TO_DSP(flags);
    command.src = DSP_32BIT_TO_DSP((DSPAddrInARM)src);
    command.dst = DSP_32BIT_TO_DSP((DSPAddrInARM)dst);
    command.len = DSP_32BIT_TO_DSP(len);
    DSP_WriteProcessPipe(DSPiProcessG711, DSP_PIPE_BINARY, &command, sizeof(command));

    ++DSPiG711CommandSend;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_LoadG711Core

  Description:  Load the component that supports the G.711 encoding format to DSP
  
  Arguments:    file: G.711 component file
                slotB: WRAM-B allowed for use for code memory
                slotC: WRAM-C allowed for use for data memory
  
  Returns:      TRUE if the G.711 component is correctly loaded to DSP.
 *---------------------------------------------------------------------------*/
BOOL DSPi_LoadG711Core(FSFile *file, int slotB, int slotC)
{
    if (!DSPiG711Available)
    {
        DSP_InitProcessContext(DSPiProcessG711, "g711");
        if (DSP_ExecuteProcess(DSPiProcessG711, file, slotB, slotC))
        {
            DSPiG711Available = TRUE;
        }
    }
    return DSPiG711Available;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_UnloadG711Core

  Description:  Unloads the component that supports the G.711 encoding format from DSP.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_UnloadG711Core(void)
{
    if (DSPiG711Available)
    {
        DSP_QuitProcess(DSPiProcessG711);
        DSPiG711Available = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_EncodeG711Core

  Description:  G.711 encoding.
  
  Arguments:    dst: Conversion destination buffer. (Pack stream for A-law or u-law)
                src: Conversion source buffer. (PCM 8kHz * 8bits)
                len: Conversion sample count
                mode: Codec type
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_EncodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    DSPi_SendCodecG711(dst, src, len,
                       (DSPAudioCodecMode)(DSP_AUDIO_CODEC_TYPE_ENCODE |
                                           (mode & DSP_AUDIO_CODEC_MODE_MASK)));
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_DecodeG711Core

  Description:  G.711 decoding.
  
  Arguments:    dst: Conversion destination buffer. (PCM 8kHz * 8bits)
                src: Conversion source buffer. (Pack stream for A-law or u-law)
                len: Conversion sample count
                mode: Codec type
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_DecodeG711Core(void *dst, const void *src, u32 len, DSPAudioCodecMode mode)
{
    DSPi_SendCodecG711(dst, src, len,
                       (DSPAudioCodecMode)(DSP_AUDIO_CODEC_TYPE_DECODE |
                                           (mode & DSP_AUDIO_CODEC_MODE_MASK)));
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_TryWaitForG711Core

  Description:  Determines whether the command that was first issued is completed.
  
  Arguments:    None.
  
  Returns:      TRUE if the command that was first issued is completed.
 *---------------------------------------------------------------------------*/
BOOL DSPi_TryWaitForG711Core(void)
{
    BOOL    retval = (DSPiG711CommandSend == DSPiG711CommandRecv);
    if (!retval)
    {
        DSPByte32   ack;
        DSPPipe     binin[1];
        (void)DSP_LoadPipe(binin, DSP_PIPE_BINARY, DSP_PIPE_INPUT);
        if (DSP_GetPipeReadableSize(binin) >= sizeof(ack))
        {
            DSP_ReadPipe(binin, &ack, sizeof(ack));
            ++DSPiG711CommandRecv;
            retval = (DSPiG711CommandSend == DSPiG711CommandRecv);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_WaitForG711Core

  Description:  Waits for the command that was first issued to be completed.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSPi_WaitForG711Core(void)
{
    if (DSPiG711CommandSend != DSPiG711CommandRecv)
    {
        DSPByte32   ack;
        DSPPipe     binin[1];
        (void)DSP_LoadPipe(binin, DSP_PIPE_BINARY, DSP_PIPE_INPUT);
        while (DSPiG711CommandSend != DSPiG711CommandRecv)
        {
            DSP_ReadPipe(binin, &ack, sizeof(ack));
            ++DSPiG711CommandRecv;
        }
    }
}
