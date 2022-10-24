/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - spinwait-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-01#$
  $Rev: 5205 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define TicksToCPUCycles( tick )        ((tick)*64*2)

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OSTick  begin, end;
    u32     n;

    OS_Init();
    OS_InitTick();

    n = 1000;
    for (n = 1000; n <= 1000000; n *= 2)
    {
        begin = OS_GetTick();
        OS_SpinWait(n);
        end = OS_GetTick();

        OS_Printf("OS_SpinWait(%d)  Wait=%lld cycles\n", n, TicksToCPUCycles((u64)end - (u64)begin));
    }

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
