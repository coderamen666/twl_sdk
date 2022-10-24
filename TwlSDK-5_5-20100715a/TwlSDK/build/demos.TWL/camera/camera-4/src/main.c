/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - camera - camera-4
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/camera.h>
#include "DEMO.h"

#define NDMA_NO      1           // The NDMA number (0 to 3) to use
#define WIDTH       256         // Image width
#define HEIGHT      192         // Image height

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // Number of lines transferred in one cycle
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // Number of bytes in one line's transfer

static void VBlankIntr(void);
static BOOL CameraInit(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);
static void CameraDmaRecvIntr(void* arg);

/////
//  Sample configuration with double buffer in main memory and 1 layer of VRAM
/////

static int wp;  // Buffer while capturing data from camera
static int rp;  // Buffer most recently copied to VRAM
static BOOL wp_pending; // Data capture was cancelled (recapture to same buffer)
static u32 stabilizedCount;
static BOOL switchFlag;

static CAMERASelect current;
static CAMERAFlip   flipIn, flipOut;
static u16 buffer[2][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
static void DebugReport(void)
{
    OS_TPrintf("\nCapture to No.%d\tDisplay from No.%d\n", wp, rp);
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// Display text (one row only)
static u16 text_buffer[ 32 ] ATTRIBUTE_ALIGN(32);
static void PutString( char *format, ... )
{
    u16             *dest = text_buffer;
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    MI_CpuClearFast(text_buffer, sizeof(text_buffer));
    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (1 << 12));
    }
    DC_StoreRange(text_buffer, sizeof(text_buffer));
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    CAMERAResult result;

    // Initialization
    OS_Init();
    OS_InitThread();
    GX_Init();
    OS_InitTick();
    OS_InitAlarm();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    // DMA is not used in GX (the old DMA conflicts with camera DMA)
    (void)GX_SetDefaultDMA(GX_DMA_NOT_USE);

    // Clears VRAM
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    GX_SetBankForLCDC(GX_VRAM_LCDC_B);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_A, 128 * 1024);
    MI_CpuClearFast((void*)HW_LCDC_VRAM_B, 128 * 1024);

    // Direct bitmap display mode and text display
    GX_SetBankForBG(GX_VRAM_BG_256_AB);         // Allocate VRAM-A and B banks to BG
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3);

    G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
    G2_SetBG1Priority(1);
    G2_BG1Mosaic(FALSE);

    G2_SetBG3ControlDCBmp(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x20000);
    G2_SetBG3Priority(3);
    G2_BG3Mosaic(FALSE);

    // Load text
    {
        static const GXRgb pal[16] = { GX_RGB(0, 0, 0), GX_RGB(31, 31, 31), };
        GX_LoadBG1Char(DEMOAsciiChr, 0x00000, sizeof(DEMOAsciiChr));
        GX_LoadBGPltt(pal, 0x0000, sizeof(pal));
    }
    wp = 0;
    rp = 1;
    wp_pending = TRUE;
    stabilizedCount = 0;
    switchFlag = FALSE;

    // V-Blank interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    OS_WaitVBlankIntr();
    GX_DispOn();

    // Initialize camera
    current = CAMERA_SELECT_IN;
    flipIn = CAMERA_FLIP_HORIZONTAL;
    flipOut = CAMERA_FLIP_NONE;
    (void)CameraInit();

    // Configure DMA interrupt
    (void)OS_EnableIrqMask(OS_IE_NDMA1);

    // Camera VSYNC interrupt callback
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // Camera error interrupt callback
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // Camera restart completion callback
    CAMERA_SetRebootCallback(CameraIntrReboot);

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // Start capturing
    CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;
        static BOOL standby = FALSE;

        OS_WaitVBlankIntr();

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        if (trg & PAD_BUTTON_Y)
        {
            DebugReport();
        }
        if (trg & PAD_BUTTON_X)
        {
            switchFlag = TRUE;
        }
        if (trg & PAD_BUTTON_B)
        {
            standby ^= 1;
            if (standby)
            {
                // If you set Activate to NONE before StopCapture, there is a risk that IsBusy will be TRUE until the next time you set "Activate ON"
                CAMERA_StopCapture();
                while(CAMERA_IsBusy())
                {
                }
                MI_StopNDma(NDMA_NO);
                result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
            }
            else
            {
                result = CAMERA_I2CActivate(current);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
                stabilizedCount = 0;
                CAMERA_DmaRecvAsync(NDMA_NO, buffer[wp], CAMERA_GetBytesAtOnce(WIDTH), CAMERA_GET_FRAME_BYTES(WIDTH, HEIGHT), CameraDmaRecvIntr, NULL);
                CAMERA_ClearBuffer();
                CAMERA_StartCapture();
            }
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
        }

        if (trg & PAD_BUTTON_SELECT)
        {
            result = CAMERA_I2CFlip(current, CAMERA_FLIP_VERTICAL);
            if (result != CAMERA_RESULT_SUCCESS)
            {
                OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
            }
            *(current == CAMERA_SELECT_IN ? &flipIn : &flipOut) = CAMERA_FLIP_VERTICAL;
        }
        if (trg & PAD_BUTTON_L)
        {
            result = CAMERA_I2CFlip(current, CAMERA_FLIP_HORIZONTAL);
            if (result != CAMERA_RESULT_SUCCESS)
            {
                OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
            }
            *(current == CAMERA_SELECT_IN ? &flipIn : &flipOut) = CAMERA_FLIP_HORIZONTAL;
        }
        if (trg & PAD_BUTTON_R)
        {
            result = CAMERA_I2CFlip(current, CAMERA_FLIP_REVERSE);
            if (result != CAMERA_RESULT_SUCCESS)
            {
                OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
            }
            *(current == CAMERA_SELECT_IN ? &flipIn : &flipOut) = CAMERA_FLIP_REVERSE;
        }
        if (trg & PAD_BUTTON_START)
        {
            result = CAMERA_I2CFlip(current, CAMERA_FLIP_NONE);
            if (result != CAMERA_RESULT_SUCCESS)
            {
                OS_TPrintf("CAMERA_I2CEffect was failed. (%d)\n", result);
            }
            *(current == CAMERA_SELECT_IN ? &flipIn : &flipOut) = CAMERA_FLIP_NONE;
        }
    }
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
    if (wp == rp)
    {
        rp ^= 1;
        // This is just a simple copy, but format conversion and so on could be included
        GX_LoadBG3Scr(buffer[rp], 0, BYTES_PER_LINE * HEIGHT);
        DC_InvalidateRange(buffer[rp], BYTES_PER_LINE * HEIGHT);
    }
    GX_LoadBG1Scr(text_buffer, 0, sizeof(text_buffer));
}

//--------------------------------------------------------------------------------
//    Camera initialization
//
BOOL CameraInit(void)
{
    CAMERAResult result;

    result = CAMERA_Init();
    if(result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_Init was failed.");
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;

    result = CAMERA_I2CFlip(CAMERA_SELECT_IN, flipIn);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CFlip was failed. (%d)\n", result);
    }
    result = CAMERA_I2CFlip(CAMERA_SELECT_OUT, flipOut);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CFlip was failed. (%d)\n", result);
    }
    result = CAMERA_I2CActivate(current);
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
#define FPS_SAMPLES 4
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
        current = (current == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
        OS_TPrintf("call CAMERA_I2CActivate(%s)... ", (current == CAMERA_SELECT_IN ? "CAMERA_SELECT_IN" : "CAMERA_SELECT_OUT"));
        result = CAMERA_I2CActivate(current);
        if(result == CAMERA_RESULT_FATAL_ERROR)
            OS_Panic("CAMERA FATAL ERROR\n");
        stabilizedCount = 0;
        switchFlag = FALSE;
        OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
    }
}

void CameraDmaRecvIntr(void* arg)
{
#pragma unused(arg)
    MI_StopNDma(NDMA_NO);

    if (CAMERA_IsBusy() == TRUE)
    {
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

    // Display frame rate
    {
        static OSTick begin = 0;
        static int uspf[FPS_SAMPLES] = { 0 };
        static int count = 0;
        int i;
        int sum = 0;
        OSTick end = OS_GetTick();
        if (begin)  // Leave out the first time
        {
            uspf[count] = (int)OS_TicksToMicroSeconds(end - begin);
            count = (count + 1) % FPS_SAMPLES;
        }
        begin = end;
        // Calculate average value
        for (i = 0; i < FPS_SAMPLES; i++)
        {
            if (uspf[i] == 0)  break;
            sum +=  uspf[i];
        }
        if (sum)
        {
            int mfps = (int)(1000000000LL * i / sum);
            PutString("%2d.%03d fps", mfps / 1000, mfps % 1000);
        }
    }
}
