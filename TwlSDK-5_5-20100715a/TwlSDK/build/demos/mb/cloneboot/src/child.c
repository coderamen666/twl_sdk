/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     child.c

  Copyright 2006-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
*---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"

//============================================================================
//  Prototype Declarations
//============================================================================

static void ModeConnect(void);         // Begins a connection to a parent device
static void ModeError(void);           // Error display screen
static void ModeWorking(void);         // Busy screen
static void ChildReceiveCallback(WMmpRecvBuf *data);


//============================================================================
//  Variable Definitions
//============================================================================

static s32 gFrame;                     // Frame counter

static WMBssDesc gMBParentBssDesc ATTRIBUTE_ALIGN(32);

//============================================================================
//  Function Definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         ChildMain

  Description:  Child main routine

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ChildMain(void)
{

    // Initialize screen, OS
    CommonInit();

    // This child checks to see if it is a child started from multiboot
    if (!MB_IsMultiBootChild())
    {
        OS_Panic("not found Multiboot child flag!\n");
    }

    //--------------------------------------------------------------
    // If it was started from multi-boot, it will be reset once, and communication will be disconnected.
    // The child maintains the BssDesc of the parent that booted it. Use this information to reconnect to the parent.
    //  
    // There is no particular problem with extracting the MAC address from BssDesc, then specifying that MAC address and scanning for and connecting to that parent. However, to connect to the parent directly using the stored BssDesc, it is necessary to set the parent's and child's communications size and transfer mode to match ahead of time.  
    //  
    //  
    //  
    //--------------------------------------------------------------

    /* 
     * Gets parent data for reconnecting to the parent.                   
     * The WMBssDesc used for the connection must be 32-byte aligned.
     * When reconnecting without using the parent's MAC address to rescan, make the KS/CS flags and the maximum send size match in advance for the parent and the child.
     * 
     * All of these values may be 0 if you rescan before connecting.
     */
    MB_ReadMultiBootParentBssDesc(&gMBParentBssDesc, WH_PARENT_MAX_SIZE,        // Maximum parent send size
                                  WH_CHILD_MAX_SIZE,    // Maximum child send size
                                  0,   // Key sharing flag
                                  0);  // Continuous transfer mode flag

    // When connecting without rescanning the parent, the tgid on the parent and child must match.
    //  
    // In order to avoid connections from an unrelated IPL, it is necessary to change the tgid of the parent after the parent is restarted, along with the tgid of the child.
    //  
    // In this demo, the parent and child are both incremented by 1.
    gMBParentBssDesc.gameInfo.tgid++;

    // Initialize screen
    DispInit();
    // Initialize the heap
    InitAllocateSystem();

    // Enable interrupts
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    {                                  /* FS initialization */
        static u32 fs_tablework[0x100 / 4];
        FS_Init(FS_DMA_NOT_USE);
        (void)FS_LoadTable(fs_tablework, sizeof(fs_tablework));
    }

    GInitDataShare();

    //********************************
    // Initialize wireless
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
    //********************************

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_TPrintf("MB child: Simple DataSharing demo started.\n");

    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    ReadKey();

    // Main loop
    for (gFrame = 0; TRUE; gFrame++)
    {
        // Get key input data
        ReadKey();

        // Clear the screen
        BgClear();

        // Distributes processes based on communication status
        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_CONNECT_FAIL:
            {
                // If WM_StartConnect fails, the WM internal state is invalid. Use WM_Reset to reset the state to the IDLE state.
                //  
                WH_Reset();
            }
            break;
        case WH_SYSSTATE_IDLE:
            {
                static  retry = 0;
                enum
                {
                    MAX_RETRY = 5
                };

                if (retry < MAX_RETRY)
                {
                    ModeConnect();
                    retry++;
                    break;
                }
                // If connection to parent is not possible after MAX_RETRY, display ERROR
            }
        case WH_SYSSTATE_ERROR:
            ModeError();
            break;
        case WH_SYSSTATE_BUSY:
        case WH_SYSSTATE_SCANNING:
            ModeWorking();
            break;

        case WH_SYSSTATE_CONNECTED:
        case WH_SYSSTATE_KEYSHARING:
        case WH_SYSSTATE_DATASHARING:
            {
                BgPutString(8, 1, 0x2, "Child mode");
                GStepDataShare(gFrame);
                GMain();
            }
            break;
        }

        // Display of signal strength
        {
            int     level;
            level = WH_GetLinkLevel();
            BgPrintStr(31, 23, 0xf, "%d", level);
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeConnect

  Description:  Connection start

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeConnect(void)
{
#define USE_DIRECT_CONNECT

    // When connecting directly without parent rescan.
#ifdef USE_DIRECT_CONNECT
    //********************************
    (void)WH_ChildConnect(WH_CONNECTMODE_DS_CHILD, &gMBParentBssDesc);
    // WH_ChildConnect(WH_CONNECTMODE_MP_CHILD, &gMBParentBssDesc, TRUE);
    // WH_ChildConnect(WH_CONNECTMODE_KS_CHILD, &gMBParentBssDesc, TRUE);
    //********************************
#else
    WH_SetGgid(gMBParentBssDesc.gameInfo.ggid);
    // When performing rescanning of the parent device
    //********************************
    (void)WH_ChildConnectAuto(WH_CONNECTMODE_DS_CHILD, gMBParentBssDesc.bssid,
                              gMBParentBssDesc.channel);
    // WH_ChildConnect(WH_CONNECTMODE_MP_CHILD, &gMBParentBssDesc, TRUE);
    // WH_ChildConnect(WH_CONNECTMODE_KS_CHILD, &gMBParentBssDesc, TRUE);
    //********************************
#endif
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Displays error notification screen

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    BgPrintStr(5, 10, 0x1, "======= ERROR! =======");

    if (WH_GetLastError() == WM_ERRCODE_OVER_MAX_ENTRY)
    {
        BgPrintStr(5, 13, 0xf, "OVER_MAX_ENTRY");
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  Displays working screen

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeWorking(void)
{
    BgPrintStr(9, 11, 0xf, "Now working...");

    if (IS_PAD_TRIGGER(PAD_BUTTON_START))
    {
        //********************************
        (void)WH_Finalize();
        //********************************
    }
}
