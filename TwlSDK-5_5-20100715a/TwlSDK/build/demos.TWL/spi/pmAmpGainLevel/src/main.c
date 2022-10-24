/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - spi - pmAmpGainLevel
  File:     main.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    A sample that controls the amplifier gain for microphone input.

    USAGE:
        UP, DOWN: Control amplifier gain
        LEFT: Minimize amplifier gain
        RIGHT: Maximize amplifier gain

    HOWTO:
        1. Initialize MIC library.
        2. Start auto sampling of MIC by default configuration.
        3. When you change amplifier gain, you don't have to stop sampling.
 *---------------------------------------------------------------------------*/

#include    <twl.h>
#include    "DEMO.h"

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
#define     TEST_BUFFER_SIZE    (4 * 1024)   // 4 KB
#define     RETRY_MAX_COUNT     8

/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void     StepUpAmpGain(void);
static void     StepDownAmpGain(void);
static void     MaximizeAmpGain(void);
static void     MinimizeAmpGain(void);
static u32      GetDefaultMicSamplingRate(void);
static SNDEXFrequency   GetI2SFrequency(void);
static void     StartMicSampling(const MICAutoParam* param);

static void     SetDrawData(void *address, MICSamplingType type);

static void     VBlankIntr(void);
static void     Init3D(void);
static void     Draw3D(void);
static void     DrawLine(s16 sx, s16 sy, s16 ex, s16 ey);

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static MICAutoParam gMicAutoParam;
static u8           gDrawData[192];
static u8           gMicData[TEST_BUFFER_SIZE] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    /* Various types of initialization */
    OS_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    /* Initializes display settings */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, GX_LCD_SIZE_Y, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, GX_LCD_SIZE_Y, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    /* 3D-related initialization */
    Init3D();

    /* Interrupt settings */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    (void)GX_VBlankIntr(TRUE);

    /* Initialize related libraries */
    {
        (void)PM_SetAmp(PM_AMP_ON);
        (void)PM_SetAmpGain(PM_AMPGAIN_80);
    
        MIC_Init();
        gMicAutoParam.type          =   MIC_SAMPLING_TYPE_8BIT;
        gMicAutoParam.buffer        =   (void *)gMicData;
        gMicAutoParam.size          =   TEST_BUFFER_SIZE;
        gMicAutoParam.loop_enable   =   TRUE;
        gMicAutoParam.full_callback =   NULL;
        gMicAutoParam.rate          =   GetDefaultMicSamplingRate();
        StartMicSampling((const MICAutoParam *)&gMicAutoParam);
    }

    /* Initialize internal variables */
    MI_CpuFill8(gDrawData, 0x80, sizeof(gDrawData));

    /* LCD display start */
    GX_DispOn();
    GXS_DispOn();

    /* Debug string output */
    OS_Printf("pmAmpGainLevel demo started.\n");
    OS_Printf("   up/down    -> change amplifier gain\n");
    OS_Printf("   left       -> minimize amplifier gain\n");
    OS_Printf("   right      -> maximize amplifier gain\n");
    OS_Printf("\n");

    /* Empty call for getting key input data (strategy for pressing A Button in the IPL) */
    (void)PAD_Read();

    {
        u16     keyOld  =   0;
        u16     keyTrg;
        u16     keyNow;
    
        /* Main loop */
        while (TRUE)
        {
            /* Get key input data */
            keyNow  =   PAD_Read();
            keyTrg  =   (u16)((keyOld ^ keyNow) & keyNow);
            keyOld  =   keyNow;
        
            /* Change the amp gain */
            if (keyTrg & PAD_KEY_UP)
            {
                StepUpAmpGain();
            }
            if (keyTrg & PAD_KEY_DOWN)
            {
                StepDownAmpGain();
            }
            if (keyTrg & PAD_KEY_LEFT)
            {
                MinimizeAmpGain();
            }
            if (keyTrg & PAD_KEY_RIGHT)
            {
                MaximizeAmpGain();
            }
        
            /* Render waveform */
            SetDrawData(MIC_GetLastSamplingAddress(), gMicAutoParam.type);
            Draw3D();
        
            /* Waiting for the V-Blank */
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StepDownAmpGain

  Description:  Raises the amp gain by one level.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StepUpAmpGain(void)
{
    u8  gain;

    if (PM_GetAmpGainLevel(&gain) == PM_RESULT_SUCCESS)
    {
        if (OS_IsRunOnTwl() == TRUE)
        {
            gain ++;
            if ((gain >= PM_AMPGAIN_LEVEL_MIN) && (gain <= PM_AMPGAIN_LEVEL_MAX))
            {
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
        else
        {
            switch (gain)
            {
            case PM_AMPGAIN_LEVEL_DS0:
                gain    =   PM_AMPGAIN_LEVEL_DS1;
                break;
            case PM_AMPGAIN_LEVEL_DS1:
                gain    =   PM_AMPGAIN_LEVEL_DS2;
                break;
            case PM_AMPGAIN_LEVEL_DS2:
                gain    =   PM_AMPGAIN_LEVEL_DS3;
                break;
            default:
                return;
            }
            if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
            {
                OS_Printf("Amplifier gain level was changed to %d\n", gain);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StepDownAmpGain

  Description:  Lowers the amp gain by one level.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StepDownAmpGain(void)
{
    u8  gain;

    if (PM_GetAmpGainLevel(&gain) == PM_RESULT_SUCCESS)
    {
        if (OS_IsRunOnTwl() == TRUE)
        {
            gain --;
            if ((gain >= PM_AMPGAIN_LEVEL_MIN) && (gain <= PM_AMPGAIN_LEVEL_MAX))
            {
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
        else
        {
            switch (gain)
            {
            case PM_AMPGAIN_LEVEL_DS3:
                gain    =   PM_AMPGAIN_LEVEL_DS2;
                break;
            case PM_AMPGAIN_LEVEL_DS2:
                gain    =   PM_AMPGAIN_LEVEL_DS1;
                break;
            case PM_AMPGAIN_LEVEL_DS1:
                gain    =   PM_AMPGAIN_LEVEL_DS0;
                break;
            default:
                return;
            }
            if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
            {
                OS_Printf("Amplifier gain level was changed to %d\n", gain);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MaximizeAmpGain

  Description:  Maximizes amp gain.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
MaximizeAmpGain(void)
{
    u8  gain;

    if (PM_GetAmpGainLevel(&gain) == PM_RESULT_SUCCESS)
    {
        if (OS_IsRunOnTwl() == TRUE)
        {
            if (gain != PM_AMPGAIN_LEVEL_MAX)
            {
                gain    =   PM_AMPGAIN_LEVEL_MAX;
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
        else
        {
            if (gain != PM_AMPGAIN_LEVEL_DS3)
            {
                gain    =   PM_AMPGAIN_LEVEL_DS3;
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MinimizeAmpGain

  Description:  Minimizes amp gain.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
MinimizeAmpGain(void)
{
    u8  gain;

    if (PM_GetAmpGainLevel(&gain) == PM_RESULT_SUCCESS)
    {
        if (OS_IsRunOnTwl() == TRUE)
        {
            if (gain != PM_AMPGAIN_LEVEL_MIN)
            {
                gain    =   PM_AMPGAIN_LEVEL_MIN;
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
        else
        {
            if (gain != PM_AMPGAIN_LEVEL_DS0)
            {
                gain    =   PM_AMPGAIN_LEVEL_DS0;
                if (PM_SetAmpGainLevel(gain) == PM_RESULT_SUCCESS)
                {
                    OS_Printf("Amplifier gain level was changed to %d\n", gain);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetDefaultMicSamplingRate

  Description:  Determines the sampling rate for the microphone's auto-sampling.

  Arguments:    None.

  Returns:      u32: Returns an appropriate sampling rate.
 *---------------------------------------------------------------------------*/
static u32
GetDefaultMicSamplingRate(void)
{
    if ((OS_IsRunOnTwl() == TRUE) && (GetI2SFrequency() == SNDEX_FREQUENCY_47610))
    {
        return MIC_SAMPLING_RATE_15870;
    }
    else
    {
        return MIC_SAMPLING_RATE_16360;
    }
}

#include    <twl/ltdmain_begin.h>
/*---------------------------------------------------------------------------*
  Name:         GetI2SFrequency

  Description:  Gets the I2S operating frequency.

  Arguments:    None.

  Returns:      SNDEXFrequency: Returns the I2S operating frequency.
 *---------------------------------------------------------------------------*/
static SNDEXFrequency
GetI2SFrequency(void)
{
    SNDEXResult     result;
    SNDEXFrequency  freq;
    s32             retry   =   0;

    SNDEX_Init();
    while (TRUE)
    {
        result  =   SNDEX_GetI2SFrequency(&freq);
        switch (result)
        {
        case SNDEX_RESULT_SUCCESS:          // Success
            return freq;
        case SNDEX_RESULT_EXCLUSIVE:        // Exclusion error
        case SNDEX_RESULT_PXI_SEND_ERROR:   // PXI queue is full
            if (++ retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("%s: Retry count overflow.\n", __FUNCTION__);
            break;
        case SNDEX_RESULT_BEFORE_INIT:      // Pre-initialization
        case SNDEX_RESULT_ILLEGAL_STATE:    // Abnormal state
            OS_TWarning("%s: Illegal state to get I2S frequency.\n", __FUNCTION__);
            break;
        case SNDEX_RESULT_FATAL_ERROR:      // Fatal error
        default:                            // Other fatal error
            OS_Panic("Fatal error (%d).\n", result);
            /* Never return */
        }
    }
    return SNDEX_FREQUENCY_32730;
}
#include    <twl/ltdmain_end.h>

/*---------------------------------------------------------------------------*
  Name:         StartMicSampling

  Description:  Calls the microphone auto-sampling start function and performs error processing.

  Arguments:    param: Parameters passed to the microphone functions

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
StartMicSampling(const MICAutoParam* param)
{
    MICResult   result;
    s32         retry   =   0;

    while (TRUE)
    {
        result  =   MIC_StartLimitedSampling(param);
        switch (result)
        {
        case MIC_RESULT_SUCCESS:            // Success
            return;
        case MIC_RESULT_BUSY:               // Another thread is using the microphone
        case MIC_RESULT_SEND_ERROR:         // PXI queue is full
            if (++retry <= RETRY_MAX_COUNT)
            {
                OS_Sleep(1);
                continue;
            }
            OS_TWarning("%s: Retry count overflow.\n", __FUNCTION__);
            return;
        case MIC_RESULT_ILLEGAL_STATUS:     // Auto-sampling is not currently paused
            OS_TWarning("%s: Already started sampling.\n", __FUNCTION__);
            return;
        case MIC_RESULT_ILLEGAL_PARAMETER:  // Unsupported parameter
            OS_TWarning("%s: Illegal parameter to start automatic sampling.\n", __FUNCTION__);
            return;
        default:                            // Other fatal error
            OS_Panic("Fatal error (%d).\n", result);
            /* Never return */
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetDrawData

  Description:  Stores the current newest sampled data in the buffer that puts it on the display.
                

  Arguments:    address: Location in main memory where the most recent sampling data was stored by the component
                          
                type: Sampling type (bit width)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetDrawData(void *address, MICSamplingType type)
{
    s32     i;

    /* If sampling has never been performed, do nothing and stop.
       (Because it would delete memory cache(s) unrelated to the microphone) */
    if ((address < gMicData) || (address >= (gMicData + TEST_BUFFER_SIZE)))
    {
        return;
    }

    switch (type)
    {
    case MIC_SAMPLING_TYPE_8BIT:
    case MIC_SAMPLING_TYPE_SIGNED_8BIT:
        /* In the case of 8-bit sampling */
        {
            u8     *p;

            p = (u8 *)((u32)address - 191);
            if (p < gMicData)
            {
                p = (u8 *)((u32)p + TEST_BUFFER_SIZE);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawData[i] = *p;
                p++;
                if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
                {
                    p -= TEST_BUFFER_SIZE;
                }
                if (((u32)p % HW_CACHE_LINE_SIZE) == 0)
                {
                    DC_InvalidateRange(p, HW_CACHE_LINE_SIZE);
                }
            }
        }
        break;
    case MIC_SAMPLING_TYPE_12BIT:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT:
    case MIC_SAMPLING_TYPE_12BIT_FILTER_OFF:
    case MIC_SAMPLING_TYPE_SIGNED_12BIT_FILTER_OFF:
        /* For 12-bit sampling */
        {
            u16    *p;

            p = (u16 *)((u32)address - 382);
            if ((u32)p < (u32)gMicData)
            {
                p = (u16 *)((u32)p + TEST_BUFFER_SIZE);
            }
            DC_InvalidateRange((void *)((u32)p & ~(HW_CACHE_LINE_SIZE - 1)), HW_CACHE_LINE_SIZE);
            for (i = 0; i < 192; i++)
            {
                gDrawData[i] = (u8)((*p >> 8) & 0x00ff);
                p++;
                if ((u32)p >= (u32)(gMicData + TEST_BUFFER_SIZE))
                {
                    p = (u16 *)((u32)p - TEST_BUFFER_SIZE);
                }
                if (((u32)p % HW_CACHE_LINE_SIZE) == 0)
                {
                    DC_InvalidateRange(p, HW_CACHE_LINE_SIZE);
                }
            }
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    /* Sets the IRQ check flag */
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         Init3D

  Description:  Initialization for 3D display

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

  Description:  Renders lines with triangular polygons

  Arguments:    sx:   x coordinate of line's starting point
                sy:   y coordinate of line's starting point
                ex:   x coordinate of line's ending point
                ey:   y coordinate of line's ending point

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

