/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-Model
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


#include	<nitro.h>
#include <nitro/wm.h>
#include <nitro/mb.h>

#include "mbp.h"
#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"

/*
 * Sample application that reconnects after multiboot
 *
 * Because the MB library samples use the multiboot functionality, multiple development units with the same communications environment (wired or wireless) are required
 * 
 * The mb_child.bin program in the directory $NitroSDK/bin/ARM9-TS/Release/ is a sample providing the same features as the multiboot child in the final commercial unit. Load this binary into other development units using the same method as for sample programs, and execute them together.
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
static void OnPreSendMBVolat(u32 ggid);


//============================================================================
//  Constant Definitions
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
    MBP_CHILD_MAX + 1,                 // Max. number of players, including parents
};



//============================================================================
//   Variable Definitions
//============================================================================

static s32 gFrame;                     // Frame counter

//-----------------------
// For keeping communication channels
//-----------------------
static u16 sChannel = 0;
static const MBPChildInfo *sChildInfo[MBP_CHILD_MAX];


//============================================================================
//   Function Definitions
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

    // Initialize screen, OS
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
    
    {                                  /* FS initialization */
        static u32 fs_tablework[0x100 / 4];
        FS_Init(FS_DMA_NOT_USE);
        (void)FS_LoadTable(fs_tablework, sizeof(fs_tablework));
    }

    DispOn();

    /*
     * If the parent has the same TGID, the child does not update the parent information during DS Download Play.
     * Therefore, it is necessary to consider assigning a different TGID value to a parent each time it is started during DS Download Play.
     * 
     * Normally, you can use the WM_GetNextTgid function to get a relatively unique value based on the RTC.
     * 
     */
    tgid = WM_GetNextTgid();

    while (TRUE)
    {
        // Wait for A button to be pressed
        WaitPressButton();

        // Search for a channel with little traffic
        GetChannelMain();

        /*
         * Normally, a different tgid value is set every time the parent starts.
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
    // The parent also needs to end the communication with MB_End again in the current implementation.
    // Start the communication from the beginning while parent and children are completely disconnected.
    // 
    // At this time, the AIDs of the children will be shuffled. If necessary, save the AID and MAC address combination before multiboot, and link it to a new AID when reconnecting.
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
             * To reconnect a child to the same parent without rescanning, the parent needs to have the same tgid and channel as the child
             * 
             * In this demo, the parent and child use the same channel as, and a tgid one greater than, the multiboot value, allowing them to connect without rescanning
             * 
             * 
             * You don't need to use the same tgid or channel if you rescan with a specified MAC address
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
  Name:         WaitPressButton

  Description:  Function that waits in loop until A button is pressed.

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
     * The state must be IDLE to call WM_MeasureChannel. This cannot be run in a multiboot state because it does not stop in the IDLE state.
     * 
     * First call WM_Initialize to check radio wave activity, then end processing with WM_End. Finally, run MB_Init again.
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

  Returns:      If transfer to the child is successful, TRUE.
                If transfer fails or is canceled, FALSE.
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
                /* Transmission test of user-defined data */
                MB_SetSendVolatCallback(OnPreSendMBVolat, MB_SEND_VOLAT_CALLBACK_TIMMING_BEFORE);
            }
            break;

            //-----------------------------------------
            // Accepting entry from the child
        case MBP_STATE_ENTRY:
            {
                BgSetMessage(PLTT_WHITE, " Now Accepting            ");

                if (IS_PAD_TRIGGER(PAD_BUTTON_B))
                {
                    // Cancel multiboot with B button
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
            // Program distribution process
        case MBP_STATE_DATASENDING:
            {

                // If everyone has completed downloading, start is possible
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
                // Once all members have successfully connected, multiboot processing ends. Restarts wireless communication as a normal parent.
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

    /*  Search the MAC address for the AID when multibooting the child of cb->aid */
    playerNo = MBP_GetPlayerNo(cb->macAddress);

    OS_TPrintf("MB child(%d) -> DS child(%d)\n", playerNo, cb->aid);

    if (playerNo == 0)
    {
        return FALSE;
    }

    sChildInfo[playerNo - 1] = MBP_GetChildInfo(playerNo);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         OnPreSendMBVolat

  Description:  Callback function used for notification before a parent sends an MB beacon.

  Arguments:    ggid: GGID of game information to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void OnPreSendMBVolat(u32 ggid)
{
    /*
     * When multiple games are registered and the configuration value is switched for each game, use the ggid argument to determine the game information, as follows.
     * 
     */
    if (ggid == mbGameList.ggid)
    {
        /*
         * Up to 8 bytes of user-defined data can be sent.
         * This example sends incremental 64-bit values.
         * Note that the child may not always be able to immediately receive dynamically updated data.
         * 
         */
        static u64 count ATTRIBUTE_ALIGN(8);
        SDK_COMPILER_ASSERT(sizeof(count) <= MB_USER_VOLAT_DATA_SIZE);
        SDK_COMPILER_ASSERT(MB_USER_VOLAT_DATA_SIZE == 8);
        ++count;
        MB_SetUserVolatData(ggid, (u8 *)&count, sizeof(count));
    }
}
