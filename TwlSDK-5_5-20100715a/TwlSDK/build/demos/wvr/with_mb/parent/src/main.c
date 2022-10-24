/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - demos - with_mb
  File:     main.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11029 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/


#include <nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "mbp.h"
#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"

#include <nitro/wvr.h>

/*
 * Sample of an application that reconnects after multibooting.
 *
 * Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required.
 * 
 * The mb_child_NITRO.srl and mb_child_TWL.srl sample programs in the $TwlSDK/bin/ARM9-TS/Rom/ directory provide the same functionality as that found in a retail unit that is operating as a multiboot child. Load these binaries on some other units in the same manner as loading a normal sample program, and run them together with this sample.
 * 
 * 
 * 
 * 
 * 
 */

/******************************************************************************/

static void WaitPressButton(void);
static void GetChannelMain(void);
static BOOL ConnectMain(u16 tgid);
static void PrintChildState(void);
static BOOL JudgeConnectableChild(WMStartParentCallback *cb);

static void StartUpCallback(void *arg, WVRResult result);


//============================================================================
//  Constant definitions
//============================================================================

/* The GGID used in this demo */
#define WH_GGID                 SDK_MAKEGGID_SYSTEM(0x21)


/* The program information that this demo downloads */
const MBGameRegistry mbGameList = {
    "/child.srl",                      // Child binary code
    (u16 *)L"DataShareDemo",           // Game name
    (u16 *)L"DataSharing demo",        // Game content description
    "/data/icon.char",                 // Icon character data
    "/data/icon.plt",                  // Icon palette data
    WH_GGID,                           // GGID
    MBP_CHILD_MAX + 1,                 // Max number of players, including parents
};



//============================================================================
//   Variable Definitions
//============================================================================

static u32 gFrame;                     // Frame counter

//-----------------------
// For maintaining communication channels
//-----------------------
static u16 sChannel = 0;
static const MBPChildInfo *sChildInfo[MBP_CHILD_MAX];

static volatile u8 startCheck;


//============================================================================
//   Function definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main routine

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    u16     tgid = 0;

    // Initialize screen and OS
    CommonInit();
    // Initialize screen
    DispInit();
    // Initialize the heap
    InitAllocateSystem();

    // Set data in WH
    WH_SetGgid(WH_GGID);

    // Enable interrupts
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    /*================================================================*/
    // Start activating the wireless library
    startCheck = 0;
    if (WVR_RESULT_OPERATING != WVR_StartUpAsync(GX_VRAM_ARM7_128_D, StartUpCallback, NULL))
    {
        OS_TPanic("WVR_StartUpAsync failed. \n");
    }
    while (!startCheck)
    {
    }
    /*================================================================*/

    DispOn();

    while (TRUE)
    {
        // Wait for the A Button to be pressed
        WaitPressButton();

        // Search for a channel with little traffic
        GetChannelMain();

        /*
         * Normally, a different TGID value is set every time the parent starts.
         * Note that you will no longer be able to reconnect to a multiboot child when starting with a different tgid unless you rescan.
         * 
         * You do not need to save the tgid if you reconnect after rescanning.
         * 
         */
        // Multiboot distribution
        if (ConnectMain(++tgid))
        {
            // Multiboot child startup is successful
            break;
        }
    }

    //--------------
    // Following multiboot, the child is reset and communication is temporarily disconnected.
    // The parent also needs to end the communication with MB_End() again in the current implementation.
    // Start the communication from the beginning while parent and children are completely disconnected.
    // 
    // At this time, the AIDs of the children will be shuffled. If necessary, save the AID and MAC address combination before multiboot and link it to a new AID when reconnecting.
    // 
    // 
    //--------------


    // Set buffer for data sharing communication
    GInitDataShare();

#if !defined(MBP_USING_MB_EX)
    if(!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
#endif

    // Set the function to determine connected children
    WH_SetJudgeAcceptFunc(JudgeConnectableChild);

    /* Main routine */
    for (gFrame = 0; TRUE; gFrame++)
    {
        OS_WaitVBlankIntr();

        ReadKey();

        BgClear();

        switch (WH_GetSystemState())
        {
        case WH_SYSSTATE_IDLE:
            /* ----------------
             * To reconnect a child to the same parent without rescanning, the parent needs to have the same tgid and channel as the child.
             * 
             * In this demo, the parent and child use the same channel as, and a tgid one greater than, the multiboot value, allowing them to connect without rescanning.
             * 
             * 
             * You don't need to use the same tgid or channel if you rescan with a specified MAC address.
             * 
             * ---------------- */
            (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, (u16)(tgid + 1), sChannel);
            break;

        case WH_SYSSTATE_CONNECTED:
        case WH_SYSSTATE_KEYSHARING:
        case WH_SYSSTATE_DATASHARING:
            {
                BgPutString(8, 1, 0x2, "Parent mode");
                GStepDataShare(gFrame);
                GMain();
            }
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         StartUpCallback

  Description:  Callback function to be notified end of async processing in wireless operation control library.
                

  Arguments:    arg: Argument specified when WVR_StartUpAsync is called. Not used.
                result: The processing results from the asynchronous function

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StartUpCallback(void *arg, WVRResult result)
{
#pragma unused( arg )

    if (result != WVR_RESULT_SUCCESS)
    {
        OS_TPanic("WVR_StartUpAsync error.[%08xh]\n", result);
    }
    startCheck = 1;
}


/*---------------------------------------------------------------------------*
  Name:         WaitPressButton

  Description:  Function that waits in loop until the A Button is pressed.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WaitPressButton(void)
{
    while (TRUE)
    {
        OS_WaitVBlankIntr();
        ReadKey();
        BgClear();
        BgSetMessage(PLTT_WHITE, " Push A Button to start   ");
        if (IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            return;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetChannelMain

  Description:  Thoroughly checks the radio wave usage to find the channel to use.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetChannelMain(void)
{

    /*-----------------------------------------------*
     * Chooses the least-used channel after thoroughly checking the channels' radio wave activity.
     * 
     * The state must be IDLE to call WM_MeasureChannel(). This cannot be run in a multiboot state because it does not stop in the IDLE state.
     * 
     * First call WM_Initialize to check radio wave activity, then end processing with WM_End, and finally run MB_Init again.
     * 
     *-----------------------------------------------*/
    if(!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }

    while (TRUE)
    {
        ReadKey();
        BgClear();
        BgSetMessage(PLTT_YELLOW, " Search Channel ");

        switch (WH_GetSystemState())
        {
            //-----------------------------------------
            // Initialization complete
        case WH_SYSSTATE_IDLE:
            (void)WH_StartMeasureChannel();
            break;
            //-----------------------------------------
            // Complete channel search
        case WH_SYSSTATE_MEASURECHANNEL:
            {
                sChannel = WH_GetMeasureChannel();
#if !defined(MBP_USING_MB_EX)
                (void)WH_End();
#else
                /* Proceed to the multiboot process while maintaining the IDLE state */
                return;
#endif
            }
            break;
            //-----------------------------------------
            // End WM
        case WH_SYSSTATE_STOP:
            /* Proceed to the multiboot process once WM_End completes */
            return;
            //-----------------------------------------
            // Busy
        case WH_SYSSTATE_BUSY:
            break;
            //-----------------------------------------
            // Error occurred
        case WH_SYSSTATE_ERROR:
            (void)WH_Reset();
            break;
            //-----------------------------------------
        default:
            OS_Panic("Illegal State\n");
        }
        OS_WaitVBlankIntr();           // Wait for completion of V-Blank interrupt
    }
}


/*---------------------------------------------------------------------------*
  Name:         ConnectMain

  Description:  Connect with multiboot.

  Arguments:    tgid: The tgid for booting as parent

  Returns:      Returns TRUE if the transfer to the child succeeds; returns FALSE if transfer fails or is cancelled.
                
 *---------------------------------------------------------------------------*/
static BOOL ConnectMain(u16 tgid)
{
    MBP_Init(mbGameList.ggid, tgid);

    while (TRUE)
    {
        ReadKey();

        BgClear();

        BgSetMessage(PLTT_YELLOW, " MB Parent ");

        switch (MBP_GetState())
        {
            //-----------------------------------------
            // Idle state
        case MBP_STATE_IDLE:
            {
                MBP_Start(&mbGameList, sChannel);
            }
            break;

            //-----------------------------------------
            // Accepting entries from children
        case MBP_STATE_ENTRY:
            {
                BgSetMessage(PLTT_WHITE, " Now Accepting            ");

                if (IS_PAD_TRIGGER(PAD_BUTTON_B))
                {
                    // Cancel multiboot with B Button
                    MBP_Cancel();
                    break;
                }

                // If there is at least one child in entry, start is possible
                if (MBP_GetChildBmp(MBP_BMPTYPE_ENTRY) ||
                    MBP_GetChildBmp(MBP_BMPTYPE_DOWNLOADING) ||
                    MBP_GetChildBmp(MBP_BMPTYPE_BOOTABLE))
                {
                    BgSetMessage(PLTT_WHITE, " Push START Button to start   ");

                    if (IS_PAD_TRIGGER(PAD_BUTTON_START))
                    {
                        // Start download
                        MBP_StartDownloadAll();
                    }
                }
            }
            break;

            //-----------------------------------------
            // Program distribution
        case MBP_STATE_DATASENDING:
            {

                // If all members have completed their downloads, start is possible
                if (MBP_IsBootableAll())
                {
                    // Start boot
                    MBP_StartRebootAll();
                }
            }
            break;

            //-----------------------------------------
            // Reboot process
        case MBP_STATE_REBOOTING:
            {
                BgSetMessage(PLTT_WHITE, " Rebooting now                ");
            }
            break;

            //-----------------------------------------
            // Reconnect process
        case MBP_STATE_COMPLETE:
            {
                // Once all members have successfully connected, multiboot processing ends and wireless restarts as a normal parent device.
                // 
                BgSetMessage(PLTT_WHITE, " Reconnecting now             ");

                OS_WaitVBlankIntr();
                return TRUE;
            }
            break;

            //-----------------------------------------
            // Error occurred
        case MBP_STATE_ERROR:
            {
                // Cancel communication
                MBP_Cancel();
            }
            break;

            //-----------------------------------------
            // Communication cancellation processing
        case MBP_STATE_CANCEL:
            // None.
            break;

            //-----------------------------------------
            // Abnormal termination of communication
        case MBP_STATE_STOP:
#ifdef MBP_USING_MB_EX
            switch (WH_GetSystemState())
            {
            case WH_SYSSTATE_IDLE:
                (void)WH_End();
                break;
            case WH_SYSSTATE_BUSY:
                break;
            case WH_SYSSTATE_STOP:
                return FALSE;
            default:
                OS_Panic("illegal state\n");
            }
#else
            return FALSE;
#endif
        }

        // Display child state
        PrintChildState();

        OS_WaitVBlankIntr();           // Wait for completion of V-Blank interrupt
    }
}


/*---------------------------------------------------------------------------*
  Name:         PrintChildState

  Description:  Displays child information on screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintChildState(void)
{
    static const char *STATE_NAME[] = {
        "NONE       ",
        "CONNECTING ",
        "REQUEST    ",
        "ENTRY      ",
        "DOWNLOADING",
        "BOOTABLE   ",
        "BOOTING    ",
    };
    enum
    {
        STATE_DISP_X = 2,
        INFO_DISP_X = 15,
        BASE_DISP_Y = 2
    };

    u16     i;

    /* Child list display */
    for (i = 1; i <= MBP_CHILD_MAX; i++)
    {
        const MBPChildInfo *childInfo;
        MBPChildState childState = MBP_GetChildState(i);
        const u8 *macAddr;

        SDK_ASSERT(childState < MBP_CHILDSTATE_NUM);

        // State display
        BgPutString(STATE_DISP_X, i + BASE_DISP_Y, PLTT_WHITE, STATE_NAME[childState]);

        // User information display
        childInfo = MBP_GetChildInfo(i);
        macAddr = MBP_GetChildMacAddress(i);

        if (macAddr != NULL)
        {
            BgPrintStr(INFO_DISP_X, i + BASE_DISP_Y, PLTT_WHITE,
                       "%02x%02x%02x%02x%02x%02x",
                       macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         JudgeConnectableChild

  Description:  Determines if the child is connectable during reconnect.

  Arguments:    cb: Information for the child attempting to connect

  Returns:      If connection is accepted, TRUE.
                If not accepted, FALSE.
 *---------------------------------------------------------------------------*/
static BOOL JudgeConnectableChild(WMStartParentCallback *cb)
{
    u16     playerNo;

    /*  Use the MAC addresses to search for 'aid' when multibooting cb->aid children */
    playerNo = MBP_GetPlayerNo(cb->macAddress);

    OS_TPrintf("MB child(%d) -> DS child(%d)\n", playerNo, cb->aid);

    if (playerNo == 0)
    {
        return FALSE;
    }

    sChildInfo[playerNo - 1] = MBP_GetChildInfo(playerNo);
    return TRUE;
}
