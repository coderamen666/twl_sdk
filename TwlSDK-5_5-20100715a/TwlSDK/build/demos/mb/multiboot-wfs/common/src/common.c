/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - common
  File:     common.c

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-29#$
  $Rev: 8747 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    "common.h"
#include    "wfs.h"
#include    "wh.h"


/*****************************************************************************/
/* Constants */

/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         StateCallbackForWFS

  Description:  WFS callback function.
                This is called when the state became WFS_STATE_READY.
                WFS_GetStatus can be used at any position to make a determination without receiving notification with this callback.
                

  Arguments:    arg: User-defined argument specified in the callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateCallbackForWFS(void *arg)
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

  Arguments:    arg: User-defined argument specified in the allocator
                size: Required size if requesting memory allocation
                ptr: If NULL, allocates memory; Otherwise, requests deallocation

  Returns:      If ptr is NULL, the amount of memory in size is allocated and its pointer returned.
                If not, the ptr memory is released.
                If released, the return value is simply ignored.
 *---------------------------------------------------------------------------*/
void   *AllocatorForWFS(void *arg, u32 size, void *ptr)
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
  Name:         WCCallbackForWFS

  Description:  Callback that receives status notification from the WC's wireless automatic drive.

  Arguments:    arg: Callback pointer for the notification source WM function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WHCallbackForWFS(void *arg)
{
//    WCStatus wcStatus = WcGetStatus();
    switch (((WMCallback *)arg)->apiid)
    {
    case WM_APIID_START_MP:
        {                              /* Begin MP state */
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                /*
                 * Call WFS_Start to send a notification for a transition to the MP_PARENT or MP_CHILD state
                 
                 * Using this notification as a trigger, the WFS library begins to call the WM_SetMPDataToPort function internally to transfer blocks
                 
                 */
                WFS_Start();
                break;
            }
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitFrameLoop

  Description:  Initialization for game frame loop.

  Arguments:    p_key: Key information structure to initialize

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitFrameLoop(KeyInfo * p_key)
{
    ClearLine();
    ClearString();
    KeyRead(p_key);
}

/*---------------------------------------------------------------------------*
  Name:         WaitForNextFrame

  Description:  Wait until next drawing frame.

  Arguments:    p_key: Key information structure to update

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WaitForNextFrame(KeyInfo * p_key)
{
    FlushLine();
    OS_WaitVBlankIntr();
    ClearString();
    KeyRead(p_key);
}

/*---------------------------------------------------------------------------*
  Name:         ModeInitialize

  Description:  Initializes the wireless module.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeInitialize(void)
{
    // Use the WH module
    // The value of WH_GetSystemState changes from WH_SYSSTATE_STOP to WH_SYSSTATE_IDLE.
    if (!WH_Initialize())
    {
        OS_Panic("WH_Initialize failed.");
    }
    
    WH_SetGgid(GGID_WBT_FS);
    WH_SetSessionUpdateCallback(WHCallbackForWFS);
}

/*---------------------------------------------------------------------------*
  Name:         ModeError

  Description:  Processing in error display screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeError(void)
{
    KeyInfo key[1];

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_FATAL)
    {
        WaitForNextFrame(key);
        PrintString(5, 10, COLOR_RED, "======= ERROR! =======");
        PrintString(5, 13, COLOR_WHITE, " Fatal error occured.");
        PrintString(5, 14, COLOR_WHITE, "Please reboot program.");
    }
}

/*---------------------------------------------------------------------------*
  Name:         ModeWorking

  Description:  Processing in busy screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ModeWorking(void)
{
    KeyInfo key[1];

    InitFrameLoop(key);
    while (WH_GetSystemState() == WH_SYSSTATE_SCANNING || WH_GetSystemState() == WH_SYSSTATE_BUSY)
    {
        WaitForNextFrame(key);
        PrintString(9, 11, COLOR_WHITE, "Now working...");
        if (key->trg & PAD_BUTTON_START)
        {
            WH_Finalize();
        }
    }
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
