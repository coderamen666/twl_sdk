/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-14 #$
  $Rev: 9314 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"

/* Interface for each mode */
#include "mode.h"


/*---------------------------------------------------------------------------*/
/* Constants */

// Constants for automatic sampling
#define     SAMPLING_FREQUENCE      4
#define     SAMPLING_BUFSIZE        (SAMPLING_FREQUENCE + 1)
#define     SAMPLING_START_VCOUNT   0


/*---------------------------------------------------------------------------*/
/* Variables */

// Interface for each mode
BOOL    (*UpdateFrame) (int frame_count, const InputData * input, int player_count,
                        int own_player_id);
void    (*DrawFrame) (int frame);
void    (*EndFrame) (FSOverlayID *p_next_mode);

// Automatic sampling buffer for the touch panel
static TPData tp_auto_buf[SAMPLING_BUFSIZE];


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         InitApp

  Description:  Initialize the basic parts of the application.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitApp(void)
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);

    // Initialize the touch panel
    {
        TPCalibrateParam calibrate;
        TP_Init();
        if (TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
        }
        (void)TP_RequestAutoSamplingStart(SAMPLING_START_VCOUNT, SAMPLING_FREQUENCE, tp_auto_buf,
                                          SAMPLING_BUFSIZE);
    }

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    // Switch the GX settings to use the Touch Screen for the main display
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         GetInput

  Description:  Gets the current input state.

  Arguments:    input: Location that stores the obtained information

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetInput(InputData * input)
{
    if (input)
    {
        const u16 hold_bak = input->hold_bits;
        u16     hold_bits;
        // Update the touch panel information
        const TPData *const cur_tp = tp_auto_buf + TP_GetLatestIndexInAuto();
        if (cur_tp->touch == TP_TOUCH_OFF)
        {
            input->tp.touch = TP_TOUCH_OFF;
        }
        else if (cur_tp->validity == TP_VALIDITY_VALID)
        {
            TP_GetCalibratedPoint(&input->tp, cur_tp);
        }
        // Update the pad information (and the stylus touch bit)
        hold_bits = (u16)(PAD_Read() | (input->tp.touch ? PAD_BUTTON_TP : 0));
        input->push_bits = (u16)(~hold_bak & hold_bits);
        input->hold_bits = hold_bits;
        input->release_bits = (u16)(hold_bak & ~hold_bits);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main entry point to application.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Initialize the application framework and screen transition state
    int         player_count = 1;
    int         own_player_id;
    InputData   input[1];
    FSOverlayID cur_mode;
    FSOverlayID prev_mode;
    FS_EXTERN_OVERLAY(top_menu);

    InitApp();
    cur_mode = FS_OVERLAY_ID(top_menu);
    prev_mode = FS_OVERLAY_ID(top_menu);
    UpdateFrame = NULL;
    DrawFrame = NULL;
    EndFrame = NULL;
    MI_CpuClear8(input, sizeof(input));
    player_count = 1;
    own_player_id = 0;

    // Create each mode as overlay and repeatedly change between them
    for (;;)
    {
        // Start the current mode.
        // As loading completes, automatic initialization will be run in NitroStaticInit() and the required interfaces will configure themselves.
        // In UpdateFrame, the current mode will return conditions for ending the mode.
        // 
        {
            int     frame = 0;
            if (!FS_LoadOverlay(MI_PROCESSOR_ARM9, cur_mode))
            {
                OS_TPanic("failed to load specified mode(%08X)", cur_mode);
            }
            GetInput(&input[own_player_id]);
            for (;;)
            {
                GetInput(&input[own_player_id]);
                if (!UpdateFrame(frame, input, player_count, own_player_id))
                {
                    break;
                }
                DrawFrame(frame);
                DEMO_DrawFlip();
                OS_WaitVBlankIntr();
                ++frame;
            }
        }
        // End the current mode and transition to the next one.
        // If the current mode has not explicitly set a mode to transition to, it will return to the previous mode.
        // 
        {
            FSOverlayID next_mode = prev_mode;
            EndFrame(&next_mode);
            if (!FS_UnloadOverlay(MI_PROCESSOR_ARM9, cur_mode))
            {
                OS_TPanic("failed to unload specified mode(%08X)", cur_mode);
            }
            prev_mode = cur_mode;
            cur_mode = next_mode;
        }
    }

}
