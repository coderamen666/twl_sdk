/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - demos - switchover
  File:     main.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro.h>
#include "common.h"


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

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

    // Based on the premise that OS_Init has been already called
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
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    /* Library initialization */
    OS_Init();
    FX_Init();
    GX_Init();

    /* Memory allocation */
    InitializeAllocateSystem();

    {
        BOOL    bSwitch = FALSE;
        for (;; bSwitch = !bSwitch)
        {
            if (bSwitch)
            {
                GraphicMain();
            }
            else
            {
                WirelessMain();
            }
            (void)OS_DisableIrq();
            (void)OS_DisableInterrupts();

            (void)GX_DisableBankForLCDC();
            (void)GX_DisableBankForTex();
            (void)GX_DisableBankForTexPltt();
            (void)GX_DisableBankForBG();
            (void)GX_DisableBankForOBJ();
            (void)GX_DisableBankForSubBG();
            (void)GX_DisableBankForSubOBJ();

        }
    }
}
