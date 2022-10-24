/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - functionCost-3
  File:     main.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

void    test1(void);
void    test2(void);
void    test3(void);
void    test_sleep(void);

#define MYCOSTBUFFER_SIZE 0x3000
#define MYSTATBUFFER_SIZE 0x300

u32     myCostBuffer[MYCOSTBUFFER_SIZE / sizeof(u32)];
u32     myCostBuffer1[MYCOSTBUFFER_SIZE / sizeof(u32)];
u32     myCostBuffer2[MYCOSTBUFFER_SIZE / sizeof(u32)];

u32     myStatBuffer[MYSTATBUFFER_SIZE / sizeof(u32)];

//---- thread
#define STACK_SIZE  1024

void    proc1(void *arg);
void    proc2(void *arg);

OSThread thread1;
OSThread thread2;

u64     stack1[STACK_SIZE / sizeof(u64)];
u64     stack2[STACK_SIZE / sizeof(u64)];

#define     THREAD1_PRIO    10
#define     THREAD2_PRIO    11

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
    OS_InitTick();
    (void)OS_EnableIrq();

    //---- init thread
    //OS_InitThread(); <- maybe done in OS_Init() already.
    OS_CreateThread(&thread1, proc1, (void *)0x111, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, (void *)0x222, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD2_PRIO);
    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    //---- init functionCost
    OS_InitFunctionCost(&myCostBuffer, MYCOSTBUFFER_SIZE);
    OS_InitStatistics(&myStatBuffer, MYSTATBUFFER_SIZE);

    for (n = 0; n < 100; n++)
    {
        test3();
        OS_WakeupThreadDirect(&thread2);
    }

    //---- calculate cost
    OS_CalcStatistics(&myStatBuffer);

    //---- display all thread functionCost
    OS_DumpStatistics(&myStatBuffer);


    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

#pragma profile reset

//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused( arg )

    //---- init functionCost
    OS_InitFunctionCost(&myCostBuffer1, MYCOSTBUFFER_SIZE);

    while (1)
    {
        test1();
        test_sleep();

        //---- calculate cost
        OS_CalcStatistics(&myStatBuffer);
    }
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#pragma unused( arg )

    //---- init functionCost
    OS_InitFunctionCost(&myCostBuffer2, MYCOSTBUFFER_SIZE);

    while (1)
    {
        test2();
        OS_SleepThread(NULL);
        OS_WakeupThreadDirect(&thread1);

        //---- calculate cost
        OS_CalcStatistics(&myStatBuffer);
    }
}

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

//----------------------------------------------------------------
// test_sleep
//
void test_sleep(void)
{
    OS_SleepThread(NULL);
}

/*====== End of main.c ======*/
