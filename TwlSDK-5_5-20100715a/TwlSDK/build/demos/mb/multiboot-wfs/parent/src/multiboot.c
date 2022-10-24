/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - parent
  File:     multiboot.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8575 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include    <twl.h>
#else
#include    <nitro.h>
#endif
#include <nitro/wm.h>
#include <nitro/wbt.h>
#include <nitro/fs.h>

#include    "wfs.h"
#include    "wh.h"
#include    "mbp.h"

#include    "util.h"
#include    "common.h"


/*---------------------------------------------------------------------------*
    Internal Constant Definitions
 *---------------------------------------------------------------------------*/

/* The program information that this demo downloads */
const MBGameRegistry mbGameList = {
    "data/main.srl",                   // Child binary code
    (u16 *)L"MultibootWFS",            // Game name
    (u16 *)L"Multiboot-WFS demo",      // Game content description
    "data/icon.char",                  // Icon character data
    "data/icon.plt",                   // Icon palette data
    GGID_WBT_FS,                       // GGID
    MBP_CHILD_MAX + 1,                 // Max. number of players, including parents
};


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/

static void *sWmWork;

/* Initialize variables for channel measurement */
static volatile BOOL sChannelBusy;
static u16 sChannel;
static u16 sChannelBusyRatio;
static u16 sChannelBitmap;


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

static void GetChannelProc(void *arg);
static void GetChannelMain(void);
static void ConnectMain(void);
static void PrintChildState(void);


/*---------------------------------------------------------------------------*
  Name:         ModeMultiboot

  Description:  DS Single-Card play parent processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeMultiboot(void)
{
    /* Search channel with the lowest radio wave usage rate */
    GetChannelMain();

    /* Multiboot distribution */
    ConnectMain();
}

/*---------------------------------------------------------------------------*
  Name:         GetChannelProc

  Description:  Series of WM function callbacks for channel measurement.
                When specified as the WM_Initialize callback, measure the usage rate of all available channels, and store the lowest channel in sChannel to complete.
                
                
                When specified as the WM_End callback, complete immediately.
                When completing, set sChannelBusy to FALSE.

  Arguments:    arg: WM function callback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetChannelProc(void *arg)
{
    u16     channel = 1;
    u16     allowedChannel;

    WMCallback *cb = (WMCallback *)arg;

    /* If WM_End callback, indicate completion */
    if (cb->apiid == WM_APIID_END)
    {
        sChannelBusy = FALSE;
        return;
    }
    /* If WM_Initialize callback, start measurement process */
    else if (cb->apiid == WM_APIID_INITIALIZE)
    {
    }
    /* Otherwise, WM_MeasureChannel callback */
    else
    {
        WMMeasureChannelCallback *cb = (WMMeasureChannelCallback *)arg;
        if (cb->errcode != WM_ERRCODE_SUCCESS)
        {
            OS_TPanic("WM_MeasureChannel() failed!");
        }
        channel = cb->channel;
        /* If the usage rate is the lowest so far, use it */
        if (sChannelBusyRatio > cb->ccaBusyRatio)
        {
            sChannelBusyRatio = cb->ccaBusyRatio;
            sChannelBitmap = (u16)(1 << (channel - 1));
        }
        /* Use it also if the value is equal to the lowest so far */
        else if (sChannelBusyRatio == cb->ccaBusyRatio)
        {
            sChannelBitmap |= 1 << (channel - 1);
        }
        ++channel;
    }

    /* Obtain the available channels */
    allowedChannel = WM_GetAllowedChannel();
    if (allowedChannel == 0x8000)
    {
        OS_TPanic("WM_GetAllowedChannel() failed!");
    }
    else if (allowedChannel == 0)
    {
        OS_TPanic("no available wireless channels!");
    }

    /* Complete after all channels are measured */
    if ((1 << (channel - 1)) > allowedChannel)
    {
        int     num = MATH_CountPopulation(sChannelBitmap);
        if (num == 0)
        {
            OS_TPanic("no available wireless channels!");
        }
        else if (num == 1)
        {
            sChannel = (u16)(31 - MATH_CountLeadingZeros(sChannelBitmap) + 1);
        }
        /* If more than one channel has the same usage rate, pick one at random */
        else
        {
            int     select = (int)(((OS_GetVBlankCount() & 0xFF) * num) / 0x100);
            int     i;
            for (i = 0; i < 16; i++)
            {
                if ((sChannelBitmap & (1 << i)) && (--select < 0))
                {
                    break;
                }
            }
            sChannel = (u16)(i + 1);
        }
        sChannelBusy = FALSE;
    }
    /* If there is an unmeasured channel, continue process */
    else
    {
        while (((1 << (channel - 1)) & allowedChannel) == 0)
        {
            ++channel;
        }
        if (WM_MeasureChannel(GetChannelProc, 3, 17, channel, 30) != WM_ERRCODE_OPERATING)
        {
            OS_TPanic("WM_MeasureChannel() failed!");
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetChannelMain

  Description:  Measure channel with the lowest usage rate.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void GetChannelMain(void)
{
    KeyInfo key[1];

    /* Initialize variables for channel measurement */
    sChannelBusy = TRUE;
    sChannel = 0;
    sChannelBusyRatio = 100 + 1;
    sChannelBitmap = 0;

    /* Allocate WM work memory and start library */
    sWmWork = OS_Alloc(WM_SYSTEM_BUF_SIZE);
    if (!sWmWork)
    {
        OS_TPanic("failed to allocate memory for WM!");
    }
    else if (WM_Initialize(sWmWork, GetChannelProc, 2) != WM_ERRCODE_OPERATING)
    {
        OS_TPanic("WM_Initialize() failed!");
    }

    /* Standby until measurement is done */
    InitFrameLoop(key);
    while (sChannelBusy)
    {
        WaitForNextFrame(key);
        PrintString(4, 22, COLOR_YELLOW, " Searching Channel %c",
                    ((OS_GetVBlankCount() / 10) & 1) ? '_' : ' ');
    }

#if !defined(MBP_USING_MB_EX)
    /* Close library and release WM work memory */
    sChannelBusy = TRUE;
    if (WM_End(GetChannelProc) != WM_ERRCODE_OPERATING)
    {
        OS_TPanic("WM_End() failed!");
    }
    while (sChannelBusy)
    {
    }
    OS_Free(sWmWork), sWmWork = NULL;
#endif

}

/*---------------------------------------------------------------------------*
  Name:         ConnectMain

  Description:  Connect with multiboot.

  Arguments:    tgid: Specifies the tgid for booting as parent

  Returns:      TRUE if the transfer to the child succeeds; FALSE if transfer fails or is canceled.
                
 *---------------------------------------------------------------------------*/
static void ConnectMain(void)
{
    KeyInfo key[1];

    MBP_Init(mbGameList.ggid, WM_GetNextTgid());

    InitFrameLoop(key);
    for (;;)
    {
        int     state;

        WaitForNextFrame(key);
        PrintString(4, 22, COLOR_YELLOW, " MB Parent ");
        state = MBP_GetState();
        /* Idle state */
        if (state == MBP_STATE_IDLE)
        {
            MBP_Start(&mbGameList, sChannel);
        }
        /* Accepting entry from the child */
        else if (state == MBP_STATE_ENTRY)
        {
            PrintString(4, 22, COLOR_WHITE, " Now Accepting            ");
            /* Cancel multiboot with B button */
            if (key->trg & PAD_BUTTON_B)
            {
                MBP_Cancel();
            }
            /* If there is at least one child in entry, start is possible */
            else if (MBP_GetChildBmp(MBP_BMPTYPE_ENTRY) ||
                     MBP_GetChildBmp(MBP_BMPTYPE_DOWNLOADING) ||
                     MBP_GetChildBmp(MBP_BMPTYPE_BOOTABLE))
            {
                PrintString(4, 22, COLOR_WHITE, " Push START Button to start   ");
                if (key->trg & PAD_BUTTON_START)
                {
                    /* Start download */
                    MBP_StartDownloadAll();
                }
            }
        }
        /*
         * Program distribution process.
         * If everyone has finished downloading, send a reboot request
         */
        else if (state == MBP_STATE_DATASENDING)
        {
            if (MBP_IsBootableAll())
            {
                MBP_StartRebootAll();
            }
        }
        /* Standby processing for child reboot */
        else if (state == MBP_STATE_REBOOTING)
        {
            PrintString(4, 22, COLOR_WHITE, " Rebooting now                ");
        }
        /* Processing for when child reboot is finished */
        else if (state == MBP_STATE_COMPLETE)
        {
            PrintString(4, 22, COLOR_WHITE, " Reconnecting now             ");
            break;
        }
        /* When error occurs, cancel communication */
        else if (state == MBP_STATE_ERROR)
        {
            MBP_Cancel();
        }
        /* Communication cancellation processing */
        else if (state == MBP_STATE_CANCEL)
        {
        }
        /* Abnormal termination of communication */
        else if (state == MBP_STATE_STOP)
        {
            break;
        }

        /* Display child status */
        PrintChildState();
    }

#if defined(MBP_USING_MB_EX)
    /* Close library and release WM work memory */
    sChannelBusy = TRUE;
    if (WM_End(GetChannelProc) != WM_ERRCODE_OPERATING)
    {
        OS_TPanic("WM_End() failed!");
    }
    while (sChannelBusy)
    {
    }
    OS_Free(sWmWork), sWmWork = NULL;
#endif

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
        PrintString(STATE_DISP_X, i + BASE_DISP_Y, COLOR_WHITE, STATE_NAME[childState]);

        // User information display
        childInfo = MBP_GetChildInfo(i);
        macAddr = MBP_GetChildMacAddress(i);

        if (macAddr != NULL)
        {
            PrintString(INFO_DISP_X, i + BASE_DISP_Y, COLOR_WHITE,
                        "%02x%02x%02x%02x%02x%02x",
                        macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
        }
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
