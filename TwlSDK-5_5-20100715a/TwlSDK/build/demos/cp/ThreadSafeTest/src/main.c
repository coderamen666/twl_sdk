/*---------------------------------------------------------------------------*
  Project:  TwlSDK - ThreadSafeTest - CP
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

//---------------------------------------------------------------------------
// Thread-safe test for divider and square root arithmetic unit
// HOWTO:
// 1. The state of the divider and square root unit are switched when a thread is switched, so it is thread-safe without any special processing.
//    
//
// 2. By calling the CP_SaveContext and CP_RestoreContext functions in an interrupt before and after (respectively) using the divider and square root unit, you can restore the state of these two units and use them safely.
//    
//    
//---------------------------------------------------------------------------

#include <nitro.h>


/*---------------------------------------------------------------------------*
    Prototype Declarations
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
void    proc1(void *arg);
void    proc2(void *arg);


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define A1  0x40F337
#define B1  0x197
#define A2  0x7843276F4561
#define B2  0x3208F1


#define     THREAD1_PRIO    10
#define     THREAD2_PRIO    11

#define STACK_SIZE  1024


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
u64     stack1[STACK_SIZE / sizeof(u64)];
u64     stack2[STACK_SIZE / sizeof(u64)];

OSThread thread1;
OSThread thread2;


//---------------------------------------------------------------------------
// Main Loop
//---------------------------------------------------------------------------
void NitroMain(void)
{
    u64     a;
    u32     b;
    u64     c;

    //---------------------------------------------------------------------------
    //  Initialization
    //---------------------------------------------------------------------------
    OS_Init();
    OS_InitThread();
    FX_Init();

    GX_Init();
    GX_SetPower(GX_POWER_ALL);

    GX_DispOn();

    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();

    reg_GX_DISPSTAT = 8;


    OS_Printf("Correct Answer: %x / %x = %x\n", A1, B1, A1 / B1);
    OS_Printf("Correct Answer: %llx / %x = %llx\n\n", A2, B2, A2 / B2);

    a = A2;
    b = B2;


    // Wait for V-Blank in the middle of calculation
    CP_SetDiv64_32(a, b);
    OS_Printf("Main: Start %llx / %x ...\n", a, b);

    CP_WaitDiv();
    OS_WaitVBlankIntr();               // Waiting for the end of the V-Blank interrupt

    c = (u64)CP_GetDivResult64();

    OS_Printf("Main: Get %llx / %x = %llx\n", a, b, c);


    // Start thread
    OS_Printf("----------------------------\n");
    OS_Printf("Create Thread\n");

    OS_CreateThread(&thread1, proc1, (void *)0x111, stack1 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD1_PRIO);
    OS_CreateThread(&thread2, proc2, (void *)0x222, stack2 + STACK_SIZE / sizeof(u64), STACK_SIZE,
                    THREAD2_PRIO);

    OS_WakeupThreadDirect(&thread1);
    OS_WakeupThreadDirect(&thread2);

    OS_Printf("Idle\n");
    OS_WakeupThreadDirect(&thread2);


    // Quit
    OS_Printf("==== Finish sample.\n");
    OS_Terminate();

    while (1)
    {

        OS_WaitVBlankIntr();           // Waiting for the end of the V-Blank interrupt

    }
}


//---------------------------------------------------------------------------
// V-Blank interrupt function:
//---------------------------------------------------------------------------
static void VBlankIntr(void)
{
    u32     a, b, c;
    CPContext context;

    // Save context of arithmetic unit
    CP_SaveContext(&context);

    a = A1;
    b = B1;

    CP_SetDiv32_32(a, b);
    OS_Printf("VBlank: Start %x / %x ...\n", a, b);
    CP_WaitDiv();
    c = (u32)CP_GetDivResult32();

    OS_Printf("VBlank: Get %x / %x = %x\n", a, b, c);

    // Restore context of arithmetic unit
    CP_RestoreContext(&context);

    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt


}


//--------------------------------------------------------------------------------
//    proc1
//
void proc1(void *arg)
{
#pragma unused(arg)
    u64     a;
    u32     b;
    u64     c;

    while (1)
    {
        OS_Printf("Sleep Thread1\n");
        OS_SleepThread(NULL);
        OS_Printf("Wake Thread1 (Priority %d)\n", THREAD1_PRIO);

        a = A2;
        b = B2;

        CP_SetDiv64_32(a, b);
        OS_Printf("Thread1: Start %llx / %x ...\n", a, b);
        CP_WaitDiv();
        c = (u64)CP_GetDivResult64();

        OS_Printf("Thread1: Get %llx / %x = %x\n", a, b, c);
    }
}

//--------------------------------------------------------------------------------
//    proc2
//
void proc2(void *arg)
{
#pragma unused(arg)
    u32     a, b, c;

    while (1)
    {
        OS_Printf("Sleep Thread2\n");
        OS_SleepThread(NULL);
        OS_Printf("WakeThread2: (Priority %d)\n", THREAD2_PRIO);

        a = A1;
        b = B1;

        CP_SetDiv32_32(a, b);
        OS_Printf("Thread2: Start %x / %x ...\n", a, b);
        CP_WaitDiv();

        // Call thread 1 in the middle of calculation
        OS_WakeupThreadDirect(&thread1);
        c = (u32)CP_GetDivResult32();
        OS_Printf("Thread2: Get %x / %x = %x\n", a, b, c);
    }
}
