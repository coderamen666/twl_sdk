/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-22#$
  $Rev: 9714 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A sample that controls mic sampling status.

    USAGE:
        UP, DOWN   : Control sampling span.
        LEFT, RIGHT: Control sampling bit range. ( 8-bit, 12-bit, etc.)
        SEL , STA    : Control loop sampling enable or disable.

    HOWTO:
        1. Initialize memory allocate system to get 32byte aligned big buffer.
        2. Initialize MIC library.
        Start auto sampling of MIC by default status.
        When you change status , first , stop auto sampling.
           Then , edit status and start auto sampling again.
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

    gMicAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
    gMicAutoParam.buffer = (void *)gMicData;
    gMicAutoParam.size = TEST_BUFFER_SIZE;
    gMicAutoParam.rate = MIC_SAMPLING_RATE_8K;
    gMicAutoParam.loop_enable = TRUE;
    gMicAutoParam.full_callback = NULL;
    (void)MIC_StartAutoSampling(&gMicAutoParam);
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

    // Debug string output
    OS_Printf("ARM9: MIC demo started.\n");
    OS_Printf("   up/down    -> change sampling span\n");
    OS_Printf("   left/right -> change bit range\n");
    OS_Printf("   select     -> change loop setting\n");
    OS_Printf("\n");
    PrintfVariableData();

    // Empty call for getting key input data (strategy for pressing A button in the IPL)
    KeyRead(&gKey);

    // Main loop
    while (TRUE)
    {
        // Get key input data
        KeyRead(&gKey);

        // Change variable parameters
        {
            // Change sampling type (bit width)
            if ((gKey.trg | gKey.rep) & (PAD_KEY_LEFT | PAD_KEY_RIGHT))
            {
                //****************************************************************
                (void)MIC_StopAutoSampling();
                gMicAutoParam.type = (MICSamplingType)((gMicAutoParam.type +
                                                        1) % MIC_SAMPLING_TYPE_MAX);
                (void)MIC_StartAutoSampling(&gMicAutoParam);
                //****************************************************************
                PrintfVariableData();
            }
            // Change sampling rate
            if ((gKey.trg | gKey.rep) & PAD_KEY_UP)
            {
                //****************************************************************
                gMicAutoParam.rate = (u32)(gMicAutoParam.rate / 2);
                if (gMicAutoParam.rate < MIC_SAMPLING_RATE_LIMIT)
                {
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_LIMIT;
                }
                (void)MIC_AdjustAutoSampling(gMicAutoParam.rate);
                //****************************************************************
                PrintfVariableData();
            }
            if ((gKey.trg | gKey.rep) & PAD_KEY_DOWN)
            {
                //****************************************************************
                gMicAutoParam.rate = (u32)(gMicAutoParam.rate * 2);
                if (gMicAutoParam.rate < MIC_SAMPLING_RATE_LIMIT)
                {
                    gMicAutoParam.rate = MIC_SAMPLING_RATE_LIMIT;
                }
                (void)MIC_AdjustAutoSampling(gMicAutoParam.rate);
                //****************************************************************
                PrintfVariableData();
            }
            // Change loop availability when buffer is full
            if ((gKey.trg | gKey.rep) & (PAD_BUTTON_SELECT | PAD_BUTTON_START))
            {
                //****************************************************************
                (void)MIC_StopAutoSampling();
                gMicAutoParam.loop_enable = (gMicAutoParam.loop_enable + 1) % 2;
                (void)MIC_StartAutoSampling(&gMicAutoParam);
                //****************************************************************
                PrintfVariableData();
            }
        }

        // Render waveform
        SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
        Draw3D();

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
  Name:         Draw3D

  Description:  Displays waveforms in 3D.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Draw3D(void)
{
    G3X_Reset();
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
                         (s16)i, (s16)((s8)gDrawData[i + 1]), (s16)(i + 1));
            }
        }
        else
        {
            for (i = 0; i < 191; i++)
            {
                DrawLine((s16)(gDrawData[i]), (s16)i, (s16)(gDrawData[i + 1]), (s16)(i + 1));
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

  Returns:      None.
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
