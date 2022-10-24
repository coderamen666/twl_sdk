/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - demos - yuvToRgb
  File:     main.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-05-17#$
  $Rev: 11335 $
  $Author: kitase_hirotake $
*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
    This demo gets an image in YUV422 format from the camera and uses the DSP to convert to RGB555 and display it on screen.
    
    
    Exit with the START Button.
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>

#include <twl/camera.h>

#include <DEMO.h>
#include <twl/dsp/common/graphics.h>

/* Settings */
#define DMA_NO_FOR_FS          3    // For FS_Init
#define CAMERA_NEW_DMA_NO      1    // New DMA number used for CAMERA

#define CAM_WIDTH   256            // Width of image the camera gets
#define CAM_HEIGHT  192            // Height of image the camera gets

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(CAM_WIDTH)     // Number of lines transferred in one cycle
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(CAM_WIDTH)    // Number of bytes in one line's transfer

/*---------------------------------------------------------------------------*
 Prototype Declarations
*---------------------------------------------------------------------------*/
void VBlankIntr(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void InitializeGraphics(void);
static void InitializeCamera(void);
static void WriteScreenBuffer(u16 *data, u32 width, u32 height, u16 *scr);
static void ConvertCallbackFunc(void);

/*---------------------------------------------------------------------------*
 Internal Variable Definitions
*---------------------------------------------------------------------------*/
// Screen buffers for main and sub-screens
// Buffer that stores the image obtained by the camera as well as the DSP conversion results
static u16 TmpBuf[2][CAM_WIDTH * CAM_HEIGHT] ATTRIBUTE_ALIGN(32);

static BOOL StartRequest = FALSE;
static OSTick StartTick;       // Variable for measuring DSP processing time

static int wp;                  // Buffer while capturing data from camera
static int rp;                  // Buffer most recently copied to VRAM
static BOOL wp_pending;         // Data capture was cancelled (recapture to same buffer)

static BOOL IsConvertNow = FALSE;      // Whether YUV to RGB conversion is underway

/*---------------------------------------------------------------------------*
 Name:         TwlMain

 Description:  Initialization and main loop.

 Arguments:    None.

 Returns:      None.
*---------------------------------------------------------------------------*/
void TwlMain(void)
{
    FSFile file;
    
    DEMOInitCommon();
    OS_InitThread();
    OS_InitTick();
    OS_InitAlarm();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    DEMOInitVRAM();
    InitializeGraphics();
    InitializeCamera();

    DEMOStartDisplay();
    
    // DMA is not used in GX (the old DMA conflicts with camera DMA)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    // Because at first there is a possibility that WRAM might have been allocated to something as per the ROM header, clear it
    (void)MI_FreeWram_B( MI_WRAM_ARM9 );
    (void)MI_CancelWram_B( MI_WRAM_ARM9 );
    (void)MI_FreeWram_C( MI_WRAM_ARM9 );
    (void)MI_CancelWram_C( MI_WRAM_ARM9 );
    (void)MI_FreeWram_B( MI_WRAM_ARM7 );
    (void)MI_CancelWram_B( MI_WRAM_ARM7 );
    (void)MI_FreeWram_C( MI_WRAM_ARM7 );
    (void)MI_CancelWram_C( MI_WRAM_ARM7 );
    
    FS_Init(DMA_NO_FOR_FS);

    (void)OS_EnableInterrupts();

    // Load graphics component
    DSP_OpenStaticComponentGraphics(&file);
    if(!DSP_LoadGraphics(&file, 0xFF, 0xFF))
    {
        OS_TPanic("failed to load graphics DSP-component! (lack of WRAM-B/C)");
    }
    
    // Camera start
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    StartRequest = TRUE;
    CameraIntrVsync(CAMERA_RESULT_SUCCESS);
    OS_TPrintf("Camera is shooting a movie...\n");
    OS_TPrintf("Press A Button.\n");
    
    while (1)
    {
        DEMOReadKey();
        
        if (DEMO_IS_TRIG( PAD_BUTTON_START ))
        {
            break;    // Quit
        }
        
        if (wp == rp && !IsConvertNow)
        {
            rp ^= 1;
            DC_FlushRange(TmpBuf[rp], BYTES_PER_LINE * CAM_HEIGHT);
            GX_LoadBG3Scr(TmpBuf[rp], 0, BYTES_PER_LINE * CAM_HEIGHT);
        }
        
        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt
    }
    
    OS_TPrintf("demo end.\n");

    // Unload graphics component
    DSP_UnloadGraphics();
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}

// Camera interrupt
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");
    // Stopping
    CAMERA_StopCapture();           // Stop the camera
    CAMERA_ClearBuffer();           // Clear
    MI_StopNDma(CAMERA_NEW_DMA_NO); // Stop DMA
    wp_pending = TRUE;              // Also use same frame next time
    StartRequest = TRUE;            // Camera restart request
}

void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // Restore was not possible, even after restarting camera
    }
    // Camera start
    CAMERA_ClearBuffer();
    MI_StopNDma(CAMERA_NEW_DMA_NO);
    wp_pending = TRUE;              // Also use same frame next time
    StartRequest = TRUE;            // Camera restart request
}


void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    // The following is processing during V-sync
    if (StartRequest)
    {
        CAMERA_ClearBuffer();
        CAMERA_StartCapture();
        StartRequest = FALSE;
    }

    if (CAMERA_IsBusy() == FALSE)   // Done executing stop command?
    {
    }
    else
    {
        if (MI_IsNDmaBusy(CAMERA_NEW_DMA_NO))    // NOT done capturing last frame?
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MI_StopNDma(CAMERA_NEW_DMA_NO);  // Stop DMA
        }
        // Start to capture for next frame
        if (wp_pending)
        {
            wp_pending = FALSE;
        }
        else
        {
            // Change update buffer
            wp ^= 1;
            IsConvertNow = TRUE;
            StartTick = OS_GetTick();
            (void)DSP_ConvertYuvToRgbAsync(TmpBuf[rp ^1], TmpBuf[rp ^1], CAM_WIDTH * CAM_HEIGHT * sizeof(u16), ConvertCallbackFunc);
        }

        CAMERA_DmaRecvAsync(CAMERA_NEW_DMA_NO, TmpBuf[wp], CAMERA_GetBytesAtOnce(CAM_WIDTH), CAMERA_GET_FRAME_BYTES(CAM_WIDTH, CAM_HEIGHT), NULL, NULL);
    }
}

static void InitializeGraphics()
{
    // VRAM allocation
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);
    
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D);
    GX_SetVisiblePlane( GX_PLANEMASK_BG3 );
    GXS_SetGraphicsMode( GX_BGMODE_4 );
    GXS_SetVisiblePlane( GX_PLANEMASK_BG3 );
    
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);  // Set screen offset value
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x20000);  // Configure character base offset value
    
    G2_BlendNone();
    G2S_BlendNone();
    GX_Power2DSub(TRUE);    // Turn the sub 2D graphic engine off
    
    // Main-BG
    // BG3: scr 96KB
    {
        G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2_SetBG3Priority(2);
        G2_BG3Mosaic(FALSE);
    }
    
    // Sub-BG
    // BG3: scr 96KB
    {
        G2S_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
        G2S_SetBG3Priority(2);
        G2S_BG3Mosaic(FALSE);
    }
}

static void InitializeCamera(void)
{
    CAMERAResult result;
    
    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CActivate was failed. (%d)\n", result);

    // Camera VSYNC interrupt callback
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // Camera error interrupt callback
    CAMERA_SetBufferErrorCallback(CameraIntrError);
    
    // Camera restart completion callback
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_YUV);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(CAM_WIDTH));
}

/* Callback function called when YUV to RGB conversion is done */
static void ConvertCallbackFunc(void)
{
    OS_TPrintf("[Async]time: %d microsec.\n", OS_TicksToMicroSeconds(OS_GetTick() - StartTick));
    IsConvertNow = FALSE;
}

