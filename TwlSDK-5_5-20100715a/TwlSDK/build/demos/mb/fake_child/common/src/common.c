/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - fake_child
  File:     common.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
*---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif

#include "common.h"
#include "disp.h"


static void VBlankIntr(void);

/*
 * Common features used by this entire demo
 */
static u16 padPress;
static u16 padTrig;

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  Processing for reading keys.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ReadKey(void)
{
    u16     currData = PAD_Read();

    padTrig = (u16)(~padPress & currData);
    padPress = currData;
}

/*---------------------------------------------------------------------------*
  Name:         GetPressKey

  Description:  Gets pressed key.

  Arguments:    None.

  Returns:      Bitmap of the pressed key.
 *---------------------------------------------------------------------------*/
u16 GetPressKey(void)
{
    return padPress;
}


/*---------------------------------------------------------------------------*
  Name:         GetTrigKey

  Description:  Gets key trigger.

  Arguments:    None.

  Returns:      A bitmap of key triggers.
 *---------------------------------------------------------------------------*/
u16 GetTrigKey(void)
{
    return padTrig;
}


/*---------------------------------------------------------------------------*
  Name:         CommonInit

  Description:  Common initialization functions.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CommonInit(void)
{
    /* OS initialization */
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();

    /* GX initialization */
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    // Configure interrupts
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)GX_VBlankIntr(TRUE);

    // Read the key once as empty
    ReadKey();
}


/*---------------------------------------------------------------------------*
  Name:         InitAllocateSystem

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitAllocateSystem(void)
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
  Name:         VBlankIntr

  Description:  Gets key trigger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    DispVBlankFunc();

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
