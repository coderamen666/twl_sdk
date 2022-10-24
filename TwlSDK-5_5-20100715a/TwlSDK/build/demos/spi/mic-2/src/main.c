/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos - mic-2
  File:     main.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A sample that controls mic sampling status.

    USAGE:
        UP, DOWN   : Control sampling span
        LEFT, RIGHT: Control sampling bit range (8-bit or 12-bit)
        A: Start / stop sampling (toggle)
        B: Force-stop (Stop and ignore rest of data)

    HOWTO:
        1. Initialize memory allocation system to get 32-byte aligned buffer.
        2. Initialize MIC library.
        3. When sampling is stopped, you can change status
           and start auto sampling.
           Debug-output is sampling data for mic2wav.exe tool.
        4. Debug-output log can make its waveform files with mic2wav.exe tool.
           > $(NITROSDK_ROOT)/tools/bin/mic2wav [logfile] [,directory]
           Each sampling data (separated by A-button) creates a waveform file
           in '[directory]/%08X.wav'.

    NOTE:
        1. The speed of debug-output is slower than sampling.
           When you stop sampling, wait until all data is printed.

 *---------------------------------------------------------------------------*/

#include    <nitro.h>
#include    <nitro/spi/common/pm_common.h>
#include    <nitro/spi/ARM9/pm.h>


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats
#define     TEST_BUFFER_SIZE    ( 1024 * 1024 ) // 1M


/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/
// Key input data
typedef struct KeyInformation
{
    u16     cnt;                       // Unprocessed input value
    u16     trg;                       // Push trigger input
    u16     up;                        // Release trigger input
    u16     rep;                       // Press and hold repeat input

}
KeyInformation;

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void);
static void Init3D(void);
static void Draw3D(void);
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey);
static void VBlankIntr(void);
static void KeyRead(KeyInformation * pKey);
static void SetDrawData(void *address, MICSamplingType type);
static void PrintfVariableData(void);

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static KeyInformation gKey;
static MICAutoParam gMicAutoParam;
static u8 *gMicData;
static u8 gDrawData[192];


/* This differs from mic-1 in that there is a debug output of data and it is always "one-shot" recording 
   */

/* Variables for waveform output */
static volatile BOOL g_sample_busy = FALSE;
static const void *g_record_smps = NULL;

static void StartSampling(void);
static void OnSampleDone(MICResult result, void *arg);
static void OutputSampleWave(void *dat, MICSamplingType type);
static void StopSamplingOutput(void);

/* Begin sampling */
static void StartSampling(void)
{
    gMicAutoParam.buffer = (void *)gMicData;
    gMicAutoParam.size = TEST_BUFFER_SIZE;
    gMicAutoParam.loop_enable = FALSE;
    gMicAutoParam.full_callback = OnSampleDone;
    g_sample_busy = TRUE;
    g_record_smps = gMicData;
    PrintfVariableData();
    (void)MIC_StartAutoSampling(&gMicAutoParam);
}

/* End sampling output */
static void StopSamplingOutput(void)
{
    OS_PutString("$end\n");
    OS_PutString("\n");
    g_record_smps = NULL;
}

/* Sampling completion notification, or waveform output process at stop */
static void OnSampleDone(MICResult result, void *arg)
{
    (void)result;
    (void)arg;
    if (g_sample_busy)
    {
        g_sample_busy = FALSE;
    }
}

/* Waveform sampling log output*/
static void OutputSampleWave(void *dat, MICSamplingType type)
{
    /* Up to 2 lines at once with 16-sample units.
       If you output more than this amount, the log may contain omissions. */
    enum
    { smps_per_line = 16, max_line_per_frame = 2 };
    if (!g_record_smps || !dat)
        return;

    DC_InvalidateRange((void *)g_record_smps, (u32)((u8 *)dat - (u8 *)g_record_smps));
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        {
            typedef u8 SMP;
            /* The following is identical to MIC_SAMPLING_TYPE_12BIT */
            char    buf[1 + (sizeof(SMP) * 2 + 1) * smps_per_line + 1 + 1], *s;
            const SMP *from = (const SMP *)g_record_smps;
            const SMP *to = (const SMP *)dat;
            int     lines = 0;
            while ((lines < max_line_per_frame) && (from + smps_per_line <= to))
            {
                int     i, j;
                s = buf;
                *s++ = '|';
                for (i = 0; i < smps_per_line; ++i)
                {
                    u32     unit = from[i];
                    for (j = sizeof(SMP) * 8; (j -= 4) >= 0;)
                    {
                        u32     c = (u32)((unit >> j) & 0x0F);
                        c += (u32)((c < 10) ? ('0' - 0) : ('A' - 10));
                        MI_WriteByte(s++, (u8)c);
                    }
                    MI_WriteByte(s++, (u8)',');
                }
                MI_WriteByte(s++, (u8)'\n');
                MI_WriteByte(s++, (u8)'\0');
                OS_PutString(buf);
                from += smps_per_line;
                ++lines;
            }
            g_record_smps = from;

        }
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        {
            typedef u16 SMP;
            /* The following is identical to MIC_SAMPLING_TYPE_8BIT */
            char    buf[1 + (sizeof(SMP) * 2 + 1) * smps_per_line + 1 + 1], *s;
            const SMP *from = (const SMP *)g_record_smps;
            const SMP *to = (const SMP *)dat;
            int     lines = 0;
            while ((lines < max_line_per_frame) && (from + smps_per_line <= to))
            {
                int     i, j;
                s = buf;
                *s++ = '|';
                for (i = 0; i < smps_per_line; ++i)
                {
                    u32     unit = from[i];
                    for (j = sizeof(SMP) * 8; (j -= 4) >= 0;)
                    {
                        u32     c = (u32)((unit >> j) & 0x0F);
                        c += (u32)((c < 10) ? ('0' - 0) : ('A' - 10));
                        MI_WriteByte(s++, (u8)c);
                    }
                    MI_WriteByte(s++, (u8)',');
                }
                MI_WriteByte(s++, (u8)'\n');
                MI_WriteByte(s++, (u8)'\0');
                OS_PutString(buf);
                from += smps_per_line;
                ++lines;
            }
            g_record_smps = from;

        }
        break;
    }

    /* Also output remaining data after stopping sampling */
    if (!g_sample_busy && g_record_smps)
    {
        if ((u8 *)g_record_smps + smps_per_line * 2 >= (u8 *)dat)
        {
            StopSamplingOutput();
        }
    }

}


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Various types of initialization
    OS_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // Initializes display settings
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 3D-related initialization
    Init3D();

    // Interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    //****************************************************************
    // Initialize MIC.
    InitializeAllocateSystem();
    // Because the memory allocated with OS_Alloc is 32-byte aligned, other memory is not destroyed even if the cache is manipulated
    //  
    gMicData = (u8 *)OS_Alloc(TEST_BUFFER_SIZE);
    gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
    gMicAutoParam.rate = MIC_SAMPLING_RATE_11K;
    MIC_Init();

#ifdef  SDK_TS
    // Initialize PMIC
    PM_Init();
    // AMP on
    (void)PM_SetAmp(PM_AMP_ON);
#if defined(SDK_TS_VERSION) && ( SDK_TS_VERSION >= 100 )
    // Adjust AMP gain
    (void)PM_SetAmpGain(PM_AMPGAIN_80);
#endif
#if defined(SDK_TS_VERSION) && ( SDK_TS_VERSION == 100 )
    // Turn off LCD backlight to deal with noise
    (void)PM_SetBackLight(PM_LCD_ALL, PM_BACKLIGHT_OFF);
#endif
#endif

    //****************************************************************

    // Initialize internal variables
    {
        s32     i;
        for (i = 0; i < 192; i++)
        {
            gDrawData[i] = 0x80;
        }
    }

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Start message output
    OS_Printf("#ARM9: MIC demo started.\n");
    OS_Printf("#   up/down    -> change sampling span\n");
    OS_Printf("#   left/right -> change bit range\n");
    OS_Printf("#   A          -> start / stop\n");
    OS_Printf("#   B          -> force-stop\n");
    OS_Printf("#   select     -> terminate\n");

    // Empty call for getting key input data (strategy for pressing A button in the IPL)
    KeyRead(&gKey);

    // Main loop
    while (TRUE)
    {
        // Get key input data
        KeyRead(&gKey);

        // Start if A button is pressed; stop if A button is pressed again
        if ((gKey.trg & PAD_BUTTON_A) != 0)
        {
            if (!g_sample_busy)
            {
                StartSampling();
            }
            else
            {
                (void)MIC_StopAutoSampling();
                OnSampleDone(MIC_RESULT_SUCCESS, &gMicAutoParam);
            }
        }
        // If B button is pressed, stop and ignore remaining data
        if ((gKey.trg & PAD_BUTTON_B) != 0)
        {
            if (g_sample_busy)
            {
                (void)MIC_StopAutoSampling();
                OnSampleDone(MIC_RESULT_SUCCESS, &gMicAutoParam);
            }
            if (g_record_smps)
            {
                StopSamplingOutput();
            }
        }
        // Program ended when SELECT button is pressed
        if ((gKey.trg & PAD_BUTTON_SELECT) != 0)
        {
            OS_Exit(0);
        }

        // If sampling output is not in progress, change the variable parameters
        if (!g_record_smps)
        {
            // Change sampling type (bit width)
            if ((gKey.trg | gKey.rep) & (PAD_KEY_LEFT | PAD_KEY_RIGHT))
            {
                //****************************************************************
                if (gMicAutoParam.type == MIC_SAMPLING_TYPE_8BIT)
                {
                    gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
                }
                else
                {
                    gMicAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
                }
                //****************************************************************
                if (!g_record_smps)
                    PrintfVariableData();
            }
            // Change sampling rate
            if ((gKey.trg | gKey.rep) & PAD_KEY_UP)
            {
                //****************************************************************
                switch (gMicAutoParam.rate)
                {
                case MIC_SAMPLING_RATE_8K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_11K;
                    break;
                case MIC_SAMPLING_RATE_11K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_16K;
                    break;
                case MIC_SAMPLING_RATE_16K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_22K;
                    break;
                case MIC_SAMPLING_RATE_22K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_32K;
                    break;
                case MIC_SAMPLING_RATE_32K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
                    break;
                }
                //****************************************************************
                if (!g_record_smps)
                    PrintfVariableData();
            }
            if ((gKey.trg | gKey.rep) & PAD_KEY_DOWN)
            {
                //****************************************************************
                switch (gMicAutoParam.rate)
                {
                case MIC_SAMPLING_RATE_8K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_32K;
                    break;
                case MIC_SAMPLING_RATE_11K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
                    break;
                case MIC_SAMPLING_RATE_16K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_11K;
                    break;
                case MIC_SAMPLING_RATE_22K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_16K;
                    break;
                case MIC_SAMPLING_RATE_32K:
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_22K;
                    break;
                }
                //****************************************************************
                if (!g_record_smps)
                    PrintfVariableData();
            }
        }

        // Log output of waveform
        OutputSampleWave(MIC_GetLastSamplingAddress(), gMicAutoParam.type);

        // Render waveform
        if (g_sample_busy)
        {
            SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
            Draw3D();
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    // Based on the premise that OS_Init has been already called
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         Init3D

  Description:  Initialization for 3D display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Init3D(void)
{
    G3X_Init();
    G3X_InitMtxStack();
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    G3X_SetShading(GX_SHADING_TOON);
    G3X_AlphaTest(FALSE, 0);
    G3X_AlphaBlend(TRUE);
    G3X_AntiAlias(TRUE);
    G3X_EdgeMarking(FALSE);
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);
    G3X_SetClearColor(0, 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);
}

/*---------------------------------------------------------------------------*
  Name:         Draw3D

  Description:  Displays waveforms in 3D.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Draw3D(void)
{
    G3X_Reset();

    G3_MtxMode(GX_MTXMODE_PROJECTION);
    G3_Identity();
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
    G3_Identity();

    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    if (g_sample_busy)
    {
        s32     i;

        for (i = 0; i < 191; i++)
        {
            DrawLine((s16)(gDrawData[i]), (s16)i, (s16)(gDrawData[i + 1]), (s16)(i + 1));
        }
    }

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         DrawLine

  Description:  Renders lines with triangular polygons.

  Arguments:    sx: X-coordinate of line's starting point
                sy: Y-coordinate of line's starting point
                ex: X-coordinate of line's ending point
                ey: Y-coordinate of line's ending point

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey)
{
    fx16    fsx, fsy, fex, fey;

    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fey = (fx16)(((96 - ey) * 0x1000) / 96);

    G3_Begin(GX_BEGIN_TRIANGLES);
    {
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fsy, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fex, fey, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fsy, 1);
    }
    G3_End();
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data.
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    pKey: Structure that holds key input data to be edited

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInformation * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // Press trigger
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // Press-and-hold repeat
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // Release trigger
            }
        }
    }
    pKey->cnt = r;                     // Unprocessed key input
}

/*---------------------------------------------------------------------------*
  Name:         SetDrawData

  Description:  Stores the current newest sampled data in the buffer that puts it on the display.
                

  Arguments:    address: Location in main memory where the most recent sampling data is stored by the components
                          
                type: Sampling type (bit width)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetDrawData(void *address, MICSamplingType type)
{
    s32     i;

    if (!address)
        return;

    if (type == MIC_SAMPLING_TYPE_8BIT)
    {
        u8     *p;

        p = (u8 *)((u32)address - 191);
        if (p < gMicData)
        {
            p = (u8 *)((u32)p + TEST_BUFFER_SIZE);
        }
        DC_InvalidateRange((void *)((u32)p & 0xffffffe0), 32);
        for (i = 0; i < 192; i++)
        {
            gDrawData[i] = *p;
            p++;
            if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
            {
                p -= TEST_BUFFER_SIZE;
            }
            if (((u32)p % 32) == 0)
            {
                DC_InvalidateRange(p, 32);
            }
        }
    }

    if (type == MIC_SAMPLING_TYPE_12BIT)
    {
        u16    *p;

        p = (u16 *)((u32)address - 382);
        if ((u32)p < (u32)gMicData)
        {
            p = (u16 *)((u32)p + TEST_BUFFER_SIZE);
        }
        DC_InvalidateRange((void *)((u32)p & 0xffffffe0), 32);
        for (i = 0; i < 192; i++)
        {
            gDrawData[i] = (u8)((*p >> 8) & 0x00ff);
            p++;
            if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
            {
                p = (u16 *)((u32)p - TEST_BUFFER_SIZE);
            }
            if (((u32)p % 32) == 0)
            {
                DC_InvalidateRange(p, 32);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintfVariableData

  Description:  Print-output the variable sampling settings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintfVariableData(void)
{
    s32     range = 0;
    s32     rate = 0;

    switch (gMicAutoParam.type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        range = 8;
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        range = 16;
        break;
    }

    switch (gMicAutoParam.rate)
    {
    case MIC_SAMPLING_RATE_8K:
        rate = 8000;
        break;
    case MIC_SAMPLING_RATE_11K:
        rate = 11025;
        break;
    case MIC_SAMPLING_RATE_16K:
        rate = 16000;
        break;
    case MIC_SAMPLING_RATE_22K:
        rate = 22050;
        break;
    case MIC_SAMPLING_RATE_32K:
        rate = 32000;
        break;
    }

    OS_Printf("$rate=%d\n$bits=%d\n", rate, range);
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
