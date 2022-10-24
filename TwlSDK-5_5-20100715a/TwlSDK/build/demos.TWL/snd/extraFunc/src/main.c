/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - snd - extraFunc
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
    A sample that uses the SNDEX library.

    USAGE:
        UP, DOWN: Switch item to control.
        LEFT, RIGHT: Set configuration related to selected item.

    HOWTO:
        1. Initialize SNDEX library.
        2. Call functions to set/get each configuration.
 *---------------------------------------------------------------------------*/

#include    <twl.h>
#include    "font.h"
#include    "snd_data.h"
#include    "DEMO.h"

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
#define     INDEX_MAX   4

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16      screen[32 * 32] ATTRIBUTE_ALIGN(HW_CACHE_LINE_SIZE);
static s32      index   =   0;

static SNDEXMute        mute    =   SNDEX_MUTE_OFF;
static SNDEXFrequency   freq    =   SNDEX_FREQUENCY_32730;
static u8               rate    =   0;
static u8               volume  =   0;
static BOOL             shutter =   FALSE;


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static BOOL     CheckResult     (SNDEXResult result);
static void     UpdateScreen    (void);
static void     VBlankIntr      (void);
static void     PrintString     (s16 x, s16 y, u8 palette, char *text, ...);
static void VolumeSwitchCallback(SNDEXResult result, void* arg);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TwlMain (void)
{
    /* Initialization */
    OS_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    /* Initializes display settings */
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void*)HW_OAM, GX_LCD_SIZE_Y, HW_OAM_SIZE);
    MI_CpuClearFast((void*)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void*)HW_DB_OAM, GX_LCD_SIZE_Y, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
            GX_BG_COLORMODE_16,
            GX_BG_SCRBASE_0xf800,      // SCR base block 31
            GX_BG_CHARBASE_0x00000,    // CHR base block 0
            GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    MI_CpuClearFast((void*)screen, sizeof(screen));
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* Interrupt settings */
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /* Initialize sound */
    SND_Init();
    SND_AssignWaveArc((SNDBankData*)sound_bank_data, 0, (SNDWaveArc*)sound_wave_data);
    SND_StartSeq(0, sound_seq_data, 0, (SNDBankData*)sound_bank_data);

    /* Initialize extended sound features */
    SNDEX_Init();
    (void)CheckResult(SNDEX_GetMute(&mute));
    (void)CheckResult(SNDEX_GetI2SFrequency(&freq));
    (void)CheckResult(SNDEX_GetDSPMixRate(&rate));
    (void)CheckResult(SNDEX_GetVolume(&volume));
    UpdateScreen();
    
    /* Set the callback function for when the volume button is pressed */
    SNDEX_SetVolumeSwitchCallback(VolumeSwitchCallback, NULL);

    /* LCD display start */
    GX_DispOn();
    GXS_DispOn();

    {
        u16     keyOld  =   PAD_Read();
        u16     keyTrg;
        u16     keyNow;
        SNDEXHeadphone hp;
    
        /* Main loop */
        while (TRUE)
        {
            /* Get key input data */
            keyNow  =   PAD_Read();
            keyTrg  =   (u16)((keyOld ^ keyNow) & keyNow);
            keyOld  =   keyNow;
        
            /* Perform various operations in accordance with input */
            if (keyTrg & PAD_KEY_UP)
            {
                index   =   (index + INDEX_MAX - 1) % INDEX_MAX;
            }
            if (keyTrg & PAD_KEY_DOWN)
            {
                index   =   (index + 1) % INDEX_MAX;
            }
            if (keyTrg & PAD_KEY_RIGHT)
            {
                switch (index)
                {
                case 0: // Mute control
                    if (mute == SNDEX_MUTE_OFF)
                    {
                        if (CheckResult(SNDEX_SetMute(SNDEX_MUTE_ON)) == TRUE)
                        {
                            mute    =   SNDEX_MUTE_ON;
                        }
                    }
                    break;
                case 1: // I2S frequency control
                    if (freq == SNDEX_FREQUENCY_32730)
                    {
                        if (CheckResult(SNDEX_SetI2SFrequency(SNDEX_FREQUENCY_47610)) == TRUE)
                        {
                            freq    =   SNDEX_FREQUENCY_47610;
                        }
                    }
                    break;
                case 2: // DSP mix rate control
                    if (rate < SNDEX_DSP_MIX_RATE_MAX)
                    {
                        if (CheckResult(SNDEX_SetDSPMixRate((u8)(rate + 1))) == TRUE)
                        {
                            rate ++;
                        }
                    }
                    break;
                case 3: // Volume control
                    if (volume < SNDEX_VOLUME_MAX)
                    {
                        if (CheckResult(SNDEX_SetVolume((u8)(volume + 1))) == TRUE)
                        {
                            volume ++;
                        }
                    }
                    break;
                }
            }
            if (keyTrg & PAD_KEY_LEFT)
            {
                switch (index)
                {
                case 0: // Mute control
                    if (mute == SNDEX_MUTE_ON)
                    {
                        if (CheckResult(SNDEX_SetMute(SNDEX_MUTE_OFF)) == TRUE)
                        {
                            mute    =   SNDEX_MUTE_OFF;
                        }
                    }
                    break;
                case 1: // I2S frequency control
                    if (freq == SNDEX_FREQUENCY_47610)
                    {
                        if (CheckResult(SNDEX_SetI2SFrequency(SNDEX_FREQUENCY_32730)) == TRUE)
                        {
                            freq    =   SNDEX_FREQUENCY_32730;
                        }
                    }
                    break;
                case 2: // DSP mix rate control
                    if (rate > SNDEX_DSP_MIX_RATE_MIN)
                    {
                        if (CheckResult(SNDEX_SetDSPMixRate((u8)(rate - 1))) == TRUE)
                        {
                            rate --;
                        }
                    }
                    break;
                case 3: // Volume control
                    if (volume > SNDEX_VOLUME_MIN)
                    {
                        if (CheckResult(SNDEX_SetVolume((u8)(volume - 1))) == TRUE)
                        {
                            volume --;
                        }
                    }
                    break;
                }
            }
            if (keyTrg & PAD_BUTTON_A)
            {
                /* Determine if headphones are connected */
                if (SNDEX_PXI_RESULT_SUCCESS == SNDEX_IsConnectedHeadphone(&hp))
                {
                    if (hp == SNDEX_HEADPHONE_CONNECT)
                    {
                        OS_TPrintf("Headphone is connected.\n");
                    }
                    else
                    {
                        OS_TPrintf("Headphone is unconnected.\n");
                    }
                }
            }
        
            /* Main sound processing */
            while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
            {
            }
            (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
            
            /* Make the 'volume' value change in response to changes in audio volume */
            (void)CheckResult(SNDEX_GetVolume(&volume));
            
            /* Screen update */
            UpdateScreen();
        
            /* Wait for V-Blank */
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CheckResult

  Description:  Confirms the results of SNDEX library function calls.

  Arguments:    result: Value returned from the function

  Returns:      BOOL: Returns TRUE when the function call was successful.
                            Returns FALSE if there was a failure for any reason.
 *---------------------------------------------------------------------------*/
static BOOL
CheckResult (SNDEXResult result)
{
    switch (result)
    {
    case SNDEX_RESULT_SUCCESS:
        return TRUE;
    case SNDEX_RESULT_BEFORE_INIT:
        /* Before library initialization.
            The SNDEX_Init function must be called before any other functions. */
        OS_TWarning("Not initialized.\n");
        break;
    case SNDEX_RESULT_INVALID_PARAM:
        /* Unknown parameter.
            Review the argument(s) being passed to the function. */
        OS_TWarning("Invalid parameter.\n");
        break;
    case SNDEX_RESULT_EXCLUSIVE:
        /* Mutex is in effect.
            Do not make consecutive calls of asynchronous functions or use the SNDEX library asynchronously from multiple threads.
             */
        OS_TWarning("Overlapped requests.\n");
        break;
    case SNDEX_RESULT_ILLEGAL_STATE:
        /* Abnormal state.
            The SNDEX library can only be used with TWL hardware.
            Synchronous functions can't be called from within exception handlers.
            When the CODEC is in DS Mode, you cannot switch to forced audio output.
            When the CODEC is in DS Mode, when audio output is being forced, or when the microphone is auto-sampling, the I2S frequency cannot be changed.
             */
        OS_TWarning("In unavailable state.\n");
        break;
    case SNDEX_RESULT_PXI_SEND_ERROR:
        /* PXI send error.
            The data sending queue from ARM9 to ARM7 is full. Please wait a little before retrying so that ARM7 can delete data within the queue.
             */
        OS_TWarning("PXI queue full.\n");
        break;
    case SNDEX_RESULT_DEVICE_ERROR:
        /* Failed to operate device.
            An attempt was made to access an external CPU device (the microprocessor) to get or change volume, but that access failed.
            If conditions don't improve even after multiple retries, consider it a success and move on, since the problem is likely caused by a runaway microprocessor, malfunctioning hardware, or some other thing not recoverable by software.
            
             */
        OS_TWarning("Micro controller unusual.\n");
        return TRUE;
    case SNDEX_RESULT_FATAL_ERROR:
    default:
        /* Fatal error.
            By the library's logic, this cannot occur. It's possible this occurred (1) when a PXI command was directly issued to ARM7 (ignoring the library's internal status management), (2) when memory damage resulted in inconsistencies in internal status management, or (3) when this was combined with an unexpected ARM7 component, or for some similar reason.
            
            
             */
        OS_TWarning("Fatal error.\n");
        break;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         UpdateScreen

  Description:  Updates the displayed screen content.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
UpdateScreen (void)
{
    /* Clear the virtual screen */
    MI_CpuClearFast((void*)screen, sizeof(screen));

    /* Edit a string on the virtual screen */
    PrintString(2, 1, 0xf, "Mute     : %d/1 [ %s ]", mute, ((mute == SNDEX_MUTE_OFF) ? "OFF" : "ON"));
    PrintString(2, 3, 0xf, "I2S freq : %d/1 [ %s ]", freq, ((freq == SNDEX_FREQUENCY_32730) ? "32K" : "48K"));
    PrintString(2, 5, 0xf, "DSP mix  : %d/8", rate);
    PrintString(2, 7, 0xf, "Volume   : %d/%d", volume, (u8)SNDEX_VOLUME_MAX);
    
    PrintString(0, 11, 0xe, "- [A] : Check Headphone");
    PrintString(0, 13, 0xe, "- [Volume Button] : Print log");

    /* Change the selected item's display color */
    PrintString(0, (s16)(index * 2 + 1), 0xf, ">");
    {
        s32     i;
        s32     j;
    
        for (i = 0; i < 32; i ++)
        {
            j   =   ((index * 2 + 1) * 32) + i;
            screen[j]   &=  (u16)(~(0xf << 12));
            screen[j]   |=  (u16)(0x4 << 12);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt vector.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
VBlankIntr (void)
{
    /* Reflect virtual screen in VRAM */
    DC_FlushRange(screen, sizeof(screen));
    GX_LoadBG0Scr(screen, 0, sizeof(screen));

    /* Sets the IRQ check flag */
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  Positions the text string on the virtual screen. The string can be up to 32 chars.

  Arguments:    x: X-coordinate where character string starts (x 8 pixels).
                y: Y-coordinate where character string starts (x 8 pixels).
                palette: Specify text color by palette number.
                text: Text string to position. Null-terminated.
                ...: Virtual argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
PrintString (s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *((u16*)(&temp[32]))    =   0x0000;
    for (i = 0; ; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        screen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         VolumeSwitchCallback

  Description:  Callback function executed when the volume button on the TWL is pressed.

  Arguments:    result: Result of pressing the volume button
                arg: Argument passed when this function is called as a callback function
                

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VolumeSwitchCallback(SNDEXResult result, void* arg)
{
#pragma unused( arg )
#pragma unused( result )

    /* SNDEX library functions cannot be used during callback processing.*
     * SNDEX_RESULT_EXCLUSIVE is returned.                         */
    OS_TPrintf("volume switch pressed.\n");
}
