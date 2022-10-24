/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - CARD - card-mrom
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-07-17#$
  $Rev: 10925 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/


#include <nitro.h>


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main entry point.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    FSFile      file[1];
    OS_Init();
    OS_InitTick();
    CARD_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);

    // Repeat a large read access 10 times and measure the transfer speed.
    // Use a CPU transfer because a DMA transfer has the overhead of an interrupt handler.
    FS_InitFile(file);
    if (!FS_OpenFile(file, "kart_title.32.wav"))
    {
        OS_TPanic("cannot open testfile!");
    }
    else
    {
        enum
        { BUFFER_SIZE = 1 * 1024 * 1024 };
        static u8   buf[BUFFER_SIZE];
        int         i;
        OSTick      t;
        const u16   id = (u16)OS_GetLockID();
        const u32   addr = FS_GetFileImageTop(file);

        CARD_LockRom(id);
        for (i = 0; i < 10; ++i)
        {
            t = OS_GetTick();
            CARD_ReadRom(MI_DMA_NOT_USE, (const void *)addr, buf, BUFFER_SIZE);
            t = OS_GetTick() - t;
            /* n[B/s] = 1[MB] / t[microsecond] */
            OS_Printf("[%2d] ... %10.6f[B/s]\n", i + 1,
                      1.0 * BUFFER_SIZE / OS_TicksToMicroSeconds(t));
        }
        CARD_UnlockRom(id);

        (void)FS_CloseFile(file);
        OS_TPrintf("[end]\n");
    }

    OS_Terminate();
}
