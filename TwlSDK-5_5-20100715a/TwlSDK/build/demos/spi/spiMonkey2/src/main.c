/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-06#$
  $Rev: 10864 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro.h>
#include    <nitro/spi/common/pm_common.h>
#include    <nitro/spi/ARM9/pm.h>

#include    "snd_data.h"


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats
#define     TEST_BUFFER_SIZE    ( 1024 * 1024 ) // 1M

//
// Maximum sampling rate for the microphone
//  Using the mic simultaneously with SOUND or the touch screen lowers the maximum sampling rate, so here we limit it to a maximum of 22K. 
//  
//  If concentrating only on mic sampling, it is possible to operate up to LIMIT.
//
#define     MY_SAMPLING_RATE_LIMIT  MIC_SAMPLING_RATE_22K


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
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color);
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
static TPData gTpData;
static u8 gDrawData[192];


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
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

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
    MIC_Init();

    // Initialize PMIC
    PM_Init();
    // AMP on
    (void)PM_SetAmp(PM_AMP_ON);
    // Adjust AMP gain
    (void)PM_SetAmpGain(PM_AMPGAIN_80);

    gMicAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
    gMicAutoParam.buffer = (void *)gMicData;
    gMicAutoParam.size = TEST_BUFFER_SIZE;
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
    gMicAutoParam.loop_enable = TRUE;
    gMicAutoParam.full_callback = NULL;
    (void)MIC_StartAutoSampling(&gMicAutoParam);
    //****************************************************************

    // Touch Panel API Initialization.
    {
        TPCalibrateParam calibrate;

        TP_Init();
        if (TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
        }
        else
        {
            OS_Panic("Can't find TP calibration data.");
        }
    }


    // Initialize sound
    {
        SND_Init();
        SND_AssignWaveArc((SNDBankData *)sound_bank_data, 0, (SNDWaveArc *)sound_wave_data);
        SND_StartSeq(0, sound_seq_data, 0, (SNDBankData *)sound_bank_data);
    }

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

    // Debug string output
    OS_Printf("ARM9: MIC demo started.\n");
    OS_Printf("   up/down    -> change sampling span\n");
    OS_Printf("   left/right -> change bit range\n");
    OS_Printf("\n");
    PrintfVariableData();
    gTpData.touch = TP_TOUCH_OFF;

    // Empty call for getting key input data (strategy for pressing A button in the IPL)
    KeyRead(&gKey);

    // Main loop
    while (TRUE)
    {
        // Get key input data
        KeyRead(&gKey);

        // TP sampling can be executed once during automatic MIC sampling.
        // However, it is only executed after the next MIC sampling, so there is a brief delay before the results of the request are returned.
        //  
        {
            TP_RequestSamplingAsync();
            // <- During MIC automatic sampling, it will take a bit longer for the TP results to return.

            if (TP_WaitCalibratedResult(&gTpData) != 0)
            {
                OS_Printf("TP_RequestSampling command fail\n");
            }
        }

        // Change variable parameters
        {
            // Change sampling type (bit width)
            if ((gKey.trg | gKey.rep) & (PAD_KEY_LEFT | PAD_KEY_RIGHT))
            {
                //****************************************************************
                (void)MIC_StopAutoSampling();
                gMicAutoParam.type =
                    (MICSamplingType)((gMicAutoParam.type + 1) % MIC_SAMPLING_TYPE_MAX);
                (void)MIC_StartAutoSampling(&gMicAutoParam);
                //****************************************************************
                PrintfVariableData();
            }
            // Change sampling rate
            if ((gKey.trg | gKey.rep) & PAD_KEY_UP)
            {
                //****************************************************************
                (void)MIC_StopAutoSampling();
                gMicAutoParam.rate = (u32)(gMicAutoParam.rate / 2);
                if (gMicAutoParam.rate < MY_SAMPLING_RATE_LIMIT)
                {
                    gMicAutoParam.rate = MY_SAMPLING_RATE_LIMIT;
                }

                (void)MIC_StartAutoSampling(&gMicAutoParam);
                //****************************************************************
                PrintfVariableData();
            }
            if ((gKey.trg | gKey.rep) & PAD_KEY_DOWN)
            {
                //****************************************************************
                (void)MIC_StopAutoSampling();
                gMicAutoParam.rate = (u32)(gMicAutoParam.rate * 2);
                if (gMicAutoParam.rate < MY_SAMPLING_RATE_LIMIT)
                {
                    gMicAutoParam.rate = MY_SAMPLING_RATE_LIMIT;
                }
                (void)MIC_StartAutoSampling(&gMicAutoParam);
                //****************************************************************
                PrintfVariableData();
            }
        }

        // Render waveform
        SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);

        // Render
        Draw3D();

        // Sound
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

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

    {
        s32     i;

        if ((gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_8BIT) ||
            (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT) ||
            (gMicAutoParam.type == MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF))
        {
            for (i = 0; i < 191; i++)
            {
                DrawLine((s16)((s8)gDrawData[i]),
                         (s16)i, (s16)((s8)gDrawData[i + 1]), (s16)(i + 1), GX_RGB(31, 31, 0));
            }
        }
        else
        {
            for (i = 0; i < 191; i++)
            {
                DrawLine((s16)(gDrawData[i]),
                         (s16)i, (s16)(gDrawData[i + 1]), (s16)(i + 1), GX_RGB(31, 31, 0));
            }
        }

        // Display Touch Panel contact as '+' mark
        if (gTpData.touch == TP_TOUCH_ON)
        {
            if (!(gTpData.validity & TP_VALIDITY_INVALID_X))
            {
                // Display the x coordinate with a line
                DrawLine((s16)(gTpData.x),
                         (s16)(0), (s16)(gTpData.x), (s16)(191), GX_RGB(31, 31, 31));
            }
            if (!(gTpData.validity & TP_VALIDITY_INVALID_Y))
            {
                // Display the y coordinate with a line
                DrawLine((s16)(0),
                         (s16)(gTpData.y), (s16)(255), (s16)(gTpData.y), GX_RGB(31, 31, 31));
            }
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
                color: Color of line drawn

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color)
{
    fx16    fsx;
    fx16    fsy;
    fx16    fex;
    fx16    fey;

    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fey = (fx16)(((96 - ey) * 0x1000) / 96);

    G3_Begin(GX_BEGIN_TRIANGLES);
    {
        G3_Color(color);
        G3_Vtx(fsx, fsy, 0);
        G3_Color(color);
        G3_Vtx(fex, fey, 0);
        G3_Color(color);
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

    // If sampling has never been performed, do nothing and stop.
    // (Because it would delete memory cache(s) unrelated to the microphone)
    if ((address < gMicData) || (address >= (gMicData + TEST_BUFFER_SIZE)))
    {
        return;
    }

    // In the case of 8-bit sampling
    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
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
        break;
    case MIC_SAMPLING_TYPE_12BIT:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
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
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintfVariableData

  Description:  Print out the variable sampling settings.

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
  End of file
 *---------------------------------------------------------------------------*/
