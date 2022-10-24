/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - fake_child
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

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"


#define     DEBUG_PRINT

/*
 * This sample makes an entry as a child with demos/mb/multiboot-Model and participates in network communication with other DS Download Play children.
 * The target application is configured by the MB_FakeScanParent function's GGID argument.
 * 
 * 
 */


enum
{
    MODE_TITLE,
    MODE_START_SCAN,
    MODE_SCANNING,
    MODE_ENTRY,
    MODE_RECONNECT,
    MODE_GMAIN,
    MODE_ERROR,
    MODE_CANCEL
};

typedef struct
{
    MBUserInfo info;
    u16     valid;
    u8      maxPlayerNum;              // Max number of players
    u8      nowPlayerNum;              // Current number of players
    u8      macAddr[6];                // MAC address
}
ParentInfo;

#define WH_GGID (0x003FFF00 | (0x21))

static void PrintError(MBFakeScanErrorCallback *arg);

static void ModeTitle(void);
static void ModeStartScan(void);
static void ModeScanning(void);
static void ModeEntry(void);
static void ModeError(void);
static void ModeReconnect(void);
static void ModeGMain(void);
static void ModeCancel(void);

static void NotifyScanParent(u16 type, void *arg);
static void MBStateCallback(u32 status, void *arg);


// Parent info list
static ParentInfo parentInfo[MB_GAME_INFO_RECV_LIST_NUM];
static ParentInfo parentInfoBuf[MB_GAME_INFO_RECV_LIST_NUM];

static WMBssDesc sParentBssDesc;       // BssDesc of entered parent

static u8 *mbfBuf = NULL;
static u8 *wmBuf = NULL;


static s32 gFrame;                     // Frame counter

static u16 sMode;

enum
{
    WSTATE_STOP,                       // Stop state
    WSTATE_INITIALIZE_BUSY,            // Initializing wireless
    WSTATE_IDLE,                       // Idle state
    WSTATE_ERROR,                      // Error before starting/after stopping fake child
    WSTATE_FAKE_SCAN,                  // Scan state
    WSTATE_FAKE_END_SCAN,              // End scan
    WSTATE_FAKE_ENTRY,                 // Entry to parent
    WSTATE_FAKE_BOOT_READY,            // Entry to DS download play is complete
    WSTATE_FAKE_BOOT_END_BUSY,         // DS download processing is now finishing
    WSTATE_FAKE_BOOT_END,
    WSTATE_FAKE_CANCEL_BUSY,           // DS download canceling
    WSTATE_FAKE_ERROR                  // Error
};
static u16 sWirelessState = WSTATE_STOP;

static u16 sConnectIndex;              // Index of parent devices to connect

/******************************************************************************/
/* Functions */

#ifdef DEBUG_PRINT
static void ChangeWirelessState(u16 wstate)
{
    static const char *WSTATE_NAME[] = {
        "WSTATE_STOP",
        "WSTATE_INITIALIZE_BUSY",
        "WSTATE_IDLE",
        "WSTATE_ERROR",
        "WSTATE_FAKE_SCAN",
        "WSTATE_FAKE_END_SCAN",
        "WSTATE_FAKE_ENTRY",
        "WSTATE_FAKE_BOOT_READY",
        "WSTATE_FAKE_BOOT_END_BUSY",
        "WSTATE_FAKE_BOOT_END",
        "WSTATE_FAKE_CANCEL_BUSY",
        "WSTATE_FAKE_ERROR"
    };

    if (sWirelessState != wstate)
    {
        OS_TPrintf("change sWirelessState %s -> %s\n", WSTATE_NAME[sWirelessState],
                   WSTATE_NAME[wstate]);
    }
#else
static inline void ChangeWirelessState(u16 wstate)
{
#endif
    sWirelessState = wstate;
}

/* V-Blank interrupt process */
static void VBlankIntr(void)
{
    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}


/* Main */
#ifdef SDK_TWL
void TwlMain()
#else
void NitroMain()
#endif
{
    // Initialize system
    CommonInit();
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


    // Clear buffer for data sharing
    GInitDataShare();

    // To title screen
    sMode = MODE_TITLE;

    // LCD display start
    GX_DispOn();
    GXS_DispOn();

    // Debug string output
    OS_TPrintf("MB fake child: Simple DataSharing demo started.\n");
    // Empty call for getting key input data (strategy for pressing A Button in the IPL)
    ReadKey();

    /* Main loop */
    for (gFrame = 0; TRUE; gFrame++)
    {
        // Get key input data
        ReadKey();

        // Clear the screen
        BgClear();

        switch (sMode)
        {
        case MODE_TITLE:
            // Display title screen
            ModeTitle();
            break;
        case MODE_START_SCAN:
            // Start scan
            ModeStartScan();
            break;
        case MODE_SCANNING:
            // Scanning MB parent
            ModeScanning();
            break;
        case MODE_ENTRY:
            // Entry to MB parent
            ModeEntry();
            break;
        case MODE_RECONNECT:
            // Reconnect to MB parent
            ModeReconnect();
            break;
        case MODE_GMAIN:
            // Game main
            ModeGMain();
            break;
        case MODE_ERROR:
            // Error processing
            ModeError();
            break;
        case MODE_CANCEL:
            // Canceling
            ModeCancel();
            break;
        }

        OS_WaitVBlankIntr();
    }
}



/* ----------------------------------------------------------------------
  Name:        ModeTitle
  
  Description: Title screen
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeTitle(void)
{
    BgSetMessage(PLTT_WHITE, " Press Button A to start   ");

    ChangeWirelessState(WSTATE_STOP);

    // When A Button is pressed, set wireless to ON and enter MB parent search mode
    if (IS_PAD_TRIGGER(PAD_BUTTON_A))
    {
        sMode = MODE_START_SCAN;
    }
}

/* ----------------------------------------------------------------------
  Name:        ModeStartScan
  
  Description: Starts parent scan.
  
  Arguments:   None.
               
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeStartScan(void)
{
    BgSetMessage(PLTT_WHITE, "Now working....         ");

    switch (sWirelessState)
    {
        //--------------------------
        // Start wireless communication
    case WSTATE_STOP:
        if (!WH_Initialize())
        {
            ChangeWirelessState(WSTATE_ERROR);
            sMode = MODE_ERROR;
            return;
        }
        ChangeWirelessState(WSTATE_INITIALIZE_BUSY);
        break;

        //--------------------------
        // Initializing wireless
    case WSTATE_INITIALIZE_BUSY:
        if (WH_GetSystemState() == WH_SYSSTATE_BUSY)
        {
            BgSetMessage(PLTT_WHITE, "Now working....         ");
            return;
        }
        if (WH_GetSystemState() != WH_SYSSTATE_IDLE)
        {
            ChangeWirelessState(WSTATE_ERROR);
            sMode = MODE_ERROR;
            return;
        }
        ChangeWirelessState(WSTATE_IDLE);
        // Don't break

        //--------------------------
        // Start parent scan
    case WSTATE_IDLE:
        {
            // Set user info
            MBUserInfo userInfo;

            OSOwnerInfo info;

            OS_GetOwnerInfo(&info);
            userInfo.favoriteColor = info.favoriteColor;
            userInfo.nameLength = (u8)info.nickNameLength;
            MI_CpuCopy8(info.nickName, userInfo.name, (u32)(userInfo.nameLength * 2));

            // Normally with MB child, playerNo will be 0, but by setting a value it can be passed to a parent as a parameter.
            userInfo.playerNo = 7;

            // Allocate work buffer for multiboot entries
            mbfBuf = (u8 *)OS_Alloc(MB_FakeGetWorkSize());
            MB_FakeInit(mbfBuf, &userInfo);
            // Set the state transition notification callback function
            MB_FakeSetCStateCallback(MBStateCallback);

            // Clear parent list
            MI_CpuClear8(parentInfo, sizeof(parentInfo));

            // Wireless state changes to scanning
            ChangeWirelessState(WSTATE_FAKE_SCAN);

            // Set GGID of the parent to scan and the notification callback function; start scanning
            MB_FakeStartScanParent(NotifyScanParent, WH_GGID);

            // Transition to scan mode
            sMode = MODE_SCANNING;
        }
        break;
    default:
        OS_TPanic("illegal state %d", sWirelessState);
        break;
    }
}


/* ----------------------------------------------------------------------
  Name:        ModeScanning
  
  Description: Parent scanning.
  
  Arguments:   None.
               
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeScanning(void)
{
    enum
    { STR_X = 4, STR_Y = 4, NUM_STR_X = 24 };
    static u16 cursol_idx = 0;

    u16     i;

    // If error occurs, terminate
    if (sWirelessState == WSTATE_FAKE_ERROR)
    {
        sMode = MODE_ERROR;
        return;
    }

    // The parent information is copied to the temporary buffer before use, so that it won't be changed by an interrupt during rendering.
    //  
    {
        OSIntrMode enabled = OS_DisableInterrupts();
        MI_CpuCopy8(parentInfo, parentInfoBuf, sizeof(parentInfo));
        (void)OS_RestoreInterrupts(enabled);
    }

    // Move cursor
    if (IS_PAD_TRIGGER(PAD_KEY_UP))
    {
        cursol_idx = (u16)((cursol_idx - 1) & 0xF);
    }
    else if (IS_PAD_TRIGGER(PAD_KEY_DOWN))
    {
        cursol_idx = (u16)((cursol_idx + 1) & 0xF);
    }

    // Display parent info
    BgPutString(10, 1, PLTT_RED, "Select Parent");

    BgPutString(STR_X - 2, STR_Y + cursol_idx, PLTT_WHITE, ">");
    for (i = 0; i < MB_GAME_INFO_RECV_LIST_NUM; i++)
    {
        if (parentInfoBuf[i].valid)
        {
            BgPrintStr(STR_X, STR_Y + i, PLTT_GREEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                       parentInfoBuf[i].macAddr[0], parentInfoBuf[i].macAddr[1],
                       parentInfoBuf[i].macAddr[2], parentInfoBuf[i].macAddr[3],
                       parentInfoBuf[i].macAddr[4], parentInfoBuf[i].macAddr[5]);
            BgPrintStr(NUM_STR_X, STR_Y + i, PLTT_GREEN, "%d/%d",
                       parentInfoBuf[i].nowPlayerNum, parentInfoBuf[i].maxPlayerNum);
        }
        else
        {
            BgPutString(STR_X, STR_Y + i, PLTT_WHITE, "Searching...");
        }
    }

    // Cancel communication
    if (IS_PAD_TRIGGER(PAD_BUTTON_B))
    {
        ChangeWirelessState(WSTATE_FAKE_CANCEL_BUSY);
        sMode = MODE_CANCEL;
        MB_FakeEnd();
        return;
    }

    // Select the parent to connect to
    if (IS_PAD_TRIGGER(PAD_BUTTON_A))
    {
        if (!parentInfoBuf[cursol_idx].valid)
        {
            return;
        }

        // Get info from the parent to connect to
        sConnectIndex = cursol_idx;
        if (!MB_FakeReadParentBssDesc
            (sConnectIndex, &sParentBssDesc, WH_PARENT_MAX_SIZE, WH_CHILD_MAX_SIZE, FALSE, FALSE))
        {
            // If the parent info is not valid, continue scan
            return;
        }
        sMode = MODE_ENTRY;
    }
}


/* ----------------------------------------------------------------------
  Name:        ModeEntry
  
  Description: Entry to parent device.
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeEntry(void)
{
    BgSetMessage(PLTT_WHITE, "Now Entrying...        ");

    switch (sWirelessState)
    {
        //--------------------------
        // If scanning
    case WSTATE_FAKE_SCAN:
        {
            // Terminate scan
            MB_FakeEndScan();
            ChangeWirelessState(WSTATE_FAKE_END_SCAN);
        }
        break;

        //--------------------------
        // Terminating scan
    case WSTATE_FAKE_END_SCAN:
        break;

        //--------------------------
        // Start wireless communication
    case WSTATE_IDLE:
        {
            if (!MB_FakeEntryToParent(sConnectIndex))
            {
                // If specified index is not valid, error
                OS_TPrintf("ERR: Illegal Parent index\n");
                ChangeWirelessState(WSTATE_FAKE_ERROR);
                sMode = MODE_ERROR;
                return;
            }
            ChangeWirelessState(WSTATE_FAKE_ENTRY);
        }
        break;

        //--------------------------
        // Undergoing entry
    case WSTATE_FAKE_ENTRY:
        // If entry has not been completed, cancel with B Button
        if (IS_PAD_TRIGGER(PAD_BUTTON_B))
        {
            ChangeWirelessState(WSTATE_FAKE_CANCEL_BUSY);
            sMode = MODE_CANCEL;
            MB_FakeEnd();
            return;
        }
        break;

        //--------------------------
        // Complete entry processing
    case WSTATE_FAKE_BOOT_READY:
        {
            // Ending processing for entry processing
            ChangeWirelessState(WSTATE_FAKE_BOOT_END_BUSY);
            MB_FakeEnd();
        }
        break;

        //--------------------------
        // Wait for finish
    case WSTATE_FAKE_BOOT_END_BUSY:
        break;

        //--------------------------
        // Complete processing for fake child
    case WSTATE_FAKE_BOOT_END:
        {
            // Align with parent and increment tgid
            sParentBssDesc.gameInfo.tgid++;
            // Start reconnection
            ChangeWirelessState(WSTATE_IDLE);
            sMode = MODE_RECONNECT;
        }
        break;

        //--------------------------
        // Error complete
    case WSTATE_FAKE_ERROR:
        sMode = MODE_ERROR;
        break;

    default:
        OS_TPanic("illegal state %d\n", sWirelessState);
    }
}


/* ----------------------------------------------------------------------
  Name:        ModeReconnect
  
  Description: Title screen.
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeReconnect(void)
{
    BgSetMessage(PLTT_WHITE, "Connecting to Parent...        ");

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
            (void)WH_ChildConnect(WH_CONNECTMODE_DS_CHILD, &sParentBssDesc);
        }
        break;
    case WH_SYSSTATE_ERROR:
        ChangeWirelessState(WSTATE_ERROR);
        sMode = MODE_ERROR;
        break;
    case WH_SYSSTATE_BUSY:
        break;
    case WH_SYSSTATE_CONNECTED:
    case WH_SYSSTATE_KEYSHARING:
    case WH_SYSSTATE_DATASHARING:
        sMode = MODE_GMAIN;
        break;
    }
}


/* ----------------------------------------------------------------------
  Name:        ModeGMain
  
  Description: Title screen
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeGMain(void)
{
    if (WH_GetSystemState() == WH_SYSSTATE_ERROR)
    {
        ChangeWirelessState(WSTATE_ERROR);
        sMode = MODE_ERROR;
        return;
    }
    GStepDataShare(gFrame);
    GMain();
}


/* ----------------------------------------------------------------------
  Name:        ModeCancel
  
  Description: Canceling screen
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeCancel(void)
{
    switch (sWirelessState)
    {
        //-------------------------
        // End wireless after moving to idle state
    case WSTATE_IDLE:
        if (WH_GetSystemState() == WH_SYSSTATE_IDLE)
        {
            (void)WH_End();
            return;
        }

        if (WH_GetSystemState() != WH_SYSSTATE_STOP)
        {
            return;
        }
        ChangeWirelessState(WSTATE_STOP);
        sMode = MODE_TITLE;
        break;
        //--------------------------
        // Canceling
    case WSTATE_FAKE_CANCEL_BUSY:
        break;

        //--------------------------
        // Error occurred
    case WSTATE_ERROR:
        ChangeWirelessState(WSTATE_IDLE);
        (void)WH_Finalize();
        break;

        //--------------------------
        // End processing
    default:
        ChangeWirelessState(WSTATE_IDLE);
        (void)WH_Finalize();
        break;
    }
}


/* ----------------------------------------------------------------------
  Name:        ModeError
  
  Description: Error screen
  
  Arguments:   None.
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void ModeError(void)
{
    BgPutString(4, 10, PLTT_RED, "========== ERROR ==========");

    if (IS_PAD_TRIGGER(PAD_BUTTON_B) || IS_PAD_TRIGGER(PAD_BUTTON_A))
    {
        if (sWirelessState == WSTATE_FAKE_ERROR)
        {
            ChangeWirelessState(WSTATE_FAKE_CANCEL_BUSY);
            MB_FakeEnd();
        }
        sMode = MODE_CANCEL;
    }
}


/* ----------------------------------------------------------------------
  Name:        NotifyScanParent
  
  Description: Parent scan state notification callback.
  
  Arguments:   state: Scan state
                        MB_FAKESCAN_PARENT_FOUND, MB_FAKESCAN_PARENT_LOST,
                        MB_FAKESCAN_PARENT_FOUND, MB_FAKESCAN_PARENT_LOST,
                        These 4 types of messages are indicated.
                
               arg: If MB_FAKESCAN_PARENT_FOUND or MB_FAKESCAN_PARENT_LOST, the information of the parent can be obtained
                        
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void NotifyScanParent(u16 type, void *arg)
{
    static const char *MB_FAKESCAN_CALLBACK_TYPE_NAME[] = {
        "MB_FAKESCAN_PARENT_FOUND",
        "MB_FAKESCAN_PARENT_LOST",
        "MB_FAKESCAN_API_ERROR",
        "MB_FAKESCAN_END_SCAN",
        "MB_FAKESCAN_PARENT_BEACON",
        "MB_FAKESCAN_CONNECTED",
        "MB_FAKESCAN_ENTRY",
        "MB_FAKESCAN_DISCONNECTED",
        "MB_FAKESCAN_SUCCESS"
    };

    OS_TPrintf("> %s\n", MB_FAKESCAN_CALLBACK_TYPE_NAME[type]);

    switch (type)
    {
    case MB_FAKESCAN_PARENT_BEACON:
        {
            MBFakeScanCallback *cb = (MBFakeScanCallback *)arg;
            WMBssDesc *bssdesc = cb->bssDesc;
            const u8 *volat_data = (const u8 *)MB_GetUserVolatData(&bssdesc->gameInfo);
            OS_TPrintf(" find MB-beacon MAC=(%04x%08x)\n",
                       *(u16 *)(&bssdesc->bssid[4]), *(u32 *)(&bssdesc->bssid[0]));
            if (volat_data)
            {
                OS_TPrintf("                VOLAT=(%02x %02x %02x %02x %02x %02x %02x %02x)\n",
                           volat_data[0], volat_data[1], volat_data[2], volat_data[3],
                           volat_data[4], volat_data[5], volat_data[6], volat_data[7]);
            }
        }
        break;
    case MB_FAKESCAN_PARENT_FOUND:
        {
            MBFakeScanCallback *cb = (MBFakeScanCallback *)arg;
            WMBssDesc *bssdesc = cb->bssDesc;
            MBGameInfo *gameInfo = cb->gameInfo;

            OS_TPrintf(" find parent %04x%08x %d\n", *(u16 *)(&bssdesc->bssid[4]),
                       *(u32 *)(&bssdesc->bssid[0]), cb->index);
            OS_TPrintf("      parentMaxSize = %d, childMaxSize = %d\n",
                       bssdesc->gameInfo.parentMaxSize, bssdesc->gameInfo.childMaxSize);

            parentInfo[cb->index].valid = 1;
            parentInfo[cb->index].maxPlayerNum = gameInfo->fixed.maxPlayerNum;
            parentInfo[cb->index].nowPlayerNum = gameInfo->volat.nowPlayerNum;
            MI_CpuCopy8(&gameInfo->fixed.parent, &parentInfo[cb->index].info, sizeof(MBUserInfo));
            WM_CopyBssid(bssdesc->bssid, parentInfo[cb->index].macAddr);
        }
        break;
    case MB_FAKESCAN_PARENT_LOST:
        {
            MBFakeScanCallback *cb = (MBFakeScanCallback *)arg;

            OS_TPrintf(" lost parent %d\n", cb->index);

            parentInfo[cb->index].valid = 0;
        }
        break;
    case MB_FAKESCAN_API_ERROR:
        {
            PrintError((MBFakeScanErrorCallback *)arg);
            OS_TPrintf("ERR : API Error occured\n");
            ChangeWirelessState(WSTATE_FAKE_ERROR);
        }
        break;
    case MB_FAKESCAN_END_SCAN:
        {
            ChangeWirelessState(WSTATE_IDLE);
        }
        break;
    default:
        break;
    }
}


/* ----------------------------------------------------------------------
  Name:        MBStateCallback
  
  Description: Fake child state-transition notification callback.
               After MB_FakeEntryToParent is called, the sequence up to the child boot completion message will execute automatically. No special processing is needed.
               
               
               When all multiboot processing is complete, the MB_COMM_CSTATE_BOOT_READY callback will return.
               
  
  Arguments:   status: Current state of the MB child
               arg: Callback argument
                
  Returns:     None.
  ---------------------------------------------------------------------- */
static void MBStateCallback(u32 status, void *arg)
{
#pragma unused( arg )
    switch (status)
    {
    case MB_COMM_CSTATE_INIT_COMPLETE: // Notification of connection to parent complete
    case MB_COMM_CSTATE_CONNECT:      // Notification of connection to parent complete
    case MB_COMM_CSTATE_REQ_ENABLE:   // Notification of MP communication with parent complete
    case MB_COMM_CSTATE_DLINFO_ACCEPTED:       // Notification of completion of entry to parent
    case MB_COMM_CSTATE_RECV_PROCEED: // Notification of start of data transmission from parent
    case MB_COMM_CSTATE_RECV_COMPLETE: // Notification of completion of parent's data transmission
    case MB_COMM_CSTATE_BOOTREQ_ACCEPTED:      // Notification of reception of boot request from parent
        // Special processing is unnecessary
        break;

    case MB_COMM_CSTATE_BOOT_READY:
        // Notiffication of disconnection from parent and MB processing complete
        {
            ChangeWirelessState(WSTATE_FAKE_BOOT_READY);
        }
        break;

    case MB_COMM_CSTATE_CANCELED:
        // Notification that an entry with a parent device was cancelled mid-process
        break;

    case MB_COMM_CSTATE_CONNECT_FAILED:        // Notification of connection failure to parent
    case MB_COMM_CSTATE_DISCONNECTED_BY_PARENT:        // Notification of disconnection from parent
    case MB_COMM_CSTATE_REQ_REFUSED:  // Notification when request to parent was refused
    case MB_COMM_CSTATE_MEMBER_FULL:  // Notification that the entry child number of the parent was exceeded
    case MB_COMM_CSTATE_ERROR:        // Processing if abnormal transmission status occurred midway
        {
            OS_TPrintf("ERR : MB Error occured reason 0x%x\n", status);
            ChangeWirelessState(WSTATE_FAKE_ERROR);
        }
        break;

    case MB_COMM_CSTATE_FAKE_END:
        // Release the work passed to MB
        OS_Free(mbfBuf);
        mbfBuf = NULL;

        if (sWirelessState == WSTATE_FAKE_BOOT_END_BUSY)
        {
            ChangeWirelessState(WSTATE_FAKE_BOOT_END);
        }
        else
        {
            ChangeWirelessState(WSTATE_IDLE);
        }
    }
}



/* ----------------------------------------------------------------------
  Name:        PrintError
  
  Description: Outputs contents of the WM API error.
  
  Arguments:   arg: Error callback for MB_FakeStartScanParent
  
  Returns:     None.
  ---------------------------------------------------------------------- */
static void PrintError(MBFakeScanErrorCallback *arg)
{
#pragma unused( arg )
    static const char *APIID_NAME[] = {
        "WM_APIID_INITIALIZE",
        "WM_APIID_RESET",
        "WM_APIID_END",
        "WM_APIID_ENABLE",
        "WM_APIID_DISABLE",
        "WM_APIID_POWER_ON",
        "WM_APIID_POWER_OFF",
        "WM_APIID_SET_P_PARAM",
        "WM_APIID_START_PARENT",
        "WM_APIID_END_PARENT",
        "WM_APIID_START_SCAN",
        "WM_APIID_END_SCAN",
        "WM_APIID_START_CONNECT",
        "WM_APIID_DISCONNECT",
        "WM_APIID_START_MP",
        "WM_APIID_SET_MP_DATA",
        "WM_APIID_END_MP",
        "WM_APIID_START_DCF",
        "WM_APIID_SET_DCF_DATA",
        "WM_APIID_END_DCF",
        "WM_APIID_SET_WEPKEY",
        "WM_APIID_START_KS",
        "WM_APIID_END_KS",
        "WM_APIID_GET_KEYSET",
        "WM_APIID_SET_GAMEINFO",
        "WM_APIID_SET_BEACON_IND",
        "WM_APIID_START_TESTMODE",
        "WM_APIID_STOP_TESTMODE",
        "WM_APIID_VALARM_MP",
        "WM_APIID_SET_LIFETIME",
        "WM_APIID_MEASURE_CHANNEL",
        "WM_APIID_INIT_W_COUNTER",
        "WM_APIID_GET_W_COUNTER",
        "WM_APIID_SET_ENTRY",
        "WM_APIID_AUTO_DEAUTH",
        "WM_APIID_SET_MP_PARAMETER",
        "WM_APIID_SET_BEACON_PERIOD",
        "WM_APIID_AUTO_DISCONNECT",
        "WM_APIID_START_SCAN_EX",
        "WM_APIID_KICK_MP_PARENT",
        "WM_APIID_KICK_MP_CHILD",
        "WM_APIID_KICK_MP_RESUME",
        "WM_APIID_ASYNC_KIND_MAX",
        "WM_APIID_INDICATION",
        "WM_APIID_PORT_SEND",
        "WM_APIID_PORT_RECV",
        "WM_APIID_READ_STATUS",
        "WM_APIID_UNKNOWN",
    };

    static const char *ERRCODE_NAME[] = {
        "WM_ERRCODE_SUCCESS",
        "WM_ERRCODE_FAILED",
        "WM_ERRCODE_OPERATING",
        "WM_ERRCODE_ILLEGAL_STATE",
        "WM_ERRCODE_WM_DISABLE",
        "WM_ERRCODE_NO_DATASET",
        "WM_ERRCODE_INVALID_PARAM",
        "WM_ERRCODE_NO_CHILD",
        "WM_ERRCODE_FIFO_ERROR",
        "WM_ERRCODE_TIMEOUT",
        "WM_ERRCODE_SEND_QUEUE_FULL",
        "WM_ERRCODE_NO_ENTRY",
        "WM_ERRCODE_OVER_MAX_ENTRY",
        "WM_ERRCODE_INVALID_POLLBITMAP",
        "WM_ERRCODE_NO_DATA",
        "WM_ERRCODE_SEND_FAILED",
        "WM_ERRCODE_DCF_TEST",
        "WM_ERRCODE_WL_INVALID_PARAM",
        "WM_ERRCODE_WL_LENGTH_ERR",
        "WM_ERRCODE_FLASH_ERROR"
    };

    OS_TPrintf("ERR: %s %s\n", APIID_NAME[arg->apiid], ERRCODE_NAME[arg->errcode]);
}
