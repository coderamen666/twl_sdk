/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - demos
  File:     main.c

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

#include <nitro.h>
#include <nitro/fx/fx_trig.h>

#include <nitro/spi/common/pm_common.h>
#include <nitro/spi/ARM9/pm.h>


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/

#define USE_FFTREAL

enum {
    DRAWMODE_REALTIME,
    DRAWMODE_BAR,
    DRAWMODE_SCALE,
    DRAWMODE_MAX
};

#define KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define KEY_REPEAT_SPAN     10     // Number of frames between key repeats
#define SAMPLING_BUFFER_SIZE ( 1024 * 1024 ) // 1M

#define FFT_NSHIFT         11
#define FFT_N              (1 << FFT_NSHIFT)      // Number of samples on which an FFT will be applied
#define DRAW_STEP          MATH_MAX(FFT_N/2/256,1)    // Number of steps to render
#define DRAW_MAX           MATH_MIN(FFT_N/2/DRAW_STEP, 256) // Independent power values other than the DC component, and the minimum screen width

#define SCALE_SAMPLING_OCTAVES 5

// Sound output related
#define CHANNEL_NUM 4
#define ALARM_NUM 0
#define STREAM_THREAD_PRIO 12
#define THREAD_STACK_SIZE 1024
#define STRM_BUF_PAGESIZE 64*32
#define STRM_BUF_SIZE STRM_BUF_PAGESIZE*2
#define STRM_SAMPLE_RATE 44100
#define OSC_MAX_VOLUME 32767

// Derive the frequency from the key number
#define GetFreq(pitch) (SND_TIMER_CLOCK / SND_CalcTimer((SND_TIMER_CLOCK / 440), (pitch - 69 * 64)))

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

// Sound output related
// Stream object
typedef struct StreamInfo
{
    u32     bufPage;
}
StreamInfo;

// Touch panel
typedef struct TpInformation
{
    int     touch:1;
    int     trg:1;
    int     rls:1;
    u16     x;
    u16     y;
}
TpInformation;

// Oscillator
typedef struct Oscillator
{
    fx16    index;
    fx16    step;
    fx32    rate;                      // Output sampling rate
    u16     gain;
    u16     dummy;
}
Oscillator;

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void);
static void Init3D(void);
static void Draw3D_Realtime(void);
static void Draw3D_Bar(void);
static void Draw3D_Scale(void);
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey);
static void DrawBar(s16 sx, s16 sy, s16 ex, s16 ey);
static void DrawBarWithColor(s16 sx, s16 sy, s16 ex, s16 ey, u32 c);
static void VBlankIntr(void);
static void KeyRead(KeyInformation * pKey);
static void SetDrawData(void *address);
static void PrintfVariableData(void);

// Sound output related
static void SoundAlarmHandler(void *arg);
static void StrmThread(void *arg);
static void Play(StreamInfo * strm);
static void Stop();
static void MakeStreamData(StreamInfo * strm);
static void TpRead(TpInformation* tp);

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static MICAutoParam gMicAutoParam;
static u8 *gMicData;
static u8 gDrawData[DRAW_MAX];

// FFT buffers
static fx16 sinTable[FFT_N - FFT_N / 4];
#ifdef USE_FFTREAL
static fx16 sinTable2[(FFT_N - FFT_N / 4) / 2];
static fx32 data[FFT_N];
#else
static fx32 data[FFT_N * 2];
#endif
static s32 power[FFT_N/2+1];
static s32 smoothedPower[FFT_N/2+1];

static BOOL drawMode;
static u8 blockCount;

// Sound output related
static u64 strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread strmThread;
static OSMessageQueue msgQ;
static OSMessage msgBuf[1];

static u8 strmBuf[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);

static Oscillator osc;
static TPCalibrateParam calibrate;

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    KeyInformation key;
    TpInformation tp;
    StreamInfo strm;

    // Various types of initialization
    OS_Init();
    OS_InitTick();
    OS_InitThread();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();
    SND_Init();
    TP_Init();

    // initializes display settings
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 3D related initialization
    Init3D();

    // Initializes touch panel read
    (void)TP_GetUserInfo(&calibrate);
    TP_SetCalibrateParam(&calibrate);

    // Oscillator settings
    osc.rate = STRM_SAMPLE_RATE << FX32_SHIFT;

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
    gMicData = (u8 *)OS_Alloc(SAMPLING_BUFFER_SIZE);
    SDK_NULL_ASSERT(gMicData);
    MIC_Init();

    // Initialize PMIC
    PM_Init();
    // AMP on
    (void)PM_SetAmp(PM_AMP_ON);
    // Adjust AMP gain
    (void)PM_SetAmpGain(PM_AMPGAIN_80);

    // Perform 12-bit sampling to increase the accuracy of Fourier transforms
    gMicAutoParam.type = MIC_SAMPLING_TYPE_12BIT;
    gMicAutoParam.buffer = (void *)gMicData;
    gMicAutoParam.size = SAMPLING_BUFFER_SIZE;
    gMicAutoParam.loop_enable = TRUE;
    gMicAutoParam.full_callback = NULL;
#ifdef SDK_TWL
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8180;
    (void)MIC_StartLimitedSampling(&gMicAutoParam);
#else   // ifdef SDK_TWL
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
    (void)MIC_StartAutoSampling(&gMicAutoParam);
#endif  // ifdef SDK_TWL else

    //****************************************************************

    // Initialize internal variables
    MATH_MakeFFTSinTable(sinTable, FFT_NSHIFT);
#ifdef USE_FFTREAL
    MATH_MakeFFTSinTable(sinTable2, FFT_NSHIFT - 1);
#endif
    MI_CpuClear8(gDrawData, sizeof(gDrawData));

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_Printf("ARM9: FFT spectrum demo started.\n");
    OS_Printf("   up/down    -> change bar width (Bar Mode)\n");
    OS_Printf("   left/right -> change draw mode\n");
    OS_Printf("\n");
    PrintfVariableData();

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    KeyRead(&key);

    // Lock the channel
    SND_LockChannel(1 << CHANNEL_NUM, 0);

    /* Startup stream thread */
    OS_CreateThread(&strmThread,
                    StrmThread,
                    NULL,
                    strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&strmThread);

    // The initial display mode is bar display
    drawMode = DRAWMODE_BAR;
    blockCount = 4;

    // Main loop
    while (TRUE)
    {
        // Receive ARM7 command reply
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        // Get key input data
        KeyRead(&key);

        // Read the touch panel
        TpRead(&tp);

        // Change variable parameters
        {

            // Change display mode
            if ((key.trg | key.rep) & (PAD_KEY_LEFT | PAD_KEY_RIGHT))
            {
                if ((key.trg | key.rep) & PAD_KEY_RIGHT)
                {
                    drawMode++;
                }
                if ((key.trg | key.rep) & PAD_KEY_LEFT)
                {
                    drawMode--;
                }

                if (drawMode < 0)
                {
                    drawMode += DRAWMODE_MAX;
                }
                if (drawMode >= DRAWMODE_MAX)
                {
                    drawMode -= DRAWMODE_MAX;
                }

                // Initialize accumulated data when switching display modes
                MI_CpuClear32(smoothedPower, sizeof(smoothedPower));
            }

            // Update block count
            if ((key.trg | key.rep) & PAD_KEY_UP)
            {
                blockCount += 1;
                if (blockCount >= 16)
                {
                    blockCount = 16;
                }
                OS_Printf("block = %d\n", blockCount);
            }
            if ((key.trg | key.rep) & PAD_KEY_DOWN)
            {
                blockCount -= 1;
                if (blockCount <= 2)
                {
                    blockCount = 2;
                }
                OS_TPrintf("block = %d\n", blockCount);
            }
        }

        // Calculate waveform data
        SetDrawData(MIC_GetLastSamplingAddress());

        // Render waveform
        // Switch for each drawMode
        switch (drawMode)
        {
        case DRAWMODE_REALTIME:
            Draw3D_Realtime();
            break;

        case DRAWMODE_BAR:
            Draw3D_Bar();
            break;

        case DRAWMODE_SCALE:
            Draw3D_Scale();
            break;
        }

        // Output the waveform that corresponds to the position touched on the Touch Panel
        if (tp.touch)
        {
            osc.step = (fx16)FX_Div(GetFreq(tp.x * 12 + 60 * 64) << FX32_SHIFT, osc.rate);
            osc.gain = (u16)(tp.y * OSC_MAX_VOLUME / 192);
        }

        // Plays the PSG square wave
        if (tp.trg)
        {
            Play(&strm);
        }

        if (tp.rls)
        {
            Stop();
        }

        // Command flush
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

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

  Description:  Initialization for 3D display

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void Init3D(void)
{
    G3X_Init();
    G3X_InitMtxStack();
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    G3X_AlphaTest(FALSE, 0);
    G3X_AntiAlias(TRUE);
    G3X_EdgeMarking(FALSE);
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);
    G3X_SetClearColor(0, 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
}

/*---------------------------------------------------------------------------*
  Name:         Draw3D_Realtime

  Description:  Displays 3D waveforms in real-time

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void Draw3D_Realtime(void)
{
    G3X_Reset();
    G3_Identity();
    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    {
        s32     i;

        for (i = 0; i < DRAW_MAX-1; i++)
        {
            DrawLine((s16)i, (s16)(192 - gDrawData[i]),
                     (s16)(i + 1), (s16)(192 - gDrawData[i + 1]));
        }
    }

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         Draw3D_Bar

  Description:  Bar display for waveforms in 3D

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void Draw3D_Bar(void)
{
    G3X_Reset();
    G3_Identity();
    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    {
        s32     i;

        for (i = 0; i < DRAW_MAX-1; i += blockCount)
        {
            DrawBar((s16)i, (s16)(192 - gDrawData[i]), (s16)(i + blockCount), (s16)192);
        }
    }

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         Draw3D_Scale

  Description:  Displays waveforms in 3D by musical scale

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void Draw3D_Scale(void)
{
    G3X_Reset();
    G3_Identity();
    G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_MODULATE, GX_CULL_NONE, 0, 31, 0);

    {
        s32     i;

        for (i = 0; i < 12; i++)
        {
            s32     j;
            for (j = 0; j < SCALE_SAMPLING_OCTAVES; j++)
            {
                DrawBarWithColor((s16)((i * (SCALE_SAMPLING_OCTAVES + 2) + j) * 3 + 2),
                                 (s16)(192 - gDrawData[i * SCALE_SAMPLING_OCTAVES + j]),
                                 (s16)((i * (SCALE_SAMPLING_OCTAVES + 2) + j) * 3 + 5),
                                 (s16)192, (u32)(j + 1));
            }
        }
    }

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

const GXRgb ColorTable[8] =
{
    GX_RGB( 0,  0,  0),
    GX_RGB(20, 20, 31),
    GX_RGB(20, 31, 20),
    GX_RGB(28, 31, 20),
    GX_RGB(31, 25, 20),
    GX_RGB(31, 20, 20),
    GX_RGB(31, 24, 24),
    GX_RGB(31, 31, 31),
};

/*---------------------------------------------------------------------------*
  Name:         DrawLine

  Description:  Renders lines with triangular polygons

  Arguments:    sx:   x coordinate of line's starting point
                sy:   y coordinate of line's starting point
                ex:   x coordinate of line's ending point
                ey:   y coordinate of line's ending point

  Returns:      None
 *---------------------------------------------------------------------------*/
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);

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
  Name:         DrawBar

  Description:  Render a bar with square (rectangular) polygons.

  Arguments:    sx:   Upper-left x coordinate of square to render
                sy:   Upper-left y coordinate of square to render
                ex:   Lower-right x coordinate of square to render
                ey:   Lower-right y coordinate of square to render

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawBar(s16 sx, s16 sy, s16 ex, s16 ey)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);

    G3_Begin(GX_BEGIN_QUADS);
    {
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fsy, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fsx, fey, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fex, fey, 0);
        G3_Color(GX_RGB(31, 31, 31));
        G3_Vtx(fex, fsy, 0);
    }
    G3_End();
}

/*---------------------------------------------------------------------------*
  Name:         DrawBarWithColor

  Description:  Render a bar with square (rectangular) polygons.

  Arguments:    sx:   Upper-left x coordinate of square to render
                sy:   Upper-left y coordinate of square to render
                ex:   Lower-right x coordinate of square to render
                ey:   Lower-right y coordinate of square to render
                c:    Color to render in

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawBarWithColor(s16 sx, s16 sy, s16 ex, s16 ey, u32 c)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);
    GXRgb color;

    if ( c >= sizeof(ColorTable)/sizeof(ColorTable[0]) )
    {
        c = 0;
    }
    color = ColorTable[c];

    G3_Begin(GX_BEGIN_QUADS);
    {
        G3_Color(color);
        G3_Vtx(fsx, fsy, 0);
        G3_Color(color);
        G3_Vtx(fsx, fey, 0);
        G3_Color(color);
        G3_Vtx(fex, fey, 0);
        G3_Color(color);
        G3_Vtx(fex, fsy, 0);
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

  Arguments:    pKey:   Structure that holds key input data to be edited

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

// Index in array of Fourier transforms, each corresponding to a musical scale
// Reference Information:
//   According to sampling theory, the maximum frequency for which coefficients can be obtained by a Fourier transform is half of the sampling frequency (4000 Hz).
//   
//   Also, there can be FFT_N/2 independent power values obtained by the Fourier transform.
//   The spectral frequency resolution that can be obtained as the result of the Fourier transform is 4000Hz / (FFT_N/2).
//   
//   (1) To increase the frequency resolution:
//     Increase FFT_N, which will increase the processing time and decrease the time resolution.
//     Decrease the sampling frequency (cull input data), which will lower the frequency band
//   (2) To raise the frequency band:
//     Increase the sampling frequency, which will decrease the frequency resolution.
static u32 scaleSamplingPoints[12][SCALE_SAMPLING_OCTAVES][2] =
{
    // FFT_N = 2048
    { // C
        { 33 , 34  }, /* (130.813 Hz : 33.4881) */
        { 65 , 69  }, /* (261.626 Hz : 66.9761) */
        { 130, 138 }, /* (523.251 Hz : 133.952) */
        { 260, 276 }, /* (1046.5 Hz : 267.905) */
        { 521, 552 }, /* (2093 Hz : 535.809) */
    },
    { // C#
        { 34 , 37  }, /* (138.591 Hz : 35.4794) */
        { 69 , 73  }, /* (277.183 Hz : 70.9588) */
        { 138, 146 }, /* (554.365 Hz : 141.918) */
        { 276, 292 }, /* (1108.73 Hz : 283.835) */
        { 552, 584 }, /* (2217.46 Hz : 567.67 ) */
    },
    { // D
        { 37 , 39  }, /* (146.832 Hz : 37.5891) */
        { 73 , 77  }, /* (293.665 Hz : 75.1782) */
        { 146, 155 }, /* (587.33 Hz : 150.356) */
        { 292, 310 }, /* (1174.66 Hz : 300.713) */
        { 584, 619 }, /* (2349.32 Hz : 601.425) */
    },
    { // D#
        { 39 , 41  }, /* (155.563 Hz : 39.8243) */
        { 77 , 82  }, /* (311.127 Hz : 79.6485) */
        { 155, 164 }, /* (622.254 Hz : 159.297) */
        { 310, 328 }, /* (1244.51 Hz : 318.594) */
        { 619, 656 }, /* (2489.02 Hz : 637.188) */
    },
    { // E
        { 41 , 43  }, /* (164.814 Hz : 42.1923) */
        { 82 , 87  }, /* (329.628 Hz : 84.3847) */
        { 164, 174 }, /* (659.255 Hz : 168.769) */
        { 328, 347 }, /* (1318.51 Hz : 337.539) */
        { 656, 695 }, /* (2637.02 Hz : 675.077) */
    },
    { // F
        { 43 , 46  }, /* (174.614 Hz : 44.7012) */
        { 87 , 92  }, /* (349.228 Hz : 89.4024) */
        { 174, 184 }, /* (698.456 Hz : 178.805) */
        { 347, 368 }, /* (1396.91 Hz : 357.61 ) */
        { 695, 736 }, /* (2793.83 Hz : 715.219) */
    },
    { // F#
        { 46 , 49  }, /* (184.997 Hz : 47.3593) */
        { 92 , 97  }, /* (369.994 Hz : 94.7186) */
        { 184, 195 }, /* (739.989 Hz : 189.437) */
        { 368, 390 }, /* (1479.98 Hz : 378.874) */
        { 736, 780 }, /* (2959.96 Hz : 757.749) */
    },
    { // G
        { 49 , 52  }, /* (195.998 Hz : 50.1754) */
        { 97 , 103 }, /* (391.995 Hz : 100.351) */
        { 195, 207 }, /* (783.991 Hz : 200.702) */
        { 390, 413 }, /* (1567.98 Hz : 401.403) */
        { 780, 826 }, /* (3135.96 Hz : 802.807) */
    },
    { // G#
        { 52 , 55  }, /* (207.652 Hz : 53.159 ) */
        { 103, 109 }, /* (415.305 Hz : 106.318) */
        { 207, 219 }, /* (830.609 Hz : 212.636) */
        { 413, 438 }, /* (1661.22 Hz : 425.272) */
        { 826, 875 }, /* (3322.44 Hz : 850.544) */
    },
    { // A
        { 55 , 58  }, /* (220 Hz : 56.32  ) */
        { 109, 116 }, /* (440 Hz : 112.64 ) */
        { 219, 232 }, /* (880 Hz : 225.28 ) */
        { 438, 464 }, /* (1760 Hz : 450.56 ) */
        { 875, 928 }, /* (3520 Hz : 901.12 ) */
    },
    { // A#
        { 58 , 61  }, /* (233.082 Hz : 59.669 ) */
        { 116, 123 }, /* (466.164 Hz : 119.338) */
        { 232, 246 }, /* (932.328 Hz : 238.676) */
        { 464, 491 }, /* (1864.66 Hz : 477.352) */
        { 928, 983 }, /* (3729.31 Hz : 954.703) */
    },
    { // B
        { 61 , 65  }, /* (246.942 Hz : 63.2171) */
        { 123, 130 }, /* (493.883 Hz : 126.434) */
        { 246, 260 }, /* (987.767 Hz : 252.868) */
        { 491, 521 }, /* (1975.53 Hz : 505.737) */
        { 983, 1023 }, /* (3951.07 Hz : 1011.47) */
    },
};

/*---------------------------------------------------------------------------*
  Name:         SetDrawData

  Description:  Stores the current newest sampled data in the buffer that puts it on the display.
                

  Arguments:    address: Location in main memory where the most recent sampling data was stored by the component
                          
                          
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetDrawData(void *address)
{
    s32     i;
    u16    *p;

    // If sampling has never been performed, do nothing and stop.
    // (Because it would delete memory cache(s) unrelated to the microphone)
    if ((address < gMicData) || (address >= (gMicData + SAMPLING_BUFFER_SIZE)))
    {
        return;
    }

    // Apply a FFT for the FFT_N most recent sampling values
    // With 12-bit sampling, each value is two bytes
    p = (u16 *)((u32)address - (FFT_N-1)*2);
    if ((u32)p < (u32)gMicData)
    {
        p = (u16 *)((u32)p + SAMPLING_BUFFER_SIZE);
    }
    DC_InvalidateRange((void *)((u32)p & 0xffffffe0), 32);
    for (i = 0; i < FFT_N; i++)
    {
#ifdef USE_FFTREAL
        data[i] = ((*p) << (FX32_SHIFT - (16 - 12)));
#else
        data[i * 2] = ((*p) << (FX32_SHIFT - (16 - 12)));
        data[i * 2 + 1] = 0; // Substitute 0 in the imaginary part
#endif
        p++;
        if ((u32)p >= (u32)(gMicData + SAMPLING_BUFFER_SIZE))
        {
            p = (u16 *)((u32)p - SAMPLING_BUFFER_SIZE);
        }
        if (((u32)p % 32) == 0)
        {
            DC_InvalidateRange(p, 32);
        }
    }

#ifdef USE_FFTREAL
    MATH_FFTReal(data, FFT_NSHIFT, sinTable, sinTable2);
#else
    MATH_FFT(data, FFT_NSHIFT, sinTable);
#endif

    // Do not calculate above FFT_N/2 because only conjugated, inverted values of those below FFT_N/2 can be obtained
    for (i = 0; i <= FFT_N/2; i++)
    {
        fx32 real;
        fx32 imm;

#ifdef USE_FFTREAL
        if (0 < i  && i < FFT_N/2)
        {
            real = data[i * 2];
            imm  = data[i * 2 + 1];
        }
        else
        {
            // Process the results of the MATH_FFTReal function because they are stored specially
            if (i == 0)
            {
                real = data[0];
            }
            else // i == FFT_N/2
            {
                real = data[1];
            }
            imm  = 0;
        }
#else
        real = data[i * 2];
        imm  = data[i * 2 + 1];
#endif

        // Calculate the power of each frequency
        // If only the relative value size is necessary, omitting the sqrt can result in a speed increase 
        power[i] = FX_Whole(FX_Sqrt(FX_MUL(real, real) + FX_MUL(imm, imm)));

        // Record the accumulated time values for the power of each frequency
        // Perform damping a little at a time
        smoothedPower[i] += power[i] - (smoothedPower[i]/8);
    }

    switch (drawMode)
    {
    case DRAWMODE_REALTIME:
        // Real-time display

        for (i = 0; i < DRAW_MAX; i++)
        {
            s32     j, index;
            fx32    tmpPower;

            // Reason for adding 1: The data in power[0] is the DC component, so it is not displayed
            index = i * DRAW_STEP + 1;
            tmpPower = 0;
            for (j = 0; j < DRAW_STEP; j++)
            {
                tmpPower += power[index];
                index++;
            }
            SDK_ASSERT(index <= sizeof(power)/sizeof(power[0]));
            tmpPower /= DRAW_STEP /*j*/;

            // Multiply by a factor suitable to scale the values to be on-screen coordinate values.
            // Display expands in the vertical direction, so only up to 192 can be displayed
            gDrawData[i] = (u8)MATH_MIN(tmpPower, 191);
        }

        break;

    case DRAWMODE_BAR:
        // Bar display

        for (i = 0; i < DRAW_MAX; i += blockCount)
        {
            s32     j, index;
            fx32    tmpPower;

            // Reason for adding 1: The data in power[0] is the DC component, so it is not displayed
            index = i * DRAW_STEP + 1;
            tmpPower = 0;
            for (j = 0; j < blockCount * DRAW_STEP && index < sizeof(smoothedPower)/sizeof(smoothedPower[0]); j++)
            {
                tmpPower += smoothedPower[index];
                index++;
            }
            tmpPower /= j;

            // Multiply by a factor suitable to scale the values to be on-screen coordinate values.
            // Display expands in the vertical direction, so only up to 192 can be displayed
            gDrawData[i] = (u8)MATH_MIN(tmpPower/8, 191);
        }
        break;

    case DRAWMODE_SCALE:
        // Musical scale display

        for (i = 0; i < 12; i ++)
        {
            s32     j;

            for (j = 0; j < SCALE_SAMPLING_OCTAVES ; j++)
            {
                u32 k;
                fx32 maxPower;

                // Maximum value in each musical scale's frequency band
                maxPower = 0;
                for (k = scaleSamplingPoints[i][j][0]; k < scaleSamplingPoints[i][j][1]; k++)
                {
                    fx32 tmpPower;
                    // Take the sum of adjacent values to handle any peaks that may occur between frequencies
                    // scaleSamplingPoints has been configured in such a way that k+1 will not overflow [1, FFT_N/2-1]
                    tmpPower = smoothedPower[k] + smoothedPower[k+1];
                    if ( maxPower < tmpPower )
                    {
                        maxPower = tmpPower;
                    }
                }

                // Multiply by a factor suitable to scale the values to be on-screen coordinate values.
                // Display expands in the vertical direction, so only up to 192 can be displayed
                gDrawData[i * SCALE_SAMPLING_OCTAVES + j] = (u8)MATH_MIN(maxPower/16, 191);
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

    OS_Printf(" sampling-span: %d , bit-range: ", gMicAutoParam.rate);
    switch (gMicAutoParam.type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
        OS_Printf("8");
        break;
    case MIC_SAMPLING_TYPE_12BIT:
        OS_Printf("12");
        break;
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        OS_Printf("signed 8");
        break;
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
        OS_Printf("signed 12");
        break;
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
        OS_Printf("12(filter off)");
        break;
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
        OS_Printf("signed 12(filter off)");
        break;
    }
    if (gMicAutoParam.loop_enable)
    {
        OS_Printf(" , loop: on\n");
    }
    else
    {
        OS_Printf(" , loop: off\n");
    }
}


/*---------------------------------------------------------------------------*
  Name:         Play

  Description:  Plays streaming playback.

  Arguments:    strm:   Stream object
                filename:   Name of the streaming playback file

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Play(StreamInfo * strm)
{
    s32     timerValue;
    u32     alarmPeriod;

    osc.index = 0;

    /* Set parameters */
    timerValue = SND_TIMER_CLOCK / STRM_SAMPLE_RATE;
    alarmPeriod = timerValue * (STRM_BUF_PAGESIZE / sizeof(s16)) / 32;

    // Read initial stream data
    strm->bufPage = 0;
    MakeStreamData(strm);
    MakeStreamData(strm);

    // Set up the channel and alarm
    SND_SetupChannelPcm(CHANNEL_NUM,
                        SND_WAVE_FORMAT_PCM16,
                        strmBuf,
                        SND_CHANNEL_LOOP_REPEAT,
                        0,
                        STRM_BUF_SIZE / sizeof(u32),
                        127, SND_CHANNEL_DATASHIFT_NONE, timerValue, 0);
    SND_SetupAlarm(ALARM_NUM, alarmPeriod, alarmPeriod, SoundAlarmHandler, strm);
    SND_StartTimer(1 << CHANNEL_NUM, 0, 1 << ALARM_NUM, 0);
}

/*---------------------------------------------------------------------------*
  Name:         StopStream

  Description:  Stops streaming playback.

  Arguments:    strm:   Stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Stop()
{
    SND_StopTimer(1 << CHANNEL_NUM, 0, 1 << ALARM_NUM, 0);
}

/*---------------------------------------------------------------------------*
  Name:         StrmThread

  Description:  The stream thread.

  Arguments:    arg - user data (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StrmThread(void * arg)
{
#pragma unused(arg)
    OSMessage message;

    OS_InitMessageQueue(&msgQ, msgBuf, 1);

    while (1)
    {
        (void)OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK);
        (void)MakeStreamData((StreamInfo *) message);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SoundAlarmHandler

  Description:  alarm callback function

  Arguments:    arg - stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SoundAlarmHandler(void *arg)
{
    (void)OS_SendMessage(&msgQ, (OSMessage)arg, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         MakeStreamData

  Description:  Generates stream data.

  Arguments:    strm:   Stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeStreamData(StreamInfo * strm)
{
    u8     *buf;
    int     i;

    // Buffer page settings
    if (strm->bufPage == 0)
    {
        buf = strmBuf;
        strm->bufPage = 1;
    }
    else
    {
        buf = strmBuf + STRM_BUF_PAGESIZE;
        strm->bufPage = 0;
    }

    // Generates data
    for (i = 0; i < STRM_BUF_PAGESIZE / sizeof(s16); i++)
    {
        ((s16 *)buf)[i] = (s16)FX_Whole(FX_Mul32x64c(osc.gain << FX32_SHIFT,
                                                     FX_SinFx64c(FX_Mul32x64c(osc.index,
                                                                              FX64C_TWOPI))));
        osc.index += osc.step;
        osc.index &= FX32_DEC_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         TpRead

  Description:  Reads the touch panel.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TpRead(TpInformation* tp)
{
    TPData  tp_data;
    TPData  tp_raw;
    int     old;

    old = tp->touch;
    while (TP_RequestRawSampling(&tp_raw) != 0)
    {
    };
    TP_GetCalibratedPoint(&tp_data, &tp_raw);

    tp->touch = tp_data.touch;
    tp->trg = tp->touch & (tp->touch ^ old);
    tp->rls = old & (old ^ tp->touch);

    switch (tp_data.validity)
    {
    case TP_VALIDITY_VALID:
        tp->x = tp_data.x;
        tp->y = tp_data.y;
        break;
    case TP_VALIDITY_INVALID_X:
        tp->y = tp_data.y;
        break;
    case TP_VALIDITY_INVALID_Y:
        tp->x = tp_data.x;
        break;
    case TP_VALIDITY_INVALID_XY:
        break;
    default:
        break;
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
