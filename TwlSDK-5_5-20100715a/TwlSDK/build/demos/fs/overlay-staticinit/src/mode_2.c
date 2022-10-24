/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     mode_2.c

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
/* Variables */

// Stylus press state
static BOOL is_tp_on;
static struct Point { int x, y; }  bak_pos, cur_pos;


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

    // Update the input state of the stylus
    bak_pos = cur_pos;
    is_tp_on = IS_INPUT_(input[own_player_id], PAD_BUTTON_TP, push);
    if (is_tp_on)
    {
        cur_pos.x = input[own_player_id].tp.x;
        cur_pos.y = input[own_player_id].tp.y;
    }
    // End if anything is pressed other than the stylus
    return !IS_INPUT_(input[own_player_id], PAD_ALL_MASK, push);
}

/*---------------------------------------------------------------------------*
  Name:         MyDrawFrame

  Description:  Performs a rendering update based on the internal state in the current mode.

  Arguments:    frame_count: Frame count of the current operation

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyDrawFrame(int frame_count)
{
    // The first frame clears the screen
    if (frame_count == 0)
    {
        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
        DEMODrawText(0, 10, "%s", __FILE__);
        DEMODrawText(30, 40, "touch screen : draw line");
        DEMODrawText(30, 50, "press any key to return");
    }
    // Render the current position and trajectory
    DEMOFillRect(cur_pos.x - 2, cur_pos.y - 2, 4, 4, GX_RGBA(0, 31, 0, 1));
    if (is_tp_on)
    {
        DEMODrawLine(bak_pos.x, bak_pos.y, cur_pos.x, cur_pos.y, GX_RGBA(16, 16, 31, 1));
    }
}

/*---------------------------------------------------------------------------*
  Name:         MyEndFrame

  Description:  Ends current mode.

  Arguments:    p_next_mode: The ID indicated by this pointer is overwritten when the next mode is specified explicitly.
                                 If no mode is specified, the mode that called the current mode will be selected.
                                 
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyEndFrame(FSOverlayID *p_next_mode)
{
    (void)p_next_mode;
}

/*---------------------------------------------------------------------------*
  Name:         NitroStaticInit

  Description:  Function for auto-initialization as static initializer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NitroStaticInit(void)
{
    UpdateFrame = MyUpdateFrame;
    DrawFrame = MyDrawFrame;
    EndFrame = MyEndFrame;

    /* Perform the necessary initialization processes for each mode here */

    is_tp_on = FALSE;
    cur_pos.x = HW_LCD_WIDTH / 2;
    cur_pos.y = HW_LCD_HEIGHT / 2;
    bak_pos = cur_pos;
}
