/*
  Project:  NitroSDK - CHT - demos - catch-min
  File:     wmscan.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: wmscan.h,v $
  Revision 1.2  2006/01/18 02:12:40  kitase_hirotake
  do-indent

  Revision 1.1  2005/07/27 07:44:06  seiki_masashi
  New additions

  $NoKeywords$
 */
#include <nitro/types.h>
#include <nitro/wm.h>

typedef void (*WSStartScanCallbackFunc) (WMBssDesc *bssDesc);

enum
{
    WS_SYSSTATE_STOP,                  // Wireless is stopped
    WS_SYSSTATE_IDLE,                  // IDLE state
    WS_SYSSTATE_SCANNING,              // Scanning
    WS_SYSSTATE_SCANIDLE,              // Scanning is stopped
    WS_SYSSTATE_BUSY,                  // Busy
    WS_SYSSTATE_ERROR,                 // Error occurred
    WS_SYSSTATE_FATAL                  // A FATAL error has occurred
};


/*---------------------------------------------------------------------------*
  Name:         WS_Initialize
  Description:  Initializes wireless.
  Arguments:    buf:   Buffer size passed to WM. A region the size of WM_SYSTEM_BUF_SIZE required.
                dmaNo: The DMA number used by wireless.
  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WS_Initialize(void *buf, u16 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         WS_StartScan
  Description:  Starts scanning for a parent.
  Arguments:    callback:  Callback when parent is found.
                macAddr:  Specifies the parent's MAC address.
                         When searching for all parents, specify FF:FF:FF:FF:FF:FF.
                If the 'continuous' flag is set to TRUE, until WS_EndScan is called,
                           all valid channels will continue to be scanned.
                           If FALSE, while cycling between valid channels,
                           it will stop scanning after each scan and transition to the WS_SYSSTATE_SCANIDLE state.
  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WS_StartScan(WSStartScanCallbackFunc callback, const u8 *macAddr, BOOL continuous);

/*---------------------------------------------------------------------------*
  Name:         WS_EndScan
  Description:  Function that terminates scanning.
  Arguments:    None.
  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WS_EndScan(void);

/*---------------------------------------------------------------------------*
  Name:         WS_End
  Description:  Ends wireless communication.
  Arguments:    None.
  Returns:      If it succeeds returns TRUE.
 *---------------------------------------------------------------------------*/
BOOL    WS_End(void);

/*---------------------------------------------------------------------------*
  Name:         WS_SetGgid
  Description:  Sets the game's group ID.
                Call before making a connection to the parent device.
  Arguments:    ggid    The game group ID to configure.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WS_SetGgid(u32 ggid);

/*---------------------------------------------------------------------------*
  Name:         WS_GetSystemState
  Description:  Gets the WS state.
  Arguments:    None.
  Returns:      The WS state.
 *---------------------------------------------------------------------------*/
int     WS_GetSystemState(void);

/*---------------------------------------------------------------------------*
  Name:         WS_TurnOnPictoCatch
  Description:  Enables the Pictocatch feature.
                This causes a callback function to be invoked when PictoChat is discovered while scanning with WS_StartScan.
                
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WS_TurnOnPictoCatch(void);

/*---------------------------------------------------------------------------*
  Name:         WS_TurnOffPictoCatch
  Description:  Disables the pictocatch feature.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WS_TurnOffPictoCatch(void);
