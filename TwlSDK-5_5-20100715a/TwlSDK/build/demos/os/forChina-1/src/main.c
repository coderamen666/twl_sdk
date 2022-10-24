/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - OS - forChina-1
  File:     main.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-05-28 #$
  $Rev: 10640 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include "DEMO.h"


/*---------------------------------------------------------------------------*/
/* Constants */

// ISBN char strings for Chinese language support
// (Example) { "978-7-900381-62-7", "025-2004-017", "2005", " 065" }
static const char *isbn[4] = { "000-0-000000-00-0", "000-0000-000", "0000", "000" };


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    OS_InitChina(isbn, OS_CHINA_ISBN_DISP);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    OS_Printf("forChina-1 test started.\n");

    {
        OSOwnerInfo ownerInfo;
        int         frame;

        OS_GetOwnerInfo(&ownerInfo);

        for (frame = 0; ; ++frame)
        {
            DEMOClearString();

            DEMOPutString(4, 4, "Frame : %d", frame);
            DEMOPutString(4, 8, "LanguageCode : %d", ownerInfo.language);

            DEMO_DrawFlip();
            OS_WaitVBlankIntr();
        }
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
