/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     top_menu.c

  Copyright 2007 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-06 #$
  $Rev: 2135 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "mode.h"
#include "DEMO.h"

// Include this header when NitroStaticInit() is specified as a static initializer.
// 
#include <nitro/sinit.h>


/*---------------------------------------------------------------------------*/
/* Constants */

// The overlay ID of each mode to be selected from the top menu
FS_EXTERN_OVERLAY(mode_1);
FS_EXTERN_OVERLAY(mode_2);
FS_EXTERN_OVERLAY(mode_3);

enum
{
    MENU_MODE_1,
    MENU_MODE_2,
    MENU_MODE_3,
    MENU_BEGIN = 0,
    MENU_END = MENU_MODE_3,
    MENU_MAX = MENU_END + 1
};


/*---------------------------------------------------------------------------*/
/* Variables */

// menu cursor
static int menu_cursor;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         MyUpdateFrame

  Description:  Updates the internal state by one frame in the current mode.

  Arguments:    frame_count: Frame count of the current operation
                input: Array of input data
                player_count: Current number of total players (the number of valid input elements)
                own_player_id: Local player number

  Returns:      Returns FALSE if the current mode ends this frame and TRUE otherwise.
                
 *---------------------------------------------------------------------------*/
static BOOL MyUpdateFrame(int frame_count,
                          const InputData * input, int player_count, int own_player_id)
{
    (void)frame_count;
    (void)player_count;

    // Select menu with Up and Down keys
    if (IS_INPUT_(input[own_player_id], PAD_KEY_UP, push))
    {
        if (--menu_cursor < MENU_BEGIN)
        {
            menu_cursor = MENU_END;
        }
    }
    if (IS_INPUT_(input[own_player_id], PAD_KEY_DOWN, push))
    {
        if (++menu_cursor > MENU_END)
        {
            menu_cursor = MENU_BEGIN;
        }
    }
    // Confirm with the A Button
    return !IS_INPUT_(input[own_player_id], PAD_BUTTON_A, push);
}

/*---------------------------------------------------------------------------*
  Name:         MyDrawFrame

  Description:  Performs a rendering update based on the internal state in the current mode.

  Arguments:    frame_count: Frame count of the current operation

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyDrawFrame(int frame_count)
{
    int     i;

    (void)frame_count;

    DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
    // Display file name and a simple description of operations
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText(0, 10, "%s", __FILE__);
    DEMODrawText(30, 40, "up/down: select menu");
    DEMODrawText(30, 50, "   A   : run selected mode");
    // Menu and cursor display
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 16, 1));
    for (i = MENU_BEGIN; i <= MENU_END; ++i)
    {
        DEMODrawText(40, 80 + i * 10, "%c mode %d", ((i == menu_cursor) ? '>' : ' '), i + 1);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MyEndFrame

  Description:  Ends the current mode.

  Arguments:    p_next_mode: The ID indicated by this pointer is overwritten when the next mode is specified explicitly.
                                 In the absence of a particular specification, the mode that called the current mode is selected.
                                 
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyEndFrame(FSOverlayID *p_next_mode)
{
    // Expressly configure and return the selected mode
    switch (menu_cursor)
    {
    case MENU_MODE_1:
        *p_next_mode = FS_OVERLAY_ID(mode_1);
        break;
    case MENU_MODE_2:
        *p_next_mode = FS_OVERLAY_ID(mode_2);
        break;
    case MENU_MODE_3:
        *p_next_mode = FS_OVERLAY_ID(mode_3);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroStaticInit

  Description:  This is an automatic initialization function as a static initializer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NitroStaticInit(void)
{
    UpdateFrame = MyUpdateFrame;
    DrawFrame = MyDrawFrame;
    EndFrame = MyEndFrame;

    // Perform the necessary initialization processes for each mode here

    menu_cursor = 0;
}
