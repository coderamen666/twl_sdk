/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_etc.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     WM_SIZE_TEMP_USR_GAME_INFO_BUF 128


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u32 tmpUserGameInfoBuf[WM_SIZE_TEMP_USR_GAME_INFO_BUF / sizeof(u32)] ATTRIBUTE_ALIGN(32);


#ifdef  WM_ENABLE_TESTMODE
/*---------------------------------------------------------------------------*
  Name:         WM_StartTestMode

  Description:  Starts communication in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                signal: 0: No modulation (data=0), 1: No modulation (data=1), 2: PN15 sequence, 3: 01 pattern (with scrambling), 4: 01 pattern (without scrambling)
                                
                rate:            1: 1Mbps, 2: 2Mbps
                channel     -   Specifies the channel to send data (1 to 14).

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestMode(WMCallbackFunc callback, u16 signal, u16 rate, u16 channel)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_TESTMODE, callback);

    // Notify ARM7 with FIFO
    {
        WMStartTestModeReq Req;

        Req.apiid = WM_APIID_START_TESTMODE;
        Req.signal = signal;
        Req.rate = rate;
        Req.channel = channel;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        if (result != WM_ERRCODE_SUCCESS)
        {
            return result;
        }
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestMode

  Description:  Stops communication in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestMode(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot run unless state is TESTMODE
    result = WMi_CheckState(WM_STATE_TESTMODE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_STOP_TESTMODE, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_STOP_TESTMODE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StartTestRxMode

  Description:  Starts receiving data in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                channel     -   Specifies the channel for receiving data (1 to 14).

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestRxMode(WMCallbackFunc callback, u16 channel)
{
    WMErrCode result;

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_TESTRXMODE, callback);

    // Notify ARM7 with FIFO
    {
        WMStartTestRxModeReq Req;

        Req.apiid = WM_APIID_START_TESTRXMODE;
        Req.channel = channel;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        if (result != WM_ERRCODE_SUCCESS)
        {
            return result;
        }
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestRxMode

  Description:  Stops receiving data in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestRxMode(WMCallbackFunc callback)
{
    WMErrCode result;

    // Cannot run unless state is TESTMODE
    result = WMi_CheckState(WM_STATE_TESTMODE_RX);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_STOP_TESTRXMODE, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_STOP_TESTRXMODE, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKey

  Description:  Sets the encryption feature and encryption key.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                wepmode     -   0: No encryption feature.
                                1: RC4 (40bit) encryption mode.
                                2: RC4 (104bit) encryption mode.
                                3: RC4 (128bit) encryption mode.
                wepkey      -   Pointer to the encryption key data (80 bytes).
                                Key data consists of 4 pieces of data, each 20 bytes long.
                                Of each 20 byte piece, in 40-bit mode 5 bytes are used, in 104-bit mode 13 bytes, and in 128-bit mode 16 bytes are used.
                                 
                                
                                
                                
                                The data entity is forcibly stored in cache.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKey(WMCallbackFunc callback, u16 wepmode, const u16 *wepkey)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Check parameters
    if (wepmode > 3)
    {
        WM_WARNING("Parameter \"wepmode\" must be less than %d.\n", 3);
        return WM_ERRCODE_INVALID_PARAM;
    }

    if (wepmode != WM_WEPMODE_NO)
    {
        // Check parameters
        if (wepkey == NULL)
        {
            WM_WARNING("Parameter \"wepkey\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if ((u32)wepkey & 0x01f)
        {
            // Alignment check is a warning only, not an error
            WM_WARNING("Parameter \"wepkey\" is not 32-byte aligned.\n");
        }

        // Write out the specified buffer's cache
        DC_StoreRange((void *)wepkey, WM_SIZE_WEPKEY);
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_WEPKEY, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_WEPKEY, 2, (u32)wepmode, (u32)wepkey);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKeyEx

  Description:  Sets encryption functionality, encryption key, and encryption key ID.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                wepmode     -   0: No encryption feature.
                                1: RC4 (40bit) encryption mode.
                                2: RC4 (104bit) encryption mode.
                                3: RC4 (128bit) encryption mode.
                wepkeyid    -   Selects which of 4 specified wepkeys to use.
                                Specify using 0-3.
                wepkey      -   Pointer to the encryption key data (80 bytes).
                                Key data consists of 4 pieces of data, each 20 bytes long.
                                Of each 20-byte set, the following data will be used.
                                 5 bytes in 40-bit mode
                                13 bytes in 104-bit mode
                                16 bytes in 128-bit mode
                                
                                The data entity is forcibly stored in cache.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKeyEx(WMCallbackFunc callback, u16 wepmode, u16 wepkeyid, const u8 *wepkey)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Check parameters
    if (wepmode > 3)
    {
        WM_WARNING("Parameter \"wepmode\" must be less than %d.\n", 3);
        return WM_ERRCODE_INVALID_PARAM;
    }

    if (wepmode != WM_WEPMODE_NO)
    {
        // Check parameters
        if (wepkey == NULL)
        {
            WM_WARNING("Parameter \"wepkey\" must not be NULL.\n");
            return WM_ERRCODE_INVALID_PARAM;
        }
        if (wepkeyid > 3)
        {
            WM_WARNING("Parameter \"wepkeyid\" must be less than %d.\n", 3);
        }
        if ((u32)wepkey & 0x01f)
        {
            // Alignment check is a warning only, not an error
            WM_WARNING("Parameter \"wepkey\" is not 32-byte aligned.\n");
        }

        // Write out the specified buffer's cache
        DC_StoreRange((void *)wepkey, WM_SIZE_WEPKEY);
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_WEPKEY_EX, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_WEPKEY_EX, 3, (u32)wepmode, (u32)wepkey, (u32)wepkeyid);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetGameInfo

  Description:  Sets the game information. Initial value is set by WM_SetParentParameter. Use this function to change the game information.
                

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                userGameInfo     - Pointer to the user game information.
                userGameInfoSize - Size of the user game information.
                ggid        -   Game group ID
                tgid        -   Temporary group ID
                attr        -   Flag group. Sets the logical sum of the following flags.
                                    WM_ATTR_FLAG_ENTRY - Entry permitted
                                    WM_ATTR_FLAG_MB    - Accepting multiboot
                                    WM_ATTR_FLAG_KS    - Key sharing
                                    WM_ATTR_FLAG_CS    - Continuous transfer mode
  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode
WM_SetGameInfo(WMCallbackFunc callback, const u16 *userGameInfo, u16 userGameInfoSize,
               u32 ggid, u16 tgid, u8 attr)
{
    WMErrCode result;

    // Confirm status as active parent
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // Check parameters
    if (userGameInfo == NULL)
    {
        WM_WARNING("Parameter \"userGameInfo\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (userGameInfoSize > WM_SIZE_USER_GAMEINFO)
    {
        WM_WARNING("Parameter \"userGameInfoSize\" must be less than %d.\n", WM_SIZE_USER_GAMEINFO);
        return WM_ERRCODE_INVALID_PARAM;
    }

    // Temporarily save the specified buffer
    MI_CpuCopy16((void *)userGameInfo, (void *)tmpUserGameInfoBuf, userGameInfoSize);
    DC_StoreRange((void *)tmpUserGameInfoBuf, userGameInfoSize);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_GAMEINFO, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_GAMEINFO, 5,
                             (u32)tmpUserGameInfoBuf,
                             (u32)userGameInfoSize, (u32)ggid, (u32)tgid, (u32)attr);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetBeaconIndication

  Description:  Switches the beacon send/receive indication between enabled/disabled.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                flag        -   0: Disabled
                                1: Enabled

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetBeaconIndication(WMCallbackFunc callback, u16 flag)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Check parameters
    if ((0 != flag) && (1 != flag))
    {
        WM_WARNING("Parameter \"flag\" must be \"0\" of \"1\".\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_BEACON_IND, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_BEACON_IND, 1, (u32)flag);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetLifeTime

  Description:  Sets the lifetime.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                tableNumber:     CAM table number that sets lifetime (With 0xFFFF, all tables)
                camLifeTime: CAM lifetime (in 100-ms units; invalid when it is 0xFFFF)
                frameLifeTime: Beacon intervals for the configured frame lifetime (in 100-ms units; invalid when it is 0xFFFF)
                mpLifeTime: MP communication lifetime (in 100-ms units; invalid when it is 0xFFFF)

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode
WM_SetLifeTime(WMCallbackFunc callback, u16 tableNumber, u16 camLifeTime, u16 frameLifeTime,
               u16 mpLifeTime)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_LIFETIME, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_LIFETIME, 4,
                             (u32)tableNumber,
                             (u32)camLifeTime, (u32)frameLifeTime, (u32)mpLifeTime);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_MeasureChannel

  Description:  Measure channel usage status.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                ccaMode:        CCA operation mode.
                                0: Carrier sense only. ED threshold value is ignored.
                                1: Valid with ED threshold value only.
                                2: Logical product of carrier sense and ED threshold value.
                                3: Logical sum of carrier sense and ED threshold value.
                EDThreshold:     ED threshold (0 thru 61), -60dBm thru -80dBm
                channel:        Channel to investigate (Only one channel per one cycle of MeasureChannel)
                measureTime:     Time to investigate.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode
WM_MeasureChannel(WMCallbackFunc callback, u16 ccaMode, u16 edThreshold, u16 channel,
                  u16 measureTime)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Cannot execute if not in IDLE state.
    result = WMi_CheckState(WM_STATE_IDLE);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_MEASURE_CHANNEL, callback);

    // Notify ARM7 with FIFO
    {
        WMMeasureChannelReq Req;

        Req.apiid = WM_APIID_MEASURE_CHANNEL;
        Req.ccaMode = ccaMode;
        Req.edThreshold = edThreshold;
        Req.channel = channel;
        Req.measureTime = measureTime;

        result = WMi_SendCommandDirect(&Req, sizeof(Req));
        WM_CHECK_RESULT(result);
    }
    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_InitWirelessCounter

  Description:  Initializes WirelessCounter.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitWirelessCounter(WMCallbackFunc callback)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_INIT_W_COUNTER, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_INIT_W_COUNTER, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCounter

  Description:  Gets send/receive frame count and send/receive error frame count of Wireless NIC.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_GetWirelessCounter(WMCallbackFunc callback)
{
    WMErrCode result;

    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_GET_W_COUNTER, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_GET_W_COUNTER, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetEntry

  Description:  Switches between accepting and refusing a connection from a child as a parent.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                enabled:        Entry permitted / not permitted flag. TRUE: permitted, FALSE: not permitted.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetEntry(WMCallbackFunc callback, BOOL enabled)
{
    WMErrCode result;

    // Only a parent can execute
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_ENTRY, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_ENTRY, 1, (u32)enabled);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SetBeaconPeriod

  Description:  Changes the beacon intervals.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                beaconPeriod -  Beacon interval (10 to 1000 TU (1024 microseconds))

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SetBeaconPeriod(WMCallbackFunc callback, u16 beaconPeriod)
{
    WMErrCode result;

    // Only a parent can execute
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_MP_PARENT);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_BEACON_PERIOD, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_BEACON_PERIOD, 1, (u32)beaconPeriod);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetPowerSaveMode()

  Description:  Changes the PowerSaveMode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                powerSave   -   When using power save mode, TRUE. When not using it, FALSE.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPowerSaveMode(WMCallbackFunc callback, BOOL powerSave)
{
    WMErrCode result;

    // This function cannot be executed except when in the DCF child state
    result = WMi_CheckState(WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_PS_MODE, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_PS_MODE, 1, (u32)powerSave);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
