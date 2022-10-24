/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - exceptionDisplay-1
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

extern  _u32_div_f(void);
void    myExceptionCallback(u32 context, void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();

	OS_EnableUserExceptionHandlerOnDebugger();
    OS_SetUserExceptionHandler(myExceptionCallback, (void *)0);

    //---- The reason we're flushing the cache here is because the protection unit changes when an exception is generated. We want to reliably write addresses, etc. related to the exception.
    //     
    //     
    DC_FlushAll();
    DC_WaitWriteBufferEmpty();


    //---- normal division
	OS_Printf("now calculate 120 / 6\n" );
    {
        int     a = 120;
        volatile int b = 6;
        volatile int c = a / b;
    }
    OS_Printf("not occurred exception.\n");

    //---- divisor is 0
	OS_Printf("now calculate 240 / 0\n" );
    {
        int     a = 240;
        volatile b = 0;
        volatile int c = a / b;
    }
    OS_Printf("not occurred exception.\n");

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         _u32_div_f

  Description:  Integer division
                If divisor is 0, access 0 address.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
SDK_FORCE_EXPORT static asm s32 _s32_div_f(void)
{
       stmfd       sp!,{r4,lr}

       //---- Compare divisor to 0
       cmp         r1,#0
       bne         _1

       //---- Force exception to occur
       ldr         r11, [r1,#0]
_1:
       eor         r4,r0,r1
       and         r4,r4,#-2147483648
       cmp         r0,#0
       rsblt       r0,r0,#0
       addlt       r4,r4,#1
       cmp         r1,#0
       rsblt       r1,r1,#0
       bl          _u32_div_f
       ands        r3,r4,#-2147483648
       rsbne       r0,r0,#0
       ands        r3,r4,#1
       rsbne       r1,r1,#0
       ldmfd       sp!,{r4,lr}
       bx          lr
}
#include <nitro/codereset.h>

/*---------------------------------------------------------------------------*
  Name:         myExceptionCallback

  Description:  User callback for exception.

  Arguments:    context:
                arg:

  Returns:      None.
 *---------------------------------------------------------------------------*/
void myExceptionCallback(u32 context, void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( context, arg )
#endif

    OS_Printf("---- USER EXCEPTION CALLBACK\n");
    OS_Printf("context=%x  arg=%x\n", context, arg);

    OS_Printf("---- Thread LIST\n");
    OS_DumpThreadList();
}

/*====== End of main.c ======*/
