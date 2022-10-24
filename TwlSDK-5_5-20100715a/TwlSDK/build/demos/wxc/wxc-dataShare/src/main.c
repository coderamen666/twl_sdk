/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
  File:     main.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-31#$
  $Rev: 11030 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

/*
 * This demo uses the WH library for wireless communications, but does not perform wireless shutdown processing.
 * 
 * For details on WH library wireless shutdown processing, see the comments at the top of the WH library source code or the wm/dataShare-Model demo.
 * 
 * 
 */

#include <nitro.h>

#include <nitro/wxc.h>

#include "user.h"
#include "common.h"
#include "disp.h"
#include "gmain.h"
#include "wh.h"

void VBlankIntr(void);          // V-Blank interrupt handler

/*---------------------------------------------------------------------------*
    Constant definitions
 *---------------------------------------------------------------------------*/
#define     KEY_REPEAT_START    25     // Number of frames until key repeat starts
#define     KEY_REPEAT_SPAN     10     // Number of frames between key repeats

#define     PICTURE_FRAME_PER_GAME_FRAME    2


/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x52)

/* GGID used for this Sharing */
#define WH_GGID                 SDK_MAKEGGID_SYSTEM(0x21)

/*---------------------------------------------------------------------------*
    Structure definitions
 *---------------------------------------------------------------------------*/
// Key input data
typedef struct KeyInfo
{
    u16     cnt;                       // Unprocessed input value
    u16     trg;                       // Push trigger input
    u16     up;                        // Release trigger input
    u16     rep;                       // Press and hold repeat input

}
KeyInfo;


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void ModeSelect(void);          // Parent/child select screen
static void ModeError(void);           // Error display screen


// General purpose subroutines
static void KeyRead(KeyInfo * pKey);
static void InitializeAllocateSystem(void);

static void GetMacAddress(u8 * macAddr, u16 * gScreen);


static void ModeConnect();
static void ModeWorking(void);

/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static KeyInfo gKey;                   // Key input
static vs32 gPictureFrame;             // Picture frame counter


/*---------------------------------------------------------------------------*
    External Variable Definitions
 *---------------------------------------------------------------------------*/
int passby_endflg;
WMBssDesc bssdesc;
WMParentParam parentparam;
u8 macAddress[6];


static void WaitPressButton(void);
static void GetChannelMain(void);
static BOOL ConnectMain(u16 tgid);
static void PrintChildState(void);
static BOOL JudgeConnectableChild(WMStartParentCallback *cb);


//============================================================================
//   Variable Definitions
//============================================================================

static s32 gFrame;                     // Frame counter

//============================================================================
//   Function definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Gets key trigger

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    // Increment picture frame counter
    gPictureFrame++;

    // Repeat rendering to match game frame
    if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
    {
        DispVBlankFunc();
    }

    //---- Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main routine

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    u16 tgid = 0;

    // Initialize screen and OS
    CommonInit();
    // Initialize screen
    DispInit();
    // Initialize the heap
    InitAllocateSystem();

    // Enable interrupts
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    OS_InitTick();
    FX_Init();

    DispOn();

    passby_endflg = 0;
    
    /* Initializes, registers, and runs chance encounter communications */
    User_Init();

    for (gFrame = 0; passby_endflg == 0; gFrame++)
    {
        /* Get key input data */
        KeyRead(&gKey);

        // Clear the screen
        BgClear();
        
        /* Distributes processes based on communication status */
        switch (WXC_GetStateCode())
        {
        case WXC_STATE_END:
            ModeSelect();
            break;
        case WXC_STATE_ENDING:
            break;
        case WXC_STATE_ACTIVE:
            if(WXC_IsParentMode() == TRUE)
            {
                BgPutString(9, 2, 0x2, "Now parent...");
            }
            else
            {
                BgPutString(9, 2, 0x2, "Now child...");
            }
            break;
        }
        if (gKey.trg & PAD_BUTTON_START)
        {
            (void)WXC_End();
        }

        // Waiting for the V-Blank
        OS_WaitVBlankIntr();
    }

    (void)WXC_End();
    
    // Wait for WXC_End to complete
    while( WXC_GetStateCode() != WXC_STATE_END )
    {
        ;
    }

    // Variable initialization
    gPictureFrame = 0;

    // Set buffer for data sharing communication
    GInitDataShare();

    // Initialize wireless
    if( WH_Initialize() == FALSE )
    {
        OS_Printf("\n WH_Initialize() ERROR!!\n");
    }

    if(passby_endflg == 1)
    {
        WH_SetGgid(WH_GGID);
        
        // Set the function to determine connected children
        WH_SetJudgeAcceptFunc(JudgeConnectableChild);
    
        /* Main routine */
        for (gFrame = 0; TRUE; gFrame++)
        {
            BgClear();

            switch (WH_GetSystemState())
            {
            case WH_SYSSTATE_IDLE:
                (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, (u16)(parentparam.tgid+1), parentparam.channel);
                break;

            case WH_SYSSTATE_CONNECTED:
            case WH_SYSSTATE_KEYSHARING:
            case WH_SYSSTATE_DATASHARING:
                {
                    BgPutString(8, 1, 0x2, "Parent mode");
                    if(GStepDataShare(gPictureFrame) == FALSE)
                    {
                        gPictureFrame--;
                    }
                    GMain();
                }
                break;
            }

            // Wait for completion of multiple cycles of picture frames
            while (TRUE)
            {
                // Wait for V-Blank
                OS_WaitVBlankIntr();
                if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
                {
                    break;
                }
            }
        }
    }
    else
    {
        OS_Printf("\nmacAddress = %02X:%02X:%02X:%02X:%02X:%02X\n", bssdesc.bssid[0],bssdesc.bssid[1],bssdesc.bssid[2],bssdesc.bssid[3],bssdesc.bssid[4],bssdesc.bssid[5]);
        
        bssdesc.gameInfo.tgid++;
    
        // Main loop
        for (gFrame = 0; TRUE; gFrame++)
        {
            // Clear the screen
            BgClear();

            // Distributes processes based on communication status
            switch (WH_GetSystemState())
            {
            case WH_SYSSTATE_CONNECT_FAIL:
                {
                    // Because the internal WM state is invalid if the WM_StartConnect function fails, WM_Reset must be called once to reset the state to IDLE
                    // 
                    WH_Reset();
                }
                break;
            case WH_SYSSTATE_IDLE:
                {
                    static retry = 0;
                    enum
                    {
                        MAX_RETRY = 30
                    };

                    if (retry < MAX_RETRY)
                    {
                        ModeConnect(bssdesc.bssid);
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
                    if(GStepDataShare(gPictureFrame) == FALSE)
                    {
                        gPictureFrame--;
                    }
                    GMain();
                }
                break;
            }

            // Display of signal strength
            {
                int level;
                level = WH_GetLinkLevel();
                BgPrintStr(31, 23, 0xf, "%d", level);
            }

            // Wait for completion of multiple cycles of picture frames
            while (TRUE)
            {
                // Wait for V-Blank
                OS_WaitVBlankIntr();
                if (!(gPictureFrame % PICTURE_FRAME_PER_GAME_FRAME))
                {
                    break;
                }
            }
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         ModeSelect

  Description:  Processing in parent/child selection screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeSelect(void)
{
    BgPutString(3, 1, 0x2, "Push A to start");

    if (gKey.trg & PAD_BUTTON_A)
    {
        //********************************
        User_Init();
        //********************************
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Processing in error display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeError(void)
{
    BgPutString(5, 10, 0x2, "======= ERROR! =======");
    BgPutString(5, 13, 0x2, " Fatal error occured.");
    BgPutString(5, 14, 0x2, "Please reboot program.");
}


/*---------------------------------------------------------------------------*
  Name:         KeyRead

  Description:  Edits key input data
                Detects press trigger, release trigger, and press-and-hold repeat.

  Arguments:    pKey: Structure that holds key input data to be edited

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void KeyRead(KeyInfo * pKey)
{
    static u16 repeat_count[12];
    int     i;
    u16     r;

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
                    repeat_count[i] = KEY_REPEAT_START - KEY_REPEAT_SPAN;
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
    pKey->cnt = r;                     // Unprocessed key input
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
    if (cb->macAddress[0] != macAddress[0] || cb->macAddress[1] != macAddress[1] || 
        cb->macAddress[2] != macAddress[2] || cb->macAddress[3] != macAddress[3] || 
        cb->macAddress[4] != macAddress[4] || cb->macAddress[5] != macAddress[5] )
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         ModeConnect

  Description:  Connection start

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ModeConnect()
{
    WH_SetGgid(WH_GGID);
    //********************************
    (void)WH_ChildConnectAuto(WH_CONNECTMODE_DS_CHILD, bssdesc.bssid,
                              bssdesc.channel);
    //********************************
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
}
