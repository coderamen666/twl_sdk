/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - functionCost-1
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

void    test1(void);
void    test2(void);
void    test3(void);

#define MYCOSTBUFFER_SIZE 0x3000
#define MYSTATBUFFER_SIZE 0x300

u32     myCostBuffer[MYCOSTBUFFER_SIZE / sizeof(u32)];
u32     myStatBuffer[MYSTATBUFFER_SIZE / sizeof(u32)];
//================================================================

//----------------------------------------------------------------
// test1
//
void test1(void)
{
    test2();
}

//----------------------------------------------------------------
// test2
//
void test2(void)
{
    test3();
}

//----------------------------------------------------------------
// test3
//
void test3(void)
{
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
    OS_Init();
    OS_InitTick();
    (void)OS_EnableIrq();

    //---- init functionCost
    OS_InitFunctionCost(&myCostBuffer, MYCOSTBUFFER_SIZE);
    OS_InitStatistics(&myStatBuffer, MYSTATBUFFER_SIZE);

    test2();
    test1();
    test2();
    test3();

    //---- calculate cost
    OS_CalcStatistics(&myStatBuffer);

    //---- display functionCost
    OS_DumpStatistics(&myStatBuffer);

    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

/*====== End of main.c ======*/
