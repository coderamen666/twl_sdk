/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - camera - camera-2
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

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

#define NDMA_NO  1           // The NDMA number (0 to 3) to use
#define NDMA_IE  OS_IE_NDMA1  // Corresponding interrupt
#define WIDTH   256         // Image width
#define HEIGHT  192         // Image height

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // Number of lines transferred in one cycle
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // Number of bytes in one line's transfer

static void VBlankIntr(void);
static BOOL CameraInit(void);
static void CameraDmaIntr(void);
static void CameraIntrVsync(CAMERAResult result);
static void CameraIntrError(CAMERAResult result);
static void CameraIntrReboot(CAMERAResult result);

static int lineNumber = 0;
static BOOL effectFlag = FALSE;
static BOOL switchFlag = FALSE;
static BOOL standbyFlag = FALSE;
static u16 pipeBuffer[BYTES_PER_LINE * LINES_AT_ONCE / sizeof(u16)] ATTRIBUTE_ALIGN(32);
static CAMERASelect current;


static int wp;  // Buffer while capturing data from camera
static int rp;  // Buffer most recently copied to VRAM
static BOOL wp_pending; // Data capture was cancelled (recapture to same buffer)
static u32 stabilizedCount;
static u16 buffer[2][WIDTH*HEIGHT] ATTRIBUTE_ALIGN(32);
static void PendingCapture(void)
{
    wp_pending = TRUE;
}

// Character display
static void PutString( char *format, ... )
{
    u16             *dest = G2_GetBG1ScrPtr();
    char            temp[32+1];
    int             i;
    va_list         va;

    va_start(va, format);
    (void)OS_VSNPrintf(temp, sizeof(temp), format, va);
    va_end(va);

    for (i = 0; i < 32 && temp[i]; i++)
    {
        dest[i] = (u16)((u8)temp[i] | (1 << 12));
    }
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain()
{
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
    (void)CameraInit();

    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // Configure DMA interrupt
    OS_SetIrqFunction(NDMA_IE, CameraDmaIntr);
    (void)OS_EnableIrqMask(NDMA_IE);

    // Camera VSYNC interrupt callback
    CAMERA_SetVsyncCallback(CameraIntrVsync);

    // Camera error interrupt callback
    CAMERA_SetBufferErrorCallback(CameraIntrError);

    // Camera restart completion callback
    CAMERA_SetRebootCallback(CameraIntrReboot);

    // DMA start (enabled forever unless explicitly stopped)
    MI_StopNDma(NDMA_NO);
    CAMERA_DmaPipeInfinity(NDMA_NO, pipeBuffer, CAMERA_GetBytesAtOnce(WIDTH), NULL, NULL);

    // Camera start
    lineNumber = 0;
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;

        OS_WaitVBlankIntr();

        if (wp == rp)
        {
            rp ^= 1;
            GX_LoadBG3Scr(buffer[rp], 0, BYTES_PER_LINE * HEIGHT);
            DC_InvalidateRange(buffer[rp], BYTES_PER_LINE * HEIGHT);
        }

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        if (trg & PAD_BUTTON_A)
        {
            effectFlag ^= 1;
            OS_TPrintf("Effect %s\n", effectFlag ? "ON" : "OFF");
        }
        if (trg & PAD_BUTTON_X)
        {
            current = (current == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
            switchFlag = TRUE;
        }
        if (trg & PAD_BUTTON_B)
        {
            standbyFlag ^= 1;

            if(standbyFlag)
            {
                switchFlag = TRUE;
            }
            else
            {
                CAMERAResult result;

                OS_TPrintf("call CAMERA_I2CActivate(%s)\n", (current == CAMERA_SELECT_IN ? "CAMERA_SELECT_IN" : "CAMERA_SELECT_OUT"));
                result = CAMERA_I2CActivateAsync(current, NULL, NULL);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
                stabilizedCount = 0;
            }
        }
    }
}

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
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

    result = CAMERA_I2CActivate(current);
    if (result == CAMERA_RESULT_FATAL_ERROR)
        OS_TPanic("CAMERA_I2CActivate was failed. (%d)\n", result);
    if(result == CAMERA_RESULT_ILLEGAL_STATUS)
        return FALSE;
    stabilizedCount = 0;

    return TRUE;
}

//--------------------------------------------------------------------------------
//    DMA interrupt process
//
#define FPS_SAMPLES 4
void CameraDmaIntr(void)
{
    static BOOL effect = FALSE;
    CAMERAResult result;

    OS_SetIrqCheckFlag(NDMA_IE);
    OS_SetIrqFunction(NDMA_IE, CameraDmaIntr);

    // Process as required and copy to frame buffer
    if (effect) // Invert negative/positive sign
    {
        int i;
        u32 *src = (u32*)pipeBuffer;
        u32 *dest = (u32*)buffer[wp] + lineNumber * WIDTH / 2;
        for (i = 0; i < WIDTH * LINES_AT_ONCE / 2; i++)
        {
            dest[i] = ~src[i] | 0x80008000;
        }
    }
    else
    {
        MI_CpuCopyFast(pipeBuffer, (u16*)buffer[wp] + lineNumber * WIDTH, sizeof(pipeBuffer));
    }
    DC_InvalidateRange(pipeBuffer, sizeof(pipeBuffer));

    lineNumber += LINES_AT_ONCE;
    if (lineNumber >= HEIGHT)
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

        if (switchFlag)
        {
            if (standbyFlag)
            {
                OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)\n");
                result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
            }
            else
            {
                OS_TPrintf("call CAMERA_I2CActivate(%s)\n", (current == CAMERA_SELECT_IN ? "CAMERA_SELECT_IN" : "CAMERA_SELECT_OUT"));
                result = CAMERA_I2CActivateAsync(current, NULL, NULL);
                if(result == CAMERA_RESULT_FATAL_ERROR)
                    OS_Panic("CAMERA FATAL ERROR\n");
                stabilizedCount = 0;
            }
            switchFlag = FALSE;
        }
        effect = effectFlag;

        // Switch buffers
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

        lineNumber = 0;
    }
}

void CameraIntrVsync(CAMERAResult result)
{
#pragma unused(result)
    if(stabilizedCount <= 30)
        stabilizedCount++;
}

//--------------------------------------------------------------------------------
//    Camera interrupt process (Generated when there is an error and for Vsync)
//
void CameraIntrError(CAMERAResult result)
{
#pragma unused(result)
    OS_TPrintf("Error was occurred.\n");

    // Stop the camera
    CAMERA_StopCapture();
    lineNumber = 0;
    wp_pending = TRUE;      // Also use same frame next time
    // Restart the camera
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
}

void CameraIntrReboot(CAMERAResult result)
{
    if(result == CAMERA_RESULT_FATAL_ERROR)
    {
        return; // Recovery was not possible, even after restarting camera
    }
    CameraIntrError(result); // DMA synchronization might have drifted, so realign
}
