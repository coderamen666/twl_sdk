/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wireless-all
  File:     main.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"

/*
 * This sample application implements all of the following wireless features.
 *
 * - DS Download Play clone booting
 * - Data sharing
 * - Using a file system via a wireless connection
 *
 * Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
 * 
 * The mb_child.bin program in the directory $NitroSDK/bin/ARM9-TS/Release/ is a sample providing the same features as the multiboot child in the final commercial unit. Load this binary into other development units using the same method as for sample programs, and execute them together.
 * 
 * 
 * 
 * 
 *
 */

/******************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         NitroMain / TwlMain

  Description:  Main routine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    /* Common system initialization process */
    InitCommon();

    /* Overall state transition */
    for (;;)
    {
        /* DS Download Play process, if parent */
        if (!MB_IsMultiBootChild())
        {
            ExecuteDownloadServer();
        }
        /* Wireless initialization and separate parent/child startup processes */
        if (!MB_IsMultiBootChild())
        {
#if !defined(MBP_USING_MB_EX)
            if (!WH_Initialize())
            {
                OS_Panic("WH_Initialize failed.");
            }
#endif
            WaitWHState(WH_SYSSTATE_IDLE);
            StartWirelessParent();
        }
        else
        {
            if (!WH_Initialize())
            {
                OS_Panic("WH_Initialize failed.");
            }
            WaitWHState(WH_SYSSTATE_IDLE);
            StartWirelessChild();
        }
        /* Main process for data sharing */
        ExecuteDataSharing();
        (void)WH_End();
        WaitWHState(WH_SYSSTATE_STOP);
    }

    /* Control should not arrive here */
    OS_Terminate();
}
