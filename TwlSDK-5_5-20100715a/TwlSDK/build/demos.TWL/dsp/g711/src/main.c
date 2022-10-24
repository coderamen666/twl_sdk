/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - DSP - g711
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-05-17#$
  $Rev: 11335 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/


#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>

#include <DEMO.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    // OS initialization
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);
    SNDEX_Init();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    // Initialize screen display
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    // Load the DSP components
    {
        FSFile  file[1];
        int     slotB;
        int     slotC;
        // In this sample, all of the WRAM is allocated for the DSP, but for actual use, you would only need as many slots as were necessary.
        // 
        (void)MI_FreeWram_B(MI_WRAM_ARM9);
        (void)MI_CancelWram_B(MI_WRAM_ARM9);
        (void)MI_FreeWram_C(MI_WRAM_ARM9);
        (void)MI_CancelWram_C(MI_WRAM_ARM9);
        (void)MI_FreeWram_B( MI_WRAM_ARM7 );
        (void)MI_CancelWram_B( MI_WRAM_ARM7 );
        (void)MI_FreeWram_C( MI_WRAM_ARM7 );
        (void)MI_CancelWram_C( MI_WRAM_ARM7 );
        slotB = 0xFF;
        slotC = 0xFF;
        // For simplification, this sample uses files in static memory.
        DSP_OpenStaticComponentG711(file);
        if (!DSP_LoadG711(file, slotB, slotC))
        {
            OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
        }
    }

    // Confirm that encryption and decryption will work correctly with G.711.
    {
        enum { sample_wave_max = 512 * 1024 };
        static u8   encbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        static u8   srcbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        static u8   dstbuf[sample_wave_max] ATTRIBUTE_ALIGN(4);
        u32         length;
        // Begin by preparing a PCM waveform for converting.
        {
            FSFile  file[1];
            u32     chunk;
            FS_InitFile(file);
            if (!FS_OpenFileEx(file, "rom:/fanfare.32.wav", FS_FILEMODE_R) ||
                !FS_SeekFile(file, 0x24, FS_SEEK_SET) ||
                (FS_ReadFile(file, &chunk, 4) != 4) ||
                (chunk != (('d'<<0)|('a'<<8)|('t'<<16) |('a'<<24))) ||
                (FS_ReadFile(file, &length, 4) != 4) ||
                (FS_ReadFile(file, srcbuf, (s32)length) != length))
            {
                OS_TPanic("cannot prepare sample waveform!");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(srcbuf, length);
        }
        // Standardize units to sample counts.
        length /= sizeof(u16);
        // Encrypt the resulting PCM waveform in A-law format.
        DSP_EncodeG711(encbuf, srcbuf, length, DSP_AUDIO_CODEC_MODE_G711_ALAW);
        // Wait for completion here.
        DSP_WaitForG711();
        // Decrypt the resulting A-law format to a PCM waveform.
        DSP_DecodeG711(dstbuf, encbuf, length, DSP_AUDIO_CODEC_MODE_G711_ALAW);
        // Wait for completion here.
        DSP_WaitForG711();

        // Press the A button to see whether the sound will play. This is not directly related to G.711, however.
        (void)SNDEX_SetDSPMixRate(0);
        for (;;)
        {
            DEMOReadKey();
            if (DEMO_IS_TRIG(PAD_BUTTON_A))
            {
                DSP_PlaySound(srcbuf, length*2, FALSE); // Because 'length' is a sample count, correct it to bytes
            }
            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }

    // Unload the DSP components if they are no longer needed.
    DSP_UnloadG711();

    // Because the sample is so simplistic, there's nothing to display.
    OS_Terminate();
}
