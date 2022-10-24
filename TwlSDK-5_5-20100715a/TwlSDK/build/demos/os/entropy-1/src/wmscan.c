/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CHT - demos - wmscan
  File:     chtmin.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include    <nitro.h>
#include    "wmscan.h"

// TODO: Not currently supported because the CHT library has not yet been implemented.
//#define TWL_SUPPORT_CHT
#if defined(TWL_SUPPORT_CHT)
#include    <nitro/cht.h>
#endif

static int sSysState = WS_SYSSTATE_STOP;        // wmscan internal state
static WSStartScanCallbackFunc sScanCallback = NULL;    // Callback pointer when parent is discovered
static WMBssDesc sBssDesc ATTRIBUTE_ALIGN(32);  // Buffer to obtain parent data
static WMScanParam sScanParam ATTRIBUTE_ALIGN(32);      // Scan parameter structure
static BOOL sContinuousScanFlag;       // Continuous scan flag
static BOOL sPictoCatchFlag = FALSE;   // PictoCatch flag
static u32 sGgid = 0;                  // GGID

#define WS_ASSERT(exp) \
    (void) ((exp) || (OSi_Panic(__FILE__, __LINE__, "Failed assertion " #exp), 0))

static void WS_ChangeSysState(int state);
static void WS_StateOutInitialize(void *arg);
static BOOL WS_StateInStartScan(void);
static void WS_StateOutStartScan(void *arg);
static BOOL WS_StateInEndScan(void);
static void WS_StateOutEndScan(void *arg);
static void WS_StateOutEnd(void *arg);


/*---------------------------------------------------------------------------*
  Name:         WS_ChangeSysState
  Description:  Changes the WS status.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WS_ChangeSysState(int state)
{
    sSysState = state;
}

/*---------------------------------------------------------------------------*
  Name:         WS_GetSystemState

  Description:  Obtains the WS state.

  Arguments:    None.

  Returns:      The WS state.
 *---------------------------------------------------------------------------*/
int WS_GetSystemState(void)
{
    return sSysState;
}

/*---------------------------------------------------------------------------*
  Name:         WS_Initialize

  Description:  Initializes wireless.

  Arguments:    buf: Buffer size passed to WM. A region the size of WM_SYSTEM_BUF_SIZE required.
                dmaNo: The DMA number used by wireless.

  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL WS_Initialize(void *buf, u16 dmaNo)
{
    WMErrCode result;

    SDK_NULL_ASSERT(buf);

    WS_ChangeSysState(WS_SYSSTATE_BUSY);
    result = WM_Initialize(buf, WS_StateOutInitialize, dmaNo);
    if (result != WM_ERRCODE_OPERATING)
    {
        WS_ChangeSysState(WS_SYSSTATE_FATAL);
        return FALSE;
    }
    sScanParam.channel = 0;
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         WS_StateOutInitialize
  Description:  Initializes wireless.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WS_StateOutInitialize(void *arg)
{
    // State after power-on
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WS_ChangeSysState(WS_SYSSTATE_FATAL);
        return;
    }

    // Changes the system state to idle (waiting)
    WS_ChangeSysState(WS_SYSSTATE_IDLE);

    // Does not set the next state, so the sequence ends here
}

/*---------------------------------------------------------------------------*
  Name:         WS_TurnOnPictoCatch

  Description:  Enables the Pictocatch feature.
                This will cause the callback function to be invoked when Pictochat is found while scanning with WS_StartScan.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WS_TurnOnPictoCatch(void)
{
    sPictoCatchFlag = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WS_TurnOffPictoCatch

  Description:  Disables the pictocatch feature.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WS_TurnOffPictoCatch(void)
{
    sPictoCatchFlag = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         WS_SetGgid

  Description:  Sets the game group ID.
                Call before making a connection to the parent device.

  Arguments:    ggid: The game group ID to configure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WS_SetGgid(u32 ggid)
{
    sGgid = ggid;
}


/*---------------------------------------------------------------------------*
  Name:         WS_StartScan

  Description:  Starts scanning for a parent.

  Arguments:    callback: Callback when parent is found
                macAddr: Specifies the parent's MAC address
                           When searching for all parents, specify FF:FF:FF:FF:FF:FF
                continuous: If this flag is set to TRUE, all valid channels will continue to be scanned until WS_EndScan is called
                           
                           If FALSE, while cycling between valid channels, it will stop scanning after each scan and transition to the WS_SYSSTATE_SCANIDLE state.
                           

  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL WS_StartScan(WSStartScanCallbackFunc callback, const u8 *macAddr, BOOL continuous)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    sScanCallback = callback;
    sContinuousScanFlag = continuous;

    // Sets the conditions for the MAC address to be searched for
    *(u16 *)(&sScanParam.bssid[4]) = *(u16 *)(macAddr + 4);
    *(u16 *)(&sScanParam.bssid[2]) = *(u16 *)(macAddr + 2);
    *(u16 *)(&sScanParam.bssid[0]) = *(u16 *)(macAddr);

    (void)OS_RestoreInterrupts(enabled);

    if (sSysState == WS_SYSSTATE_SCANNING)
    {
        return TRUE;
    }

    WS_ChangeSysState(WS_SYSSTATE_SCANNING);

    if (!WS_StateInStartScan())
    {
        WS_ChangeSysState(WS_SYSSTATE_ERROR);
        return FALSE;
    }

    return TRUE;
}

/* ----------------------------------------------------------------------
  state: StartScan
  ---------------------------------------------------------------------- */
static BOOL WS_StateInStartScan(void)
{
    // When in this state, looks for a parent
    WMErrCode result;
    u16     chanpat;

    WS_ASSERT(sSysState == WS_SYSSTATE_SCANNING);

    chanpat = WM_GetAllowedChannel();

    // Checks if wireless is usable
    if (chanpat == 0x8000)
    {
        // If 0x8000 is returned, it indicates that wireless is not initialized or there is some other abnormality with the wireless library. Therefore, set to an error.
        //  
        return FALSE;
    }
    if (chanpat == 0)
    {
        // In this state, wireless cannot be used
        return FALSE;
    }

    /* Search possible channels in ascending order from the current designation */
    while (TRUE)
    {
        sScanParam.channel++;
        if (sScanParam.channel > 16)
        {
            sScanParam.channel = 1;
        }

        if (chanpat & (0x0001 << (sScanParam.channel - 1)))
        {
            break;
        }
    }

    sScanParam.maxChannelTime = WM_GetDispersionScanPeriod();
    sScanParam.scanBuf = &sBssDesc;
    result = WM_StartScan(WS_StateOutStartScan, &sScanParam);

    if (result != WM_ERRCODE_OPERATING)
    {
        return FALSE;
    }
    return TRUE;
}

static void WS_StateOutStartScan(void *arg)
{
    WMstartScanCallback *cb = (WMstartScanCallback *)arg;

    // If the scan command fails
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WS_ChangeSysState(WS_SYSSTATE_ERROR);
        return;
    }

    if (sSysState != WS_SYSSTATE_SCANNING)
    {
        // End scan if the state has been changed
        if (!WS_StateInEndScan())
        {
            WS_ChangeSysState(WS_SYSSTATE_ERROR);
        }
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_PARENT_NOT_FOUND:
        break;

    case WM_STATECODE_PARENT_FOUND:
        // If a parent is found
        // Compares ggids and fails if different.
        // First, you must confirm WMBssDesc.gameInfoLength, and check to see if ggid has a valid value.
        //  

        // Discards the BssDesc cache that is set in the buffer, because the BssDesc information is written from the ARM7
        //  
        DC_InvalidateRange(&sBssDesc, sizeof(WMbssDesc));

#if defined(TWL_SUPPORT_CHT)
        // Determines if the parent is using pictochat
        if (sPictoCatchFlag)
        {
            if (CHT_IsPictochatParent(&sBssDesc))
            {
                // If the parent is using pictochat
                if (sScanCallback != NULL)
                {
                    sScanCallback(&sBssDesc);
                }
                break;
            }
        }
#endif

        if (cb->gameInfoLength < 8 || cb->gameInfo.ggid != sGgid)
        {
            // If GGIDs are different, this is ignored
            break;
        }

        // If the entry flag is not up, the child is not receiving, so this is ignored
        if (!(cb->gameInfo.gameNameCount_attribute & WM_ATTR_FLAG_ENTRY))
        {
            break;
        }

        // Call if callback is necessary
        if (sScanCallback != NULL)
        {
            sScanCallback(&sBssDesc);
        }

        break;
    }

    if (!sContinuousScanFlag)
    {
        WS_ChangeSysState(WS_SYSSTATE_SCANIDLE);
        return;
    }

    // Changes the channel and starts another scan
    if (!WS_StateInStartScan())
    {
        WS_ChangeSysState(WS_SYSSTATE_ERROR);
    }
}


/*---------------------------------------------------------------------------*
  Name:         WS_EndScan

  Description:  Function that terminates scanning

  Arguments:    None.

  Returns:      If the process starts successfully, TRUE.
                If it did not start, FALSE.
 *---------------------------------------------------------------------------*/
BOOL WS_EndScan(void)
{
    if (sSysState == WS_SYSSTATE_SCANIDLE)
    {
        return WS_StateInEndScan();
    }

    if (sSysState != WS_SYSSTATE_SCANNING)
    {
        return FALSE;
    }

    {
        OSIntrMode enabled = OS_DisableInterrupts();
        sScanCallback = NULL;
        (void)OS_RestoreInterrupts(enabled);
    }

    WS_ChangeSysState(WS_SYSSTATE_BUSY);
    return TRUE;
}


static BOOL WS_StateInEndScan(void)
{
    WMErrCode result;

    // In this state, scan end processing is carried out
    result = WM_EndScan(WS_StateOutEndScan);
    if (result != WM_ERRCODE_OPERATING)
    {
        return FALSE;
    }

    return TRUE;
}

static void WS_StateOutEndScan(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        return;
    }

    WS_ChangeSysState(WS_SYSSTATE_IDLE);
}


/*---------------------------------------------------------------------------*
  Name:         WS_End

  Description:  Ends wireless communications.

  Arguments:    None.

  Returns:      If it succeeds returns TRUE.
 *---------------------------------------------------------------------------*/
BOOL WS_End(void)
{
    WS_ASSERT(sSysState == WS_SYSSTATE_IDLE);

    WS_ChangeSysState(WS_SYSSTATE_BUSY);
    if (WM_End(WS_StateOutEnd) != WM_ERRCODE_OPERATING)
    {
        WS_ChangeSysState(WS_SYSSTATE_ERROR);

        return FALSE;
    }
    return TRUE;
}

/* ----------------------------------------------------------------------
  state: WS_StateOutEnd
  ---------------------------------------------------------------------- */
static void WS_StateOutEnd(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        WS_ChangeSysState(WS_SYSSTATE_FATAL);
        return;
    }
    WS_ChangeSysState(WS_SYSSTATE_STOP);
}
