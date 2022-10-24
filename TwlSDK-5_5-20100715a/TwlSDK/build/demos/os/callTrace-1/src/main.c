/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - callTrace-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

int     test1(int a);
int     test2(int a);
int     test3(int a);
void    test4(int a);
void    test5(int a);

#define MYTRACEBUFFER_SIZE 0x300
u32     myTraceBuffer[MYTRACEBUFFER_SIZE / sizeof(u32)];

SDK_WEAK_SYMBOL asm void __PROFILE_ENTRY( void ){ bx lr }
SDK_WEAK_SYMBOL asm void __PROFILE_EXIT( void ){ bx lr }

//================================================================

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
    OS_DumpCallTrace();
    return a + 4;
}

//----------------------------------------------------------------
// test4
//
void test4(int a)
{
    test5(a + 3);
    return;
}

//----------------------------------------------------------------
// test5
//
void test5(int a)
{
#pragma unused( a )
    return;
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#pragma profile off
void NitroMain(void)
{
    int     n;

    OS_Init();

    OS_Printf("---------------- stack mode demo of callTrace.\n");
    OS_InitCallTrace(&myTraceBuffer, MYTRACEBUFFER_SIZE, OS_CALLTRACE_STACK);
    OS_Printf("buffer has %d lines to record callTrace.\n",
              OS_CalcCallTraceLines(MYTRACEBUFFER_SIZE));

    OS_Printf("---- call test3.\n");
    n = test3(1);
    OS_Printf("---- call test1.\n");
    n = test1(0x100);

    OS_Printf("---------------- log mode demo of callTrace.\n");
    OS_InitCallTrace(&myTraceBuffer, MYTRACEBUFFER_SIZE, OS_CALLTRACE_LOG);

    OS_Printf("---- call test4.\n");
    test4(1);
    OS_Printf("---- call test4.\n");
    test4(0x100);

    OS_DumpCallTrace();
    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
