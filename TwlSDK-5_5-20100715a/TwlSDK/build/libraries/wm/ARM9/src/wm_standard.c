/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_standard.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-19#$
  $Rev: 9342 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static BOOL WmCheckParentParameter(const WMParentParam *param);


/*---------------------------------------------------------------------------*
  Name:         WM_Enable

  Description:  Makes the wireless hardware to the usable state.
                Internal state changes from READY state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Enable(WMCallbackFunc callback)
{
    return WMi_EnableEx(callback, 0);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EnableForListening

  Description:  Makes the wireless hardware to the usable state.
                Internal state changes from READY state to STOP state.
                Operations that require sending radio signals are not available.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                blink       -   Whether to flash the LED

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EnableForListening(WMCallbackFunc callback, BOOL blink)
{
    u32 miscFlags = WM_MISC_FLAG_LISTEN_ONLY;

    if (!blink)
    {
        miscFlags |= WM_MISC_FLAG_NO_BLINK;
    }

    return WMi_EnableEx(callback, miscFlags);
}

/*---------------------------------------------------------------------------*
  Name:         WMi_EnableEx

  Description:  Makes the wireless hardware to the usable state.
                Internal state changes from READY state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                miscFlags   -   Flags during initialization

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_EnableEx(WMCallbackFunc callback, u32 miscFlags)
{
    WMErrCode result;

    // Cannot execute if not in READY state
    result = WMi_CheckState(WM_STATE_READY);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_ENABLE, callback);
    
    // Register a callback to prohibit sleeping during communications.
    WMi_RegisterSleepCallback();

    // Notify ARM7 with FIFO
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        result = WMi_SendCommand(WM_APIID_ENABLE, 4,
                                 (u32)(p->WM7), (u32)(p->status), (u32)(p->fifo7to9), miscFlags);
        WM_CHECK_RESULT(result);
    }

    // Normal end
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Disable

  Description:  Changes the wireless hardware to the use prohibited state.
                Internal state changes from STOP state to READY state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disable(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in STOP state
    result = WMi_CheckState(WM_STATE_STOP);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_DISABLE, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_DISABLE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOn

  Description:  Starts up the wireless hardware.
                Internal state changes from STOP state to IDLE state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOn(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in STOP state
    result = WMi_CheckState(WM_STATE_STOP);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_POWER_ON, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_POWER_ON, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOff

  Description:  Shuts down the wireless hardware.
                Internal state changes from IDLE state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOff(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_POWER_OFF, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_POWER_OFF, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Initialize

  Description:  Performs the WM initialization process.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                callback    -   Callback function that is called when the asynchronous process completes.
                dmaNo       -   DMA number used by WM

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Initialize(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo)
{
    return WMi_InitializeEx(wmSysBuf, callback, dmaNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         WM_InitializeForListening

  Description:  Performs the WM initialization process.
                Operations that require sending radio signals are not available.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                callback    -   Callback function that is called when the asynchronous process completes.
                dmaNo       -   DMA number used by WM
                blink       -   Whether to flash the LED

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitializeForListening(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, BOOL blink)
{
    u32 miscFlags = WM_MISC_FLAG_LISTEN_ONLY;

    if (!blink)
    {
        miscFlags |= WM_MISC_FLAG_NO_BLINK;
    }

    return WMi_InitializeEx(wmSysBuf, callback, dmaNo, miscFlags);
}

/*---------------------------------------------------------------------------*
  Name:         WMi_InitializeEx

  Description:  Performs the WM initialization process.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                callback    -   Callback function that is called when the asynchronous process completes.
                dmaNo       -   DMA number used by WM
                miscFlags   -   Flags during initialization

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_InitializeEx(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, u32 miscFlags)
{
    WMErrCode result;

    // Execute the initialization
    result = WM_Init(wmSysBuf, dmaNo);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_INITIALIZE, callback);

    // Register a callback to prohibit sleeping during communications.
    WMi_RegisterSleepCallback();

    // Notify ARM7 with FIFO
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        result = WMi_SendCommand(WM_APIID_INITIALIZE, 4,
                                 (u32)(p->WM7), (u32)(p->status), (u32)(p->fifo7to9), miscFlags);
        WM_CHECK_RESULT(result);
    }

    // Normal end
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Reset

  Description:  Resets the wireless library, and restores the state immediately after the initialization.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Reset(WMCallbackFunc callback)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_RESET, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_RESET, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_End

  Description:  Closes the wireless library.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_End(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_END, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_END, 0);
    WM_CHECK_RESULT(result);

    // On the ARM9 side, perform the processing for ending the WM library inside the callback.

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetParentParameter

  Description:  Sets the parent device information.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                pparaBuf    -   Pointer to the structure that indicates the parent information.
                                Note that the pparaBuf and pparaBuf->userGameInfo objects will be forcibly stored in the cache.
                                

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetParentParameter(WMCallbackFunc callback, const WMParentParam *pparaBuf)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);
    
    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless.
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // Check parameters
    if (pparaBuf == NULL)
    {
        WM_WARNING("Parameter \"pparaBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)pparaBuf & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"pparaBuf\" is not 32-byte aligned.\n");
    }
    if (pparaBuf->userGameInfoLength > 0)
    {
        if (pparaBuf->userGameInfo == NULL)
        {
            WM_WARNING("Parameter \"pparaBuf->userGameInfo\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if ((u32)(pparaBuf->userGameInfo) & 0x01f)
        {
            // Alignment check is a warning only, not an error
            WM_WARNING("Parameter \"pparaBuf->userGameInfo\" is not 32-byte aligned.\n");
        }
    }

    // Check maximum data transfer length
    if ((pparaBuf->parentMaxSize +
         (pparaBuf->KS_Flag ? WM_SIZE_KS_PARENT_DATA + WM_SIZE_MP_PARENT_PADDING : 0) >
         WM_SIZE_MP_DATA_MAX)
        || (pparaBuf->childMaxSize +
            (pparaBuf->KS_Flag ? WM_SIZE_KS_CHILD_DATA + WM_SIZE_MP_CHILD_PADDING : 0) >
            WM_SIZE_MP_DATA_MAX))
    {
        WM_WARNING("Transfer data size is over %d byte.\n", WM_SIZE_MP_DATA_MAX);
        return WM_ERRCODE_INVALID_PARAM;
    }
    (void)WmCheckParentParameter(pparaBuf);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_P_PARAM, callback);

    // Write out the specified buffer's cache
    DC_StoreRange((void *)pparaBuf, WM_PARENT_PARAM_SIZE);
    if (pparaBuf->userGameInfoLength > 0)
    {
        DC_StoreRange(pparaBuf->userGameInfo, pparaBuf->userGameInfoLength);
    }

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_P_PARAM, 1, (u32)pparaBuf);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WmCheckParentParameter

  Description:  This is a debugging function that checks whether parameters for the parent settings fall within the admissible range given by the guidelines.
                

  Arguments:    param:       Pointer to parent settings parameters to check.

  Returns:      BOOL:        Returns TRUE if no problems, or FALSE if settings values are not admissible.
                            
 *---------------------------------------------------------------------------*/
static BOOL WmCheckParentParameter(const WMParentParam *param)
{
    // User game information can be up to 112 bytes
    if (param->userGameInfoLength > WM_SIZE_USER_GAMEINFO)
    {
        OS_TWarning("User gameInfo length must be less than %d .\n", WM_SIZE_USER_GAMEINFO);
        return FALSE;
    }
    // Beacon transmission interval from 10 to 1000
    if ((param->beaconPeriod < 10) || (param->beaconPeriod > 1000))
    {
        OS_TWarning("Beacon send period must be between 10 and 1000 .\n");
        return FALSE;
    }
    // Connection channels are 1 to 14
    if ((param->channel < 1) || (param->channel > 14))
    {
        OS_TWarning("Channel must be between 1 and 14 .\n");
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_StartParentEx

  Description:  Starts the communication as parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                powerSave   -   When using power save mode, TRUE. When not using it, FALSE.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_StartParentEx(WMCallbackFunc callback, BOOL powerSave)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless.
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    {
        WMArm9Buf *w9b = WMi_GetSystemWork();
#ifdef WM_DEBUG
        if (w9b->connectedAidBitmap != 0)
        {
            WM_DPRINTF("Warning: connectedAidBitmap should be 0, but %04x",
                       w9b->connectedAidBitmap);
        }
#endif
        w9b->myAid = 0;
        w9b->connectedAidBitmap = 0;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_PARENT, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_START_PARENT, 1, (u32)powerSave);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartParent

  Description:  Starts the communication as parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartParent(WMCallbackFunc callback)
{
    return WMi_StartParentEx(callback, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndParent

  Description:  Stops the communication as a parent.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndParent(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in PARENT state
    result = WMi_CheckState(WM_STATE_PARENT);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_END_PARENT, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_END_PARENT, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartScan

  Description:  As a child device, starts scanning for a parent device.
                Obtains one device's parent information with each call.
                It can be called repeatedly without calling WM_EndScan.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                param       -   Pointer to the structure that shows the scan information.
                                The ARM7 directly writes scan result information to param->scanBuf, so it must match the cache line.
                                

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartScan(WMCallbackFunc callback, const WMScanParam *param)
{
    WMErrCode result;

    // Not executable outside of IDLE CALSS1 SCAN state
    result = WMi_CheckStateEx(3, WM_STATE_IDLE, WM_STATE_CLASS1, WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless.
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // Check parameters
    if (param == NULL)
    {
        WM_WARNING("Parameter \"param\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBuf == NULL)
    {
        WM_WARNING("Parameter \"param->scanBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((param->channel < 1) || (param->channel > 14))
    {
        WM_WARNING("Parameter \"param->channel\" must be between 1 and 14.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)(param->scanBuf) & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"param->scanBuf\" is not 32-byte aligned.\n");
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_SCAN, callback);

    // Notify ARM7 with FIFO
    {
        WMStartScanReq Req;

        Req.apiid = WM_APIID_START_SCAN;
        Req.channel = param->channel;
        Req.scanBuf = param->scanBuf;
        Req.maxChannelTime = param->maxChannelTime;
        Req.bssid[0] = param->bssid[0];
        Req.bssid[1] = param->bssid[1];
        Req.bssid[2] = param->bssid[2];
        Req.bssid[3] = param->bssid[3];
        Req.bssid[4] = param->bssid[4];
        Req.bssid[5] = param->bssid[5];
        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartScanEx

  Description:  As a child device, starts scanning for a parent device.
                Obtains multiple devices' parent information with one call.
                It can be called repeatedly without calling WM_EndScan.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                param       -   Pointer to the structure that shows the scan information.
                           The ARM7 directly writes scan result information to param->scanBuf, so it must match the cache line.
                           

  Returns:      int:       Returns WM_ERRCODE_* type processing results.
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartScanEx(WMCallbackFunc callback, const WMScanExParam *param)
{
    WMErrCode result;

    // Not executable outside of IDLE CALSS1 SCAN state
    result = WMi_CheckStateEx(3, WM_STATE_IDLE, WM_STATE_CLASS1, WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless.
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // Check parameters
    if (param == NULL)
    {
        WM_WARNING("Parameter \"param\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBuf == NULL)
    {
        WM_WARNING("Parameter \"param->scanBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanBufSize > WM_SIZE_SCAN_EX_BUF)
    {
        WM_WARNING
            ("Parameter \"param->scanBufSize\" must be less than or equal to WM_SIZE_SCAN_EX_BUF.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)(param->scanBuf) & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"param->scanBuf\" is not 32-byte aligned.\n");
    }
    if (param->ssidLength > WM_SIZE_SSID)
    {
        WM_WARNING("Parameter \"param->ssidLength\" must be less than or equal to WM_SIZE_SSID.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (param->scanType != WM_SCANTYPE_ACTIVE && param->scanType != WM_SCANTYPE_PASSIVE
        && param->scanType != WM_SCANTYPE_ACTIVE_CUSTOM
        && param->scanType != WM_SCANTYPE_PASSIVE_CUSTOM)
    {
        WM_WARNING
            ("Parameter \"param->scanType\" must be WM_SCANTYPE_PASSIVE or WM_SCANTYPE_ACTIVE.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((param->scanType == WM_SCANTYPE_ACTIVE_CUSTOM
         || param->scanType == WM_SCANTYPE_PASSIVE_CUSTOM) && param->ssidMatchLength > WM_SIZE_SSID)
    {
        WM_WARNING
            ("Parameter \"param->ssidMatchLength\" must be less than or equal to WM_SIZE_SSID.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_SCAN_EX, callback);

    // Notify ARM7 with FIFO
    {
        WMStartScanExReq Req;

        Req.apiid = WM_APIID_START_SCAN_EX;
        Req.channelList = param->channelList;
        Req.scanBuf = param->scanBuf;
        Req.scanBufSize = param->scanBufSize;
        Req.maxChannelTime = param->maxChannelTime;
        MI_CpuCopy8(param->bssid, Req.bssid, WM_SIZE_MACADDR);
        Req.scanType = param->scanType;
        Req.ssidMatchLength = param->ssidMatchLength;
        Req.ssidLength = param->ssidLength;
        MI_CpuCopy8(param->ssid, Req.ssid, WM_SIZE_SSID);

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndScan

  Description:  Stops the scan process as a child device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndScan(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot execute if not in SCAN state
    result = WMi_CheckState(WM_STATE_SCAN);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_END_SCAN, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_END_SCAN, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartConnectEx

  Description:  Starts the connection to the parent as a child device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                pInfo       -   Information of the parent to connect to.
                                Specifies the structure obtained with WM_StartScan.
                                Note that this structure is forcibly stored in the cache.
                                
                ssid        -   Child information to notify to the parent (24Byte(WM_SIZE_CHILD_SSID) fixed size)
                powerSave   -   When using power save mode, TRUE. When not using it, FALSE.
                authMode    -   Authentication mode selection.
                                  WM_AUTHMODE_OPEN_SYSTEM : OPEN SYSTEM mode
                                  WM_AUTHMODE_SHARED_KEY  : SHARED KEY mode

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode
WM_StartConnectEx(WMCallbackFunc callback, const WMBssDesc *pInfo, const u8 *ssid,
                  BOOL powerSave, const u16 authMode)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless.
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return WM_ERRCODE_WM_DISABLE;
        }
    }
    #endif
    
    // Check parameters
    if (pInfo == NULL)
    {
        WM_WARNING("Parameter \"pInfo\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)pInfo & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"pInfo\" is not 32-byte aligned.\n");
    }
    if ((authMode != WM_AUTHMODE_OPEN_SYSTEM) && (authMode != WM_AUTHMODE_SHARED_KEY))
    {
        WM_WARNING
            ("Parameter \"authMode\" must be WM_AUTHMODE_OPEN_SYSTEM or WM_AUTHMODE_SHARED_KEY.\n");
    }

    // Write out the specified buffer's cache
    DC_StoreRange((void *)pInfo, (u32)(pInfo->length * 2));

    {
        WMArm9Buf *w9b = WMi_GetSystemWork();
#ifdef WM_DEBUG
        if (w9b->connectedAidBitmap != 0)
        {
            WM_DPRINTF("Warning: connectedAidBitmap should be 0, but %04x",
                       w9b->connectedAidBitmap);
        }
#endif
        w9b->myAid = 0;
        w9b->connectedAidBitmap = 0;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_CONNECT, callback);

    // Notify ARM7 with FIFO
    {
        WMStartConnectReq Req;

        Req.apiid = WM_APIID_START_CONNECT;
        Req.pInfo = (WMBssDesc *)pInfo;
        if (ssid != NULL)
        {
            MI_CpuCopy8(ssid, Req.ssid, WM_SIZE_CHILD_SSID);
        }
        else
        {
            MI_CpuClear8(Req.ssid, WM_SIZE_CHILD_SSID);
        }
        Req.powerSave = powerSave;
        Req.authMode = authMode;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Disconnect

  Description:  Cuts off the connection that had been established.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                aid         -   AID of the communication partner to be disconnected.
                                In the parent's case, individually disconnects children having IDs 1 - 15.
                                In the child's case, ends communication with the parent having ID 0.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disconnect(WMCallbackFunc callback, u16 aid)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckStateEx(5,
                              WM_STATE_PARENT, WM_STATE_MP_PARENT,
                              WM_STATE_CHILD, WM_STATE_MP_CHILD, WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    if (                               // p->status->state cache has been discarded
           (p->status->state == WM_STATE_PARENT) || (p->status->state == WM_STATE_MP_PARENT))
    {
        // For parent
        if ((aid < 1) || (aid > WM_NUM_MAX_CHILD))
        {
            WM_WARNING("Parameter \"aid\" must be between 1 and %d.\n", WM_NUM_MAX_CHILD);
            return WM_ERRCODE_INVALID_PARAM;
        }
        DC_InvalidateRange(&(p->status->child_bitmap), 2);
        if (!(p->status->child_bitmap & (0x0001 << aid)))
        {
            WM_WARNING("There is no child that have aid %d.\n", aid);
            return WM_ERRCODE_NO_CHILD;
        }
    }
    else
    {
        // For child
        if (aid != 0)
        {
            WM_WARNING("Now child mode , so aid must be 0.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_DISCONNECT, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_DISCONNECT, 1, (u32)(0x0001 << aid));
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_DisconnectChildren

  Description:  Disconnects the respective children with which the connection has been established. Function exclusively for parent use.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                aidBitmap   -   AID bitfield of the children to be disconnected.
                                The lowest order bit will be ignored. Bits 1-15 indicate AID 1-15, respectively.
                                Bits that indicate disconnected child devices will be ignored, so specify 0xFFFF to disconnect all children regardless of their connection state.
                                
                                

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_DisconnectChildren(WMCallbackFunc callback, u16 aidBitmap)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // Check parameters
    DC_InvalidateRange(&(p->status->child_bitmap), 2);
    if (!(p->status->child_bitmap & aidBitmap & 0xfffe))
    {
        WM_WARNING("There is no child that is included in \"aidBitmap\" %04x_.\n", aidBitmap);
        return WM_ERRCODE_NO_CHILD;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_DISCONNECT, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_DISCONNECT, 1, (u32)aidBitmap);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
