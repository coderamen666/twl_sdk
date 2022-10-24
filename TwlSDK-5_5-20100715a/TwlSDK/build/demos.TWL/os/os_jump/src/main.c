/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos.TWL - os - os_jump
  File:     main.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <stdlib.h>

#include "screen.h"
#include "DEMO.h"

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define KEY_REPEAT_START    25  // Number of frames until key repeat starts
#define KEY_REPEAT_SPAN     10  // Number of frames between key repeats

/*---------------------------------------------------------------------------*
    Structure Definitions
 *---------------------------------------------------------------------------*/

// Key input data
typedef struct KeyInfo
{
    u16 cnt;    // Unprocessed input value
    u16 trg;    // Push trigger input
    u16 up;     // Release trigger input
    u16 rep;    // Press and hold repeat input
} KeyInfo;

// Key input
static KeyInfo  gKey;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/

static void VBlankIntr(void);
static void InitInterrupts(void);
static void InitHeap(void);

static void ReadKey(KeyInfo* pKey);

/*---------------------------------------------------------------------------*/

void TwlMain(void)
{
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();

    // When in NITRO mode, stopped by Panic
    DEMOCheckRunOnTWL();

    GX_Init();
    GX_DispOff();
    GXS_DispOff();

    InitHeap();
    InitScreen();
    InitInterrupts();

    GX_DispOn();
    GXS_DispOn();

    ClearScreen();

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    ReadKey(&gKey);

    while(TRUE)
    {
        // Get key input data
        ReadKey(&gKey);

        // Clear the main screen
        ClearScreen();
        
        PutMainScreen(0, 0, 0xff, "OS_IsRebooted = %s", OS_IsRebooted()? "TRUE":"FALSE");
        
        PutSubScreen(0, 0, 0xff, "APPLICATION JUMP DEMO");
        PutSubScreen(0, 1, 0xff, "  A: OS_JumpToSystemMenu");
        PutSubScreen(0, 2, 0xff, "  B: OS_RebootSystem");
        PutSubScreen(0, 3, 0xff, "  X: OS_JumpToEULAViewer");
        PutSubScreen(0, 4, 0xff, "  Y: OS_JumpToInternetSetting");
        PutSubScreen(0, 5, 0xff, "  R: OS_JumpToWirelessSetting");
        
        if (gKey.trg & PAD_BUTTON_A)
        {
            if (FALSE == OS_JumpToSystemMenu() )
            {
                //Jump failure
                OS_Panic("OS_JumpToSystemMenu() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_B)
        {
            if (FALSE == OS_RebootSystem() )
            {
                //Jump failure
                OS_Panic("OS_RebootSystem() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_X)
        {
            if (FALSE == OS_JumpToEULAViewer() )
            {
                //Jump failure
                OS_Panic("OS_JumpToEULAViewer() is Failed");
            }
        }

        if (gKey.trg & PAD_BUTTON_Y)
        {
            if (FALSE == OS_JumpToInternetSetting() )
            {
                //Jump failure
                OS_Panic("OS_JumpToInternetSetting() is Failed");
            }
        }
        if (gKey.trg & PAD_BUTTON_R)
        {
            if (FALSE == OS_JumpToWirelessSetting() )
            {
                //Jump failure
                OS_Panic("OS_JumpToWirelessSetting() is Failed");
            }
        }
        // Wait for V-Blank (this supports threading)
        OS_WaitVBlankIntr();
    }

    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  V-Blank interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    // Updates text display
    UpdateScreen();

    // Sets the IRQ check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitInterrupts

  Description:  Initializes the interrupt settings.
                Allows V-Blank interrupts and configures the interrupt handler.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInterrupts(void)
{
    // V-Blank interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // Allow interrupts
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitHeap

  Description:  Initializes the memory allocation system within the main memory arena.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitHeap(void)
{
    void*           tempLo;
    OSHeapHandle    hh;

    // Creates a heap in the main memory arena
    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        // Abnormal end when heap creation fails
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         ReadKey

  Description:  Gets key input data and edits the input data structure.
                Detects push, release, and push and hold repeat triggers.

  Arguments:    pKey: Key input structure to be changed

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadKey(KeyInfo* pKey)
{
    static u16  repeat_count[12];
    int         i;
    u16         r;

    r = PAD_Read();
    pKey->trg = 0x0000;
    pKey->up = 0x0000;
    pKey->rep = 0x0000;

    for (i = 0; i < 12; i++)
    {
        if (r & (0x0001 << i))
        {
            if (!(pKey->cnt & (0x0001 << i)))
            {
                pKey->trg |= (0x0001 << i);     // Press trigger
                repeat_count[i] = 1;
            }
            else
            {
                if (repeat_count[i] > KEY_REPEAT_START)
                {
                    pKey->rep |= (0x0001 << i); // Press-and-hold repeat
                    repeat_count[i] = (u16) (KEY_REPEAT_START - KEY_REPEAT_SPAN);
                }
                else
                {
                    repeat_count[i]++;
                }
            }
        }
        else
        {
            if (pKey->cnt & (0x0001 << i))
            {
                pKey->up |= (0x0001 << i);      // Release trigger
            }
        }
    }

    pKey->cnt = r;  // Unprocessed key input
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
