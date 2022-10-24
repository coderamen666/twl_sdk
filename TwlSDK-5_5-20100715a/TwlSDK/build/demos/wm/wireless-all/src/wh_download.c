/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wireless-all
  File:     wh_download.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-28#$
  $Rev: 11027 $
  $Author: tominaga_masafumi $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"


/*****************************************************************************/
/* Constants */

/* Download program data (clone boot) */
/* *INDENT-OFF* */
const MBGameRegistry mbGameList =
{
    NULL,
    (u16 *)L"wireless-all",
    (u16 *)L"all of the wireless demo(cloneboot, dataSharing, WFS)",
    "/icon.char", "/icon.plt",
    WH_GGID,
    WH_CHILD_MAX + 1,
};
/* *INDENT-ON* */


/*****************************************************************************/
/* Functions */

/*****************************************************************************/
/* Start the definition range of the .parent section parent device exclusive region */
#include <nitro/parent_begin.h>
/*****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         ConnectMain

  Description:  Connect with multiboot.

  Arguments:    None.

  Returns:      Returns TRUE if the transfer to the child succeeds; returns FALSE if transfer fails or is cancelled.
                
 *---------------------------------------------------------------------------*/
static BOOL ConnectMain(void)
{
    for (;;)
    {
        /* Frame wait */
        (void)WaitNextFrame();

        PrintString(4, 22, PLTT_YELLOW, " MB Parent ");

        switch (MBP_GetState())
        {
            //-----------------------------------------
            // Accepting entry from the child
        case MBP_STATE_ENTRY:
            {
                PrintString(4, 22, PLTT_WHITE, " Now Accepting            ");

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
                    PrintString(4, 22, PLTT_WHITE, " Push START Button to start   ");

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

                // If everyone has completed download, start is possible
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
                PrintString(4, 22, PLTT_WHITE, " Rebooting now                ");
            }
            break;

            //-----------------------------------------
            // Reconnect process
        case MBP_STATE_COMPLETE:
            {
                // Once all members have successfully connected, the multiboot processing ends, and restarts the wireless as an normal parent
                //  
                PrintString(4, 22, PLTT_WHITE, " Reconnecting now             ");

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
            // Communication cancellation processsing
        case MBP_STATE_CANCEL:
            // None.
            break;

            //-----------------------------------------
            // Abnormal termination of communication
        case MBP_STATE_STOP:
            OS_WaitVBlankIntr();
            return FALSE;
        }

        // Display child state
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
            for (i = 1; i < WH_PLAYER_MAX; i++)
            {
                const MBPChildInfo *childInfo;
                MBPChildState childState = MBP_GetChildState(i);
                const u8 *macAddr;
                // State display
                PrintString(STATE_DISP_X, i + BASE_DISP_Y, PLTT_WHITE, STATE_NAME[childState]);
                // User information display
                childInfo = MBP_GetChildInfo(i);
                macAddr = MBP_GetChildMacAddress(i);
                if (macAddr)
                {
                    PrintString(INFO_DISP_X, i + BASE_DISP_Y, PLTT_WHITE,
                                "%02x%02x%02x%02x%02x%02x",
                                macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4],
                                macAddr[5]);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ExecuteDownloadServer

  Description:  DS Single-Card play parent processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ExecuteDownloadServer(void)
{
    WH_SetGgid(WH_GGID);

    for (;;)
    {
        /* Wireless initialization */
        if (!WH_Initialize())
        {
            OS_Panic("WH_Initialize failed.");
        }
        WaitWHState(WH_SYSSTATE_IDLE);
        while (!IS_PAD_TRIGGER(PAD_BUTTON_A))
        {
            (void)WaitNextFrame();
            PrintString(4, 22, PLTT_WHITE, " Press A Button to start   ");
        }
        /* Search for a channel with little traffic */
        SetCurrentChannel(MeasureChannel());

        /* Start DS Download Play */
#if !defined(MBP_USING_MB_EX)
        (void)WH_End();
        WaitWHState(WH_SYSSTATE_STOP);
#endif
        MBP_Init(WH_GGID, WM_GetNextTgid());
        MBP_Start(&mbGameList, GetCurrentChannel());

        if (ConnectMain())
        {
            /* Multiboot child startup is successful */
            break;
        }
        else
        {
            /* Exit the WH module and repeat */
            WH_Finalize();
            WaitWHState(WH_SYSSTATE_IDLE);
            (void)WH_End();
            WaitWHState(WH_SYSSTATE_STOP);
        }
    }

}


/*****************************************************************************/
/* End the definition boundary of the .parent section parent device exclusive region */
#include <nitro/parent_end.h>
/*****************************************************************************/
