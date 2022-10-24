/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - exceptionDisplay-3
  File:     main.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#define MYTRACEBUFFER_SIZE 0x1000
u32     myTraceBuffer[MYTRACEBUFFER_SIZE / sizeof(u32)];

int     test1(int a);
int     test2(int a);
int     test3(int a);

void    myExceptionCallback(u32 context, void *arg);

//----------------------------------------------------------------
// test1
//
int test1(int a)
{
    return test2(a + 1);
}

//----------------------------------------------------------------
// test2
//
int test2(int a)
{
    return test3(a + 2);
}

//----------------------------------------------------------------
// test3
//
int test3(int a)
{
    //---- force to occur exception
    OS_Printf("now force to occur exception...\n");

    asm
    {
        /* *INDENT-OFF* */
        ldr      r0, =0
#ifdef SDK_ARM9
        ldr      r2, [r0,#0]
#else  //SDK_ARM9
        dcd      0x06000010

#endif //SDK_ARM9
        /* *INDENT-ON* */
    }
    OS_Printf("not occurred exception.\n");

    return a + 4;
}

//======================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void NitroMain()
{
    OS_Init();

	OS_EnableUserExceptionHandlerOnDebugger();
    OS_SetUserExceptionHandler(myExceptionCallback, (void *)0);

    DC_FlushAll();
    DC_WaitWriteBufferEmpty();


    //---- setup of function call trace
    OS_InitCallTrace(myTraceBuffer, MYTRACEBUFFER_SIZE, OS_CALLTRACE_LOG /*STACK*/);

    (void)test1(0x100);

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         myExceptionCallback

  Description:  user callback for exception

  Arguments:    context :
                arg     :

  Returns:      None
 *---------------------------------------------------------------------------*/
void myExceptionCallback(u32 context, void *arg)
{
#ifdef SDK_FINALROM
#pragma unused( context, arg )
#endif

    OS_Printf("---- USER EXCEPTION CALLBACK\n");
    OS_Printf("context=%x  arg=%x\n", context, arg);

    OS_Printf("----- CallTrace\n");
    OS_DumpCallTrace();

    OS_Printf("---- Thread LIST\n");
    OS_DumpThreadList();
}

/*====== End of main.c ======*/
