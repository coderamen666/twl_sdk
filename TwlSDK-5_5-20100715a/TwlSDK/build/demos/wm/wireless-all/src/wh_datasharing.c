/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wireless-all
  File:     wh_datasharing.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         StateCallbackForWFS

  Description:  WFS callback function.
                This is called when the state became WFS_STATE_READY.
                WFS_GetStatus() can be used at any position to make a determination without receiving notification with this callback.
                

  Arguments:    arg:       User-defined argument specified in the callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StateCallbackForWFS(void *arg)
{
    (void)arg;
    switch (WFS_GetStatus())
    {
    case WFS_STATE_READY:
        OS_TPrintf("WFS ready.\n");
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         AllocatorForWFS

  Description:  Dynamic allocation function for memory specified in WFS.

  Arguments:    arg:       User-defined argument specified in the allocator
                size:      Required size if requesting memory allocation
                ptr:       If NULL, allocates memory. Otherwise, requests deallocation.

  Returns:      If ptr is NULL, the amount of memory in size is allocated and its pointer returned.
                If not, the ptr memory is released.
                If released, the return value is simply ignored.
 *---------------------------------------------------------------------------*/
static void *AllocatorForWFS(void *arg, u32 size, void *ptr)
{
    (void)arg;
    if (!ptr)
        return OS_Alloc(size);
    else
    {
        OS_Free(ptr);
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         JudgeConnectableChild

  Description:  Determines if the child is connectable during reconnect.

  Arguments:    cb:      Information for the child attempting to connect

  Returns:      If connection is accepted, TRUE.
                If not accepted, FALSE.
 *---------------------------------------------------------------------------*/
static BOOL JudgeConnectableChild(WMStartParentCallback *cb)
{
    /*  Search the MAC address for the AID of when multibooting the child of cb->aid */
    u16     playerNo = MBP_GetPlayerNo(cb->macAddress);
    OS_TPrintf("MB child(%d) -> DS child(%d)\n", playerNo, cb->aid);
    return (playerNo != 0);
}

/*---------------------------------------------------------------------------*
  Name:         StartWirelessParent

  Description:  Starts the parent wireless communication process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StartWirelessParent(void)
{
    /* Initialize WFS and start connection processing */
    WFS_InitParent(PORT_WFS, StateCallbackForWFS, AllocatorForWFS,
                   NULL, WH_PARENT_MAX_SIZE, NULL, TRUE);
    WH_SetJudgeAcceptFunc(JudgeConnectableChild);
    (void)WH_ParentConnect(WH_CONNECTMODE_DS_PARENT, WM_GetNextTgid(), GetCurrentChannel());
}

/*---------------------------------------------------------------------------*
  Name:         StartWirelessChild

  Description:  Starts the child wireless communication process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StartWirelessChild(void)
{
    for (;;)
    {
        /* Starts WFS initialization, parent search, and connection processes */
        WaitWHState(WH_SYSSTATE_IDLE);
        WFS_InitChild(PORT_WFS, StateCallbackForWFS, AllocatorForWFS, NULL);
        WH_SetGgid(WH_GGID);
        (void)WH_ChildConnectAuto(WH_CONNECTMODE_DS_CHILD,
                                  (const u8 *)MB_GetMultiBootParentBssDesc()->bssid, 0);
        while ((WH_GetSystemState() == WH_SYSSTATE_BUSY) ||
               (WH_GetSystemState() == WH_SYSSTATE_SCANNING))
        {
            (void)WaitNextFrame();
            PrintString(9, 11, 0xf, "Now working...");
            if (IS_PAD_TRIGGER(PAD_BUTTON_START))
            {
                (void)WH_Finalize();
            }
        }
        /* Retries if connection fails */
        if (WH_GetSystemState() == WH_SYSSTATE_CONNECT_FAIL)
        {
            WH_Reset();
            WaitWHState(WH_SYSSTATE_IDLE);
        }
        /* End here if an unexpected error occurs */
        else if (WH_GetSystemState() == WH_SYSSTATE_ERROR)
        {
            for (;;)
            {
                static int over_max_entry_count = 0;
                
                (void)WaitNextFrame();
                
                if (WH_GetLastError() == WM_ERRCODE_OVER_MAX_ENTRY)
                {
                    // Considering the possibility of another download child having connected to the parent while reconnecting to the parent, retry the connection several times.
                    // 
                    // If the retry fails, it is really OVER_MAX_ENTRY.
                    if (over_max_entry_count < 10)
                    {
                        WH_Reset();
                        
                        over_max_entry_count++;
                        
                        break;
                    }
                    else
                    {
                        PrintString(5, 13, 0xf, "OVER_MAX_ENTRY");
                    }
                }
                PrintString(5, 10, 0x1, "======= ERROR! =======");
            }
        }
        else
        {
            break;
        }
    }

}
