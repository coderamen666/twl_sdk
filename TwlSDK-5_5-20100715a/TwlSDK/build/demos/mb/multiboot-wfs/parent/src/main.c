/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - parent
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    This sample program runs the WBT library so that wireless multiboot children can use a file system
    

    HOWTO:
        1. To start communications as a parent, press the A button.
           When a child is found in the surrounding area with the same multiboot-wfs demo, communication automatically begins.
            Communication with up to 15 children is possible at the same time.
        2. For information on various features while connected, refer to the parent and child screens as well as comments in the source code.
           
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include    <twl.h>
#else
#include    <nitro.h>
#endif
#include <nitro/wm.h>
#include <nitro/wbt.h>
#include <nitro/fs.h>

#include    "mbp.h"
#include    "wfs.h"
#include    "wh.h"

#include    "util.h"
#include    "common.h"


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    /* Initialize render framework for sample */
    UTIL_Init();

    /*
     * Initialize the file system.
     * As a parent, you can also specify the valid DMA channels.
     * As a child, this will not be used and is simply ignored.
     */
    FS_Init(FS_DMA_NOT_USE);

    /* LCD display start */
    GX_DispOn();
    GXS_DispOn();

    /* State transition main loop */
    for (;;)
    {
        /* If previous WFS is already running, stop it once. */
        if (WFS_GetStatus() != WFS_STATE_STOP)
        {
            WFS_End();
            (void)WH_End();
            while (WH_GetSystemState() != WH_SYSSTATE_STOP) {}
        }

        /* Start DS download play */
        ModeMultiboot();
        
        /* Initialize the WH module */ 
        ModeInitialize();
        /* Wait until parent termination is complete */
        ModeWorking();
        
        /* Set the mode and start the parent */
        ModeSelect();
        /* Wait until parent termination is complete */
        ModeWorking();
        
        /* If started, display parent screen */
        ModeParent();
        
        /* If there is an error, stop here */
        ModeError();
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
