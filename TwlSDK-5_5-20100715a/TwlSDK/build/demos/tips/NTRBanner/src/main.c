/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - tips - banner - NTRBanner
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <DEMO.h>

static void InitDEMOSystem();
static void InitInteruptSystem();

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_Init();
    InitInteruptSystem();
    InitDEMOSystem();
    OS_Printf("*** start NTRBanner demo\n");
    
    for (;;)
    {
        DEMODrawText( 8, 0, "NTRBanner demo");
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  Configures display settings for console screen output.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // Initialize screen display
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  Initializes interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
}
