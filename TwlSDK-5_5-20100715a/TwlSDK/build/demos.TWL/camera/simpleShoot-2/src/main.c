/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CAMERA - demos - simpleShoot-2
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-21#$
  $Rev: 10434 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "DEMO.h"
#include <twl/camera.h>
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>

#include    "snd_data.h"

#define NDMA_NO      1           // The NDMA number (0 to 3) to use
#define WIDTH       256         // Image width
#define HEIGHT      192         // Image height

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // Number of lines transferred in one cycle
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // Number of bytes in one line's transfer

void VBlankIntr(void);
static BOOL CameraInit(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void CameraDmaRecvIntr(void* arg);

static int wp;  // Buffer while capturing data from camera
static int rp;  // Buffer most recently copied to VRAM
static BOOL wp_pending; // Data capture was cancelled (recapture to same buffer)
static u32 stabilizedCount;
static u16 buffer[2][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);

static BOOL switchFlag;

#define STRM_BUF_SIZE 1024*64

static char StrmBuf_shutter[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_shutter;
static char StrmBuf_begin[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_begin;
static char StrmBuf_end[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u32 len_end;

// Filename
static const char filename_shutter[] = "shutter_sound/shutter_sound_32730.wav";
static const char filename_begin[] = "videorec_sound/videorec_sound_begin_32730.wav";
static const char filename_end[] = "videorec_sound/videorec_sound_end_32730.wav";

static void DebugReport(void)
{
    OS_TPrintf("\nCapture to No.%d\tDisplay from No.%d\n", wp, rp);
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// Buffer storing photographed/filmed images
static u16 shoot_buffer[WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
static int shoot_flag = 0;

// REC icon
extern u16 _binary_rec_icon_nbfs[];

static u16* movie_buffer; // Video buffer
static u32  movie_buffer_max_count = 0;
static u32  movie_buffer_count = 0;
static int  movie_flag = 0;

static CAMERASelect current_camera = CAMERA_SELECT_IN;

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
    if (wp == rp)
    {
        rp ^= 1;
        MI_CpuCopyFast(buffer[rp], (void*)HW_LCDC_VRAM_A, BYTES_PER_LINE * HEIGHT);
        if(movie_flag == 1)
        {
            int i = 0;
            MI_CpuCopy(_binary_rec_icon_nbfs+1, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*1), sizeof(u16) * 30);
            for(i = 1; i < 12; i++)
                MI_CpuCopy(_binary_rec_icon_nbfs+32*i, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*256*i), sizeof(u16) * 32);
            MI_CpuCopy(_binary_rec_icon_nbfs+32*12+1, (void*)(HW_LCDC_VRAM_A + sizeof(u16)*256*12+sizeof(u16)*1), sizeof(u16) * 30);
        }
        DC_InvalidateRange(buffer[rp], BYTES_PER_LINE * HEIGHT);
        if(movie_flag == 1)
        {
            MI_CpuCopyFast(buffer[rp], (void*)(movie_buffer+(WIDTH*HEIGHT*movie_buffer_count)), BYTES_PER_LINE * HEIGHT);
            movie_buffer_count++;
            if(movie_buffer_count > movie_buffer_max_count)
                movie_flag = 2;
        }
    }
    if(shoot_flag == 1)
    {
        MI_CpuCopyFast(buffer[rp], (void*)shoot_buffer, BYTES_PER_LINE * HEIGHT);
        shoot_flag = 2;
    }
}

//--------------------------------------------------------------------------------
//    Camera initialization (only the Init- and I2C-related initialization)
//
BOOL CameraInit(void)
{
    CAMERAResult result;
    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;

    result = CAMERA_I2CEffect(CAMERA_SELECT_IN, CAMERA_EFFECT_NONE);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
    }

    result = CAMERA_I2CActivate(current_camera);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CActivate was failed. (%d)\n", result);
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;
    stabilizedCount = 0;

    return TRUE;
}

//--------------------------------------------------------------------------------
//    Camera interrupt process (occurs both when there is an error and for Vsync)
//
#define PRINT_RATE  32
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // Camera stop processing
    CAMERA_StopCapture();
    MI_StopNDma(NDMA_NO);
    CAMERA_ClearBuffer();
    wp_pending = TRUE;      // Also use same frame next time
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_StartCapture();
}

void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // Restore was not possible, even after restarting camera
    }
    CameraIntrError(result); // DMA synchronization might have drifted, so realign
}

void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    if(stabilizedCount <= 30)
        stabilizedCount++;
    if(switchFlag)
    {
        current_camera = (current_camera == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
        result = CAMERA_I2CActivate(current_camera);
        if(result == CAMERA_RESULT_FATAL_ERROR)
            OS_Panic("CAMERA FATAL ERROR\n");
        stabilizedCount = 0;
        switchFlag = FALSE;
    }
}

void CameraDmaRecvIntr(void* arg)
{
#pragma unused(arg)
    MI_StopNDma(NDMA_NO);

    if (CAMERA_IsBusy() == TRUE)
    {
        OS_TPrintf(".");
        if (MI_IsNDmaBusy(NDMA_NO)) // Check whether image transfer is complete
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma(NDMA_NO);
        }
        // Start the next frame capture
        if (wp_pending)
        {
            wp_pending = FALSE;
        }
        else
        {
            // Capture results are not displayed on screen until camera is stable
            // This demo waits the minimum of four camera frames required to avoid chromatic aberration, but if you want to wait until auto-exposure is stable, you need to wait 14 frames for an indoor shot or 30 for an outdoor one, as indicated in the Function Reference.
            // 
            if(stabilizedCount > 4)
            {
                wp ^= 1;
            }
        }
        CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    }

    {
        static OSTick begin = 0;
        static int count = 0;
        if (begin == 0)
        {
            begin = OS_GetTick();
        }
        else if (++count == PRINT_RATE)
        {
            OSTick uspf = OS_TicksToMicroSeconds(OS_GetTick() - begin) / count;
            int mfps = (int)(1000000000LL / uspf);
            OS_TPrintf("%2d.%03d fps\n", mfps / 1000, mfps % 1000);
            count = 0;
            begin = OS_GetTick();
        }
    }
}

void TwlMain(void)
{
    int     vram_slot = 0, count = 0;
    CAMERAResult result;

    //---------------------------------------------------------------------------
    // Initialize:
    // Enable IRQ interrupts and initialize VRAM
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    // DMA is not used in GX (the old DMA conflicts with camera DMA)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    GXS_SetGraphicsMode(GX_BGMODE_3);

    GXS_SetVisiblePlane(GX_PLANEMASK_BG3);
    G2S_SetBG3Priority(0);
    G2S_BG3Mosaic(FALSE);

    G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);

    MI_CpuClearFast(shoot_buffer, WIDTH*HEIGHT*sizeof(u16));
    GXS_LoadBG3Bmp(shoot_buffer, 0, WIDTH*HEIGHT*sizeof(u16));

    GXS_DispOn();

    // Initialization
    OS_InitThread();
    OS_InitTick();
    OS_InitAlarm();
    FS_Init( FS_DMA_NOT_USE );

    // V-Blank interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    // VRAM display mode
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, BYTES_PER_LINE * HEIGHT);
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    stabilizedCount = 0;
    switchFlag = FALSE;
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, GX_BGMODE_0, GX_BG0_AS_2D);
    OS_WaitVBlankIntr();
    GX_DispOn();

    movie_buffer_max_count = ((u32)OS_GetMainArenaHi() - (u32)OS_GetMainArenaLo()) / (BYTES_PER_LINE * HEIGHT) - 2;
    movie_buffer = (u16*)OS_AllocFromMainArenaLo( (u32)OS_GetMainArenaHi() - (u32)OS_GetMainArenaLo(), 32);

    // Initialize camera
    (void)CameraInit();

    // Configure DMA interrupt
    (void)OS_EnableIrqMask(OS_IE_NDMA1);

    // Camera VSYNC interrupt callback
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // Camera error interrupt callback
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // Camera restart completion callback
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetTrimmingParams(0, 0, WIDTH, HEIGHT);
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // Start capturing
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    DEMOStartDisplay();

    // Initialize the DSP shutter sound
    {
        FSFile  file[1];
        int     slotB;
        int     slotC;
        // In this sample, all of the WRAM is allocated for the DSP, but for actual use, you would only need as many slots as were necessary
        // 
        (void)MI_FreeWram_B(MI_WRAM_ARM9);
        (void)MI_CancelWram_B(MI_WRAM_ARM9);
        (void)MI_FreeWram_C(MI_WRAM_ARM9);
        (void)MI_CancelWram_C(MI_WRAM_ARM9);
        slotB = 0xFF;
        slotC = 0xFF;
        // For simplification, this sample uses files in static memory
        FS_InitFile(file);
        DSPi_OpenStaticComponentG711Core(file);
        if (!DSP_LoadG711(file, slotB, slotC))
        {
            OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
        }
        (void)FS_CloseFile(file);

        // Initialize extended sound features
        SNDEX_Init();

        // Load the shutter sound
        {
            FSFile file[1];
            SNDEXFrequency freq;

            if(SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            {
                OS_Panic("failed SNDEX_GetI2SFrequency");
            }
            // The initial I2S operating frequency should be 32.73 kHz
            if(freq != SNDEX_FREQUENCY_32730)
            {
                OS_Panic("default value is SNDEX_FREQUENCY_32730");
            }

            FS_InitFile(file);
            // Load the 32.73-kHz shutter sound
            if( !FS_OpenFileEx(file, filename_shutter, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_shutter = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_shutter, (s32)len_shutter) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_shutter, len_shutter);
            
            FS_InitFile(file);
            // Load the 32.73-kHz video filming start sound
            if( !FS_OpenFileEx(file, filename_begin, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_begin = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_begin, (s32)len_begin) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_begin, len_begin);
            
            FS_InitFile(file);
            // Load the 32.73-kHz video filming end sound
            if( !FS_OpenFileEx(file, filename_end, FS_FILEMODE_R) )
            {
                OS_Panic("failed FS_OpenFileEx");
            }
            len_end = FS_GetFileLength(file);
            if( FS_ReadFile(file, StrmBuf_end, (s32)len_end) == -1)
            {
                OS_Panic("failed FS_ReadFile");
            }
            (void)FS_CloseFile(file);
            DC_StoreRange(StrmBuf_end, len_end);
        }
    }

    /* Initialize sound */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);

    while (1)
    {
        // Reading pad information and controls
        DEMOReadKey();

        if (PAD_DetectFold() == TRUE)
        {
            // Wait until the shutter sound is done if it is playing
            while(DSP_IsShutterSoundPlaying())
            {
                OS_Sleep(100);
            }
            
            // When filming video, you must enter sleep mode after filming is complete
            if((movie_flag == 1) || (movie_flag == 2))
            {
                while(DSP_PlayShutterSound(StrmBuf_end, len_end) == FALSE)
                {
                }
                movie_flag = 0;
                // When the outer camera finishes filming a video, the outer camera LED is turned on again
                if(current_camera == CAMERA_SELECT_OUT)
                {
                    if(CAMERA_SetLED(FALSE) != CAMERA_RESULT_SUCCESS)
                        OS_TPanic("CAMERA_SetLED(FALSE) was failed.\n");
                }
                // Enter sleep only after confirming that the video filming end sound has finished playing
                while(DSP_IsShutterSoundPlaying())
                {
                    OS_Sleep(100);
                }
            }

            
            // Stop the DSP before sleeping
            DSP_UnloadG711();

            // Camera stop processing
            CAMERA_StopCapture();
            while (CAMERA_IsBusy() == TRUE)
            {
                ;
            }
            MI_StopNDma(NDMA_NO);
            result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
            if(result == CAMERA_RESULT_FATAL_ERROR)
                OS_Panic("CAMERA FATAL ERROR\n");
            wp_pending = TRUE;      // Also use same frame next time

            PM_GoSleepMode(PM_TRIGGER_COVER_OPEN | PM_TRIGGER_CARD, 0, 0);

            // Restart DSP after recovering from sleep
            {
                FSFile  file[1];
                int     slotB;
                int     slotC;

                slotB = 0xFF;
                slotC = 0xFF;
                // For simplification, this sample uses files in static memory
                FS_InitFile(file);
                DSPi_OpenStaticComponentG711Core(file);
                if (!DSP_LoadG711(file, slotB, slotC))
                {
                    OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
                }
                (void)FS_CloseFile(file);
            }
            // Because the camera is also in standby, make it active
            result = CAMERA_I2CActivate(current_camera);
            if(result == CAMERA_RESULT_FATAL_ERROR)
                OS_Panic("CAMERA FATAL ERROR\n");
            CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
            CAMERA_ClearBuffer();
            CAMERA_StartCapture();
            stabilizedCount = 0;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_A) && (shoot_flag == 0) && (movie_flag == 0))
        {
            while(DSP_PlayShutterSound(StrmBuf_shutter, len_shutter) == FALSE)
            {
            }
            // When taking still photos with the outer camera, temporarily turn the outer camera LED off
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SwitchOffLED() != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SwitchOffLED was failed.\n");
            }
            OS_Sleep(200);
            shoot_flag = 1;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_Y) && (movie_flag == 0) && (shoot_flag == 0))
        {
            while(DSP_PlayShutterSound(StrmBuf_begin, len_begin) == FALSE)
            {
            }
            movie_buffer_count = 0;
            movie_flag = 1;
            // When filming video with the outer camera, make the outer camera LED blink
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SetLED(TRUE) != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SetLED(TRUE) was failed.\n");
            }
        }
        else if(((DEMO_IS_TRIG(PAD_BUTTON_Y)) && (movie_flag == 1)) || (movie_flag == 2))
        {
            while(DSP_PlayShutterSound(StrmBuf_end, len_end) == FALSE)
            {
            }
            movie_flag = 0;
            // When the outer camera finishes filming a video, the outer camera LED is turned on again
            if(current_camera == CAMERA_SELECT_OUT)
            {
                if(CAMERA_SetLED(FALSE) != CAMERA_RESULT_SUCCESS)
                    OS_TPanic("CAMERA_SetLED(FALSE) was failed.\n");
            }
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_B) && (movie_flag == 0))
        {
            movie_flag = 3;
        }

        if(DEMO_IS_TRIG(PAD_BUTTON_X) && (movie_flag == 0))
        {
            switchFlag = TRUE;
        }

        OS_WaitVBlankIntr();

        if(shoot_flag == 2)
        {
            GXS_LoadBG3Bmp((void*)((int)shoot_buffer), 0, BYTES_PER_LINE * HEIGHT);
            shoot_flag = 0;
        }

        if(movie_flag == 3)
        {
            static int frame = 0;
            static int vsync = 0;

            if(vsync == 0) // The camera is 15 fps, so update the screen once per every four V-Blanks
            {
                GXS_LoadBG3Bmp((void*)(movie_buffer+WIDTH*HEIGHT*frame), 0, BYTES_PER_LINE * HEIGHT);
                frame++;
                if(frame >= movie_buffer_count)
                {
                    frame = 0;
                    movie_flag = 0;
                }
            }
            vsync = ++vsync % 4;
        }

        // Main sound processing
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
    }
}
