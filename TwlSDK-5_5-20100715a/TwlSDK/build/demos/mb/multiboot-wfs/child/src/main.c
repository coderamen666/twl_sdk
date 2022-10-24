/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - child
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-29#$
  $Rev: 8760 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    This sample program runs the WBT library so that wireless multiboot children can use a file system
    

    HOWTO:
        1. Press the B button to start communication as a child.
           When a parent with the same wbt-fs demo is found in the surrounding area, communication will automatically begin with that parent.
           
        2. For information on various features while connected, refer to the parent and child screens as well as comments in the source code.
           
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif
#include <nitro/wm.h>
#include <nitro/wbt.h>
#include <nitro/fs.h>

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

    /* Initialize the WH module */ 
    ModeInitialize();

    /* Main loop */
    for (;;)
    {
        
        /* If previous WFS is already running, stop it once. */
        if (WFS_GetStatus() != WFS_STATE_STOP)
        {
            WFS_End();
            WH_Reset();
        }

        /* Wait until the child is done */
        ModeWorking();
        
        /* Start the child */
        ModeSelect();
        /* Wait until the child is done */
        ModeWorking();
        
        /* After started, display child screen */
        ModeChild();
        
        if (WH_GetSystemState() == WH_SYSSTATE_FATAL)
        {
            /* If there is an error, stop here */
            ModeError();
        }
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
