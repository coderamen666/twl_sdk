/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - OS - forKorea-1
  File:     main.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-16 #$
  $Rev: 10758 $
  $Author: ooshimay $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "DEMO.h"


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Initialization and main loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    OS_Printf("forKorea-1 test started.\n");

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
