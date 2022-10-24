/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     mode_3.c

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
/* Functions */

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
    return !IS_INPUT_(input[own_player_id], ~0, push);
}

/*---------------------------------------------------------------------------*
  Name:         MyDrawFrame

  Description:  Performs a rendering update based on the internal state in the current mode.

  Arguments:    frame_count: Frame count of the current operation

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MyDrawFrame(int frame_count)
{
    (void)frame_count;
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
    (void)p_next_mode;
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

    DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText(0, 10, "%s", __FILE__);
    DEMODrawText(30, 40, "press any key to return");
    DEMOSetBitmapTextColor(GX_RGBA(31, 16, 16, 1));
    DEMODrawText(20, 80, "( no implementations )");
    DEMO_DrawFlip();
}
