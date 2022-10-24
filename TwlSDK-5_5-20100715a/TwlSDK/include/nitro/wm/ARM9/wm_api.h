/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - include
  File:     wm_api.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_WM_ARM9_WM_API_H_
#define NITRO_WM_ARM9_WM_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     WM_DEFAULT_BEACON_PERIOD    200     // ms
#define     WM_DEFAULT_SCAN_PERIOD      30      // ms
#define     WM_MAX_MP_PACKET_TIME       5600    // US


/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WM_Init

  Description:  Performs the initialization process for the WM library.
                Synchronous function that only performs initialization on the ARM9 side.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                dmaNo       -   DMA number used by WM

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_Init(void *wmSysBuf, u16 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         WM_Finish

  Description:  Performs termination processing for the WM library. Synchronous function.
                Restores the state before WM_Init function was called.

  Arguments:    None.

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_Finish(void);

/*---------------------------------------------------------------------------*
  Name:         WM_Enable

  Description:  Puts the wireless hardware into a usable state.
                Internal state changes from READY state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Enable(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_EnableForListening

  Description:  Puts the wireless hardware into a usable state.
                Internal state changes from READY state to STOP state.
                Operations that require sending radio signals are not available.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                blink       -   Whether to flash the LED

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EnableForListening(WMCallbackFunc callback, BOOL blink);

/*---------------------------------------------------------------------------*
  Name:         WMi_EnableEx

  Description:  Puts the wireless hardware into a usable state.
                Internal state changes from READY state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                miscFlags   -   Flags during initialization

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_EnableEx(WMCallbackFunc callback, u32 miscFlags);

/*---------------------------------------------------------------------------*
  Name:         WM_Disable

  Description:  Changes the wireless hardware to a use prohibited state.
                Internal state changes from STOP state to READY state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disable(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOn

  Description:  Starts up the wireless hardware.
                Internal state changes from STOP state to IDLE state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOn(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_PowerOff

  Description:  Shuts down the wireless hardware.
                Internal state changes from IDLE state to STOP state.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_PowerOff(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_Initialize

  Description:  Performs the WM initialization process.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                callback    -   Callback function that is called when the asynchronous process completes.
                dmaNo       -   DMA number used by WM

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Initialize(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         WM_InitializeForListening

  Description:  Performs the WM initialization process.
                Only scanning communications can be performed.

  Arguments:    wmSysBuf    -   Pointer to the buffer allocated by the caller.
                                Only as much as WM_SYSTEM_BUF_SIZE is required for buffer size.
                callback    -   Callback function that is called when the asynchronous process completes.
                dmaNo       -   DMA number used by WM
                blink       -   Whether to blink the LED

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitializeForListening(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, BOOL blink);

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
WMErrCode WMi_InitializeEx(void *wmSysBuf, WMCallbackFunc callback, u16 dmaNo, u32 miscFlags);

/*---------------------------------------------------------------------------*
  Name:         WM_SetIndCallback

  Description:  Sets a function that is called to the status notification from WM7.
                This function itself is a synchronous function.

  Arguments:    callback    -   Specifies the callback function called during status notification from the wireless firmware.
                                

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetIndCallback(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_SetPortCallback

  Description:  Sets a function that is called for the communication frame reception notification from WM7.

  Arguments:    port        -   Port number
                callback: Callback function that will be invoked when a receive notification occurs
                arg         -   Argument passed to the callback function as WMPortRecvCallback.arg
                                

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPortCallback(u16 port, WMCallbackFunc callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         WM_Reset

  Description:  Resets the wireless library, and restores the state immediately after the initialization.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Reset(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_End

  Description:  Closes the wireless library.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_End(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_ReadStatus

  Description:  Obtains the structure that indicates the status of the wireless library. Synchronous function.

  Arguments:    statusBuf   -   Pointer to the variable that obtains the status.

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_ReadStatus(WMStatus *statusBuf);

/*---------------------------------------------------------------------------*
  Name:         WM_SetParentParameter

  Description:  Sets the parent device information.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                pparaBuf    -   Pointer to the structure that indicates the parent information.
                                Note that the pparaBuf and pparaBuf->userGameInfo objects will be forcibly stored in the cache.
                                

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetParentParameter(WMCallbackFunc callback, const WMParentParam *pparaBuf);

/*---------------------------------------------------------------------------*
  Name:         WMi_StartParent

  Description:  Starts communication as the parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                powerSave   -   When using power save mode, TRUE. When not using it, FALSE.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_StartParentEx(WMCallbackFunc callback, BOOL powerSave);

/*---------------------------------------------------------------------------*
  Name:         WM_StartParent

  Description:  Starts communication as the parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartParent(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_EndParent

  Description:  Stops communication as the parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndParent(WMCallbackFunc callback);

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
WMErrCode WM_StartScan(WMCallbackFunc callback, const WMScanParam *param);

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
WMErrCode WM_StartScanEx(WMCallbackFunc callback, const WMScanExParam *param);

/*---------------------------------------------------------------------------*
  Name:         WM_EndScan

  Description:  Stops the scan process as a child device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndScan(WMCallbackFunc callback);

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
                                  WM_AUTHMODE_OPEN_SYSTEM: OPEN SYSTEM mode
                                  WM_AUTHMODE_SHARED_KEY: SHARED KEY mode

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartConnectEx(WMCallbackFunc callback, const WMBssDesc *pInfo, const u8 *ssid,
                            BOOL powerSave, u16 authMode);

/*---------------------------------------------------------------------------*
  Name:         WM_StartConnect

  Description:  Starts the connection to the parent as a child device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                pInfo       -   Information of the parent to connect to. Specifies the structure obtained with WM_StartScan.
                                 Note that this structure is forcibly stored in the cache.
                                
                ssid        -   Child information to notify to the parent (24Byte(WM_SIZE_CHILD_SSID) fixed size)

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
static inline WMErrCode
WM_StartConnect(WMCallbackFunc callback, const WMBssDesc *pInfo, const u8 *ssid)
{
    return WM_StartConnectEx(callback, pInfo, ssid, TRUE, WM_AUTHMODE_OPEN_SYSTEM);
}

/*---------------------------------------------------------------------------*
  Name:         WM_Disconnect

  Description:  Cuts off the connection that has been established.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                aid         -   AID of the communication partner to be disconnected.
                                In the parent's case, individually disconnects children having IDs 1 - 15.
                                In the child's case, ends communication with the parent having ID 0.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_Disconnect(WMCallbackFunc callback, u16 aid);

/*---------------------------------------------------------------------------*
  Name:         WM_DisconnectChildren

  Description:  Disconnects the respective children with which the connection has been established. Function exclusively for parent use.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                aidBitmap   -   AID bitfield of the children to be disconnected.
                                The least significant bit will be ignored. Bits 1-15 indicate AIDs 1-15, respectively.
                                Bits that indicate unconnected child devices will be ignored, so specify 0xFFFF to disconnect all children regardless of their connection state.
                                
                                

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_DisconnectChildren(WMCallbackFunc callback, u16 aidBitmap);

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPSendBufferSize

  Description:  Calculates MP communication send buffer size from the current WM state.

  Arguments:    None.

  Returns:      int      - Send buffer size to be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
int     WM_GetMPSendBufferSize(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPReceiveBufferSize

  Description:  Calculates the MP communication receive buffer size from the current WM state.

  Arguments:    None.

  Returns:      int      - Receive buffer size to be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
int     WM_GetMPReceiveBufferSize(void);

/*---------------------------------------------------------------------------*
  Name:         WM_StartMPEx

  Description:  Starts MP communication. It's possible to specify detailed operation mode. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                recvBuf     -   Receive data storage buffer.
                                    Pay attention to the cache, because the ARM7 writes out data directly.
                recvBufSize -   Size of the receive data storage buffer.
                                    It must be larger than the return value of WM_GetReceiveBufferSize().
                sendBuf     -   Buffer that stores the send data.
                                    Pay attention to the cache, because the ARM7 writes out data directly.
                sendBufSize -   Size of the send data storage buffer.
                                    It must be larger than the return value of WM_GetSendBufferSize().
                mpFreq      -   How many times MP communication is performed in one frame.
                                    0 is continuous send mode. This carries a meaning only for the parent.
                defaultRetryCount - The standard number of retries when data fails to be sent on ports 0-7.
                                     Specify 0 to not retry.
                minPollBmpMode - An operating mode that constrains the pollBitmap during MP communications to the minimum set of packet addressees.
                                    
                singlePacketMode - A special operating mode that sends only one packet for each MP transmission.
                                    
                fixFreqMode     -   Special operation mode that prohibits the increase of the MP communication by retries.
                                    The number of times for MP communication in a frame is fixed to exactly mpFreq.

                ignoreFatalError -  No longer perform AutoDisconnect when a FatalError is generated.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartMPEx(WMCallbackFunc callback,
                       u16 *recvBuf,
                       u16 recvBufSize,
                       u16 *sendBuf,
                       u16 sendBufSize,
                       u16 mpFreq,
                       u16 defaultRetryCount,
                       BOOL minPollBmpMode,
                       BOOL singlePacketMode, BOOL fixFreqMode, BOOL ignoreFatalError);

/*---------------------------------------------------------------------------*
  Name:         WM_StartMP

  Description:  Starts MP communication. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                recvBuf     -   Receive data storage buffer.
                                Pay attention to the cache, because the ARM7 writes out data directly.
                recvBufSize -   Size of the receive data storage buffer.
                                It must be larger than the return value of WM_GetReceiveBufferSize().
                sendBuf     -   Buffer that stores the send data.
                                Pay attention to the cache, because the ARM7 writes out data directly.
                sendBufSize -   Size of the send data storage buffer.
                                It must be larger than the return value of WM_GetSendBufferSize().
                mpFreq      -   How many times MP communication is performed in one frame.
                                0 is continuous send mode. This carries a meaning only for the parent.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartMP(WMCallbackFunc callback,
                     u16 *recvBuf, u16 recvBufSize, u16 *sendBuf, u16 sendBufSize, u16 mpFreq);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPParameter

  Description:  Configures the different parameters for MP communications

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                param       -   Pointer to the structure where the parameters for MP communications are stored

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPParameter(WMCallbackFunc callback, const WMMPParam * param);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPChildSize

  Description:  Sets the number of bytes a child can send in one MP communication.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                childSize   -   Number of send bytes for a child

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPChildSize(WMCallbackFunc callback, u16 childSize);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPParentSize

  Description:  Sets the number of bytes a parent can send in one MP communication.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                childSize   -   Number of send bytes for a parent.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPParentSize(WMCallbackFunc callback, u16 parentSize);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPFrequency

  Description:  Switches how many times to perform the MP communication in one frame. Function for parent device.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                mpFreq      -   How many times MP communication is performed in one frame.
                                0 is continuous send mode.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPFrequency(WMCallbackFunc callback, u16 mpFreq);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPInterval

  Description:  Sets the interval in which a connection can be made and MP communications carried out in one frame.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                parentInterval - The interval in which a connection can be made and MP communications carried out by a parent in one frame (ms)
                childInterval  - The interval in which a connection can be made and MP communications carried out by a child in one frame (ms)

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPInterval(WMCallbackFunc callback, u16 parentInterval, u16 childInterval);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPTiming

  Description:  Sets the preparation start timing for MP communications when in frame synchronous communication mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                parentVCount -  The parent operation start V Count when in frame simultaneous communications
                childVCount -  The child operation start V Count when in frame simultaneous communications

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPTiming(WMCallbackFunc callback, u16 parentVCount, u16 childVCount);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPDataToPortEx

  Description:  Reserves data with MP communication. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                arg         -   Argument to be passed to callback
                sendData    -   Pointer to the data to reserve send.
                                Note that an instance of this data is forcibly stored in the cache.
                sendDataSize -  Size of the data to reserve send.
                destBitmap  -   Specifies an aid bitmap that indicates the send destination child.
                port        -   Port number to send
                prio        -   Priority (0: highest -- 3: lowest)

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetMPDataToPortEx(WMCallbackFunc callback,
                               void *arg,
                               const u16 *sendData,
                               u16 sendDataSize, u16 destBitmap, u16 port, u16 prio);

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPDataToPort

  Description:  Reserves data with MP communication. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                sendData    -   Pointer to the data to reserve send.
                                Note that an instance of this data is forcibly stored in the cache.
                sendDataSize -  Size of the data to reserve send.
                destBitmap  -   Specifies an aid bitmap that indicates the send destination child.
                port        -   Port number to send
                prio        -   Priority (0: highest -- 3: lowest)

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
static inline WMErrCode
WM_SetMPDataToPort(WMCallbackFunc callback, const u16 *sendData, u16 sendDataSize, u16 destBitmap,
                   u16 port, u16 prio)
{
    return WM_SetMPDataToPortEx(callback, NULL, sendData, sendDataSize, destBitmap, port, prio);
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetMPData

  Description:  Reserves data with MP communication. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                sendData    -   Pointer to the data to reserve send.
                                Note that an instance of this data is forcibly stored in the cache.
                sendDataSize -  Size of the data to reserve send.
                tmptt       -   In continuous send mode, this specifies MP packet send time.
                                (Only valid for parent. ) (ignored as of 2004/07/26)
                pollbmp     -   In continuous send mode, this specifies a Poll map that indicates the send destination child(ren).

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
static inline WMErrCode
WM_SetMPData(WMCallbackFunc callback, const u16 *sendData, u16 sendDataSize, u16 tmptt, u16 pollbmp)
{
#pragma unused( tmptt )
    return WM_SetMPDataToPortEx(callback, NULL, sendData, sendDataSize, pollbmp, WM_PORT_RAWDATA,
                                WM_PRIORITY_NORMAL);
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndMP

  Description:  Stops MP communication. Function that is common to parent and child.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndMP(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_StartDCF

  Description:  Starts infrastructure-mode communications.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                recvBuf     -   Pointer to the data receive buffer.
                                Pay attention to the cache, because the ARM7 writes out data directly.
                recvBufSize -   Size of data receive buffer.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartDCF(WMCallbackFunc callback, WMDcfRecvBuf *recvBuf, u16 recvBufSize);

/*---------------------------------------------------------------------------*
  Name:         WM_SetDCFData

  Description:  Reserves data for sending by infrastructure-mode communication.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                destAdr     -   Pointer to the buffer that shows the MAC address of the communication partner.
                sendData    -   Pointer to the data to reserve send.
                                NOTE: The instance of the data reserved for sending is forcibly stored in the cache.
                                
                sendDataSize -  Size of the data to reserve send.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetDCFData(WMCallbackFunc callback,
                        const u8 *destAdr, const u16 *sendData, u16 sendDataSize);

/*---------------------------------------------------------------------------*
  Name:         WM_EndDCF

  Description:  Stops infrastructure-mode communications.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndDCF(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKey

  Description:  Sets encryption functionality and the encryption key.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                wepmode     -   0: No encryption feature.
                                1: RC4 (40-bit) encryption mode.
                                2: RC4 (104-bit) encryption mode.
                                3: RC4 (128-bit) encryption mode.
                wepkey      -   Pointer to the encryption key data (80 bytes).
                                Key data consists of 4 pieces of data, each 20 bytes long.
                                Of each 20 byte piece, in 40-bit mode 5 bytes are used, in 104-bit mode 13 bytes are used, and in 128-bit mode 16 bytes are used.
                                 
                                
                                
                                
                                The data instance is forcibly stored in the cache.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKey(WMCallbackFunc callback, u16 wepmode, const u16 *wepkey);

/*---------------------------------------------------------------------------*
  Name:         WM_SetWEPKeyEx

  Description:  Sets encryption functionality, encryption key, and encryption key ID.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                wepmode     -   0: No encryption feature.
                                1: RC4 (40-bit) encryption mode.
                                2: RC4 (104-bit) encryption mode.
                                3: RC4 (128-bit) encryption mode.
                wepkeyid    -   Selects which of 4 specified wepkeys to use.
                                Specify using 0-3.
                wepkey      -   Pointer to the encryption key data (80 bytes).
                                Key data consists of 4 pieces of data, each 20 bytes long.
                                Of each 20 bytes,
                                 5 bytes in 40-bit mode
                                13 bytes in 104-bit mode
                                and 16 bytes in 128-bit mode
                                are used.
                                The data instance is forcibly stored in the cache.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetWEPKeyEx(WMCallbackFunc callback, u16 wepmode, u16 wepkeyid, const u8 *wepkey);

/*---------------------------------------------------------------------------*
  Name:         WM_ReadMPData

  Description:  Extracts only the data of the specified child from all of the received data. Synchronous function.

  Arguments:    header      -   Pointer that indicates the entire body of received data.
                aid         -   AID of the child for which to extract data.

  Returns:      WMMpRecvData* - Returns a pointer to the data received from the corresponding child.
                                Returns NULL if extraction fails.
 *---------------------------------------------------------------------------*/
WMMpRecvData *WM_ReadMPData(const WMMpRecvHeader *header, u16 aid);

/*---------------------------------------------------------------------------*
  Name:         WM_StartDataSharing

  Description:  Enables the data sharing feature. Synchronous function.
                By performing MP communication after enabling the feature, data sharing communications will be carried out by piggybacking on the MP communications.
                

  Arguments:    dsInfo      -   WMDataSharingInfo structure.
                port:         Number of the port to use.
                aidBitmap:     AIDs of the data sharing partners.
                dataLength:     Data length of shared memory. (Specify an even number of bytes.)
                doubleMode:     Specify TRUE if using WM_StepDataSharing in every frame.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartDataSharing(WMDataSharingInfo *dsInfo,
                              u16 port, u16 aidBitmap, u16 dataLength, BOOL doubleMode);

/*---------------------------------------------------------------------------*
  Name:         WM_EndDataSharing

  Description:  Disables the data sharing feature. Synchronous function.

  Arguments:    dsInfo      -   WMDataSharingInfo structure.

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndDataSharing(WMDataSharingInfo *dsInfo);

/*---------------------------------------------------------------------------*
  Name:         WM_StepDataSharing

  Description:  Advances data sharing by one synchronization. Synchronous function.

  Arguments:    dsInfo      -   WMDataSharingInfo structure.
                sendData    -   The send data to be shared.
                receiveData -   The received shared data.

  Returns:      WMErrCode   -   Returns the processing result.
                         *_SUCCESS: Shared data send/receive successful.
                         *_NO_DATASET: Shared data not yet received.
 *---------------------------------------------------------------------------*/
WMErrCode WM_StepDataSharing(WMDataSharingInfo *dsInfo,
                             const u16 *sendData, WMDataSet *receiveData);

/*---------------------------------------------------------------------------*
  Name:         WM_GetSharedDataAddress

  Description:  Acquires the address of a particular AID that is contained in data received through data sharing.
                Synchronous function.

  Arguments:    dsInfo      -   WMDataSharingInfo structure.
                receiveData -   The received shared data.
                aid:           AID.

  Returns:      u16* :     The address of the received data. If it does not exist, returns NULL.
 *---------------------------------------------------------------------------*/
u16    *WM_GetSharedDataAddress(WMDataSharingInfo *dsInfo, WMDataSet *receiveData, u16 aid);

/*---------------------------------------------------------------------------*
  Name:         WM_StartKeySharing

  Description:  Enables the key sharing feature.
                By performing MP communication after enabling the feature, key sharing communications will be carried out by piggybacking on the MP communications.
                

  Arguments:    buf         -   Pointer to the buffer that stores the key information.
                                Its instance is a pointer to a WMDataSharingInfo structure.
                port:        Number of port to use.

  Returns:      WMErrCode   -   Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartKeySharing(WMKeySetBuf *buf, u16 port);

/*---------------------------------------------------------------------------*
  Name:         WM_EndKeySharing

  Description:  Disables the key sharing feature.

  Arguments:    buf         -   Pointer to the buffer that stores the key information.
                                Its instance is a pointer to a WMDataSharingInfo structure.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndKeySharing(WMKeySetBuf *buf);

/*---------------------------------------------------------------------------*
  Name:         WM_GetKeySet

  Description:  Loads one key set data that has been key shared.

  Arguments:    buf         -   Pointer to the buffer that stores the key information.
                                Its instance is a pointer to a WMDataSharingInfo structure.
                keySet      -   Pointer to the buffer that reads out the key set.
                                Specify a buffer other than the one provided by WM_StartKeySharing.
                                

  Returns:      MWErrCode   -   Returns process results.
 *---------------------------------------------------------------------------*/
WMErrCode WM_GetKeySet(WMKeySetBuf *buf, WMKeySet *keySet);

/*---------------------------------------------------------------------------*
  Name:         WM_SetGameInfo

  Description:  Sets the game information. Initial value is set by WM_SetParentParameter. Use this function to change the game information.
                

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                userGameInfo     - Pointer to the user game information.
                userGameInfoSize - Size of the user game information.
                ggid        -   Game group ID
                tgid        -   Temporary group ID
                attr        -   Flag group. Sets the OR of the following flags.
                                    WM_ATTR_FLAG_ENTRY - Entry permitted
                                    WM_ATTR_FLAG_MB    - Accepting multiboot
                                    WM_ATTR_FLAG_KS    - Key sharing
                                    WM_ATTR_FLAG_CS    - Continuous transfer mode
  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetGameInfo(WMCallbackFunc callback,
                         const u16 *userGameInfo, u16 userGameInfoSize,
                         u32 ggid, u16 tgid, u8 attr);

/*---------------------------------------------------------------------------*
  Name:         WM_SetBeaconIndication

  Description:  Switches the beacon send/receive indication between enabled/disabled.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                flag        -   0: Disabled
                                1: Enabled

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetBeaconIndication(WMCallbackFunc callback, u16 flag);

#ifdef  WM_ENABLE_TESTMODE
/*---------------------------------------------------------------------------*
  Name:         WM_StartTestMode

  Description:  Starts communication in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                signal      -   0: No modulation (data=0)
                                1: No modulation (data=1)
                                2: PN15-stage
                                3: 01 pattern (with scramble)
                                4: 01 pattern (without scramble)
                rate        -   1: 1Mbps
                                2: 2Mbps
                channel     -   Specifies the channel to send data (1 to 14).

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestMode(WMCallbackFunc callback, u16 signal, u16 rate, u16 channel);

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestMode

  Description:  Stops communication in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestMode(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_StartTestRxMode

  Description:  Starts receiving data in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                channel     -   Specifies the channel for receiving data (1 to 14).

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartTestRxMode(WMCallbackFunc callback, u16 channel);

/*---------------------------------------------------------------------------*
  Name:         WM_StopTestRxMode

  Description:  Stops receiving data in test mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StopTestRxMode(WMCallbackFunc callback);

#endif

/*---------------------------------------------------------------------------*
  Name:         WM_SetLifeTime

  Description:  Sets the lifetime.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                tableNumber -   Number of CAM table that sets the lifetime.
                                (With 0xFFFF, all tables)
                camLifeTime:     CAM lifetime. (in 100-ms units: disable with 0xFFFF)
                frameLifeTime:   Beacon interval of the lifetime of the frame to be set.
                                (in 100-ms units: disable with 0xFFFF)
                camLifeTime:    Lifetime of the MP communication. (in 100-ms units: disable with 0xFFFF)

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetLifeTime(WMCallbackFunc callback,
                         u16 tableNumber, u16 camLifeTime, u16 frameLifeTime, u16 mpLifeTime);

/*---------------------------------------------------------------------------*
  Name:         WM_MeasureChannel

  Description:  Measures the usage status of a channel.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                ccaMode     -   CCA operation mode.
                                0: Carrier sense only. ED threshold value is ignored.
                                1: Valid with ED threshold value only.
                                2: AND of carrier sense and ED threshold value.
                                3: OR of carrier sense and ED threshold value.
                EDThreshold -   ED threshold (0 thru 61): -60dBm thru -80dBm
                channel     -   Channel to investigate.
                                (Only one channel with one call of MeasureChannel)
                measureTime -   Time to investigate.

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_MeasureChannel(WMCallbackFunc callback,
                            u16 ccaMode, u16 edThreshold, u16 channel, u16 measureTime);

/*---------------------------------------------------------------------------*
  Name:         WM_InitWirelessCounter

  Description:  Initializes WirelessCounter.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_InitWirelessCounter(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCounter

  Description:  Obtains the Wireless NIC send/receive frame count and send/receive error frame count.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_GetWirelessCounter(WMCallbackFunc callback);

/*---------------------------------------------------------------------------*
  Name:         WM_GetAllowedChannel

  Description:  Obtains the channel that was permitted to use for the communication. Synchronous function.

  Arguments:    None.

  Returns:      u16 -   Returns the bit field of the permitted channel.
                        The least significant bit indicates channel 1, and the most significant bit indicates channel 16.
                        If a channel's corresponding bit is set to 1, it is usable; otherwise, its use is prohibited.
                        Typically, a value is returned with several of the bits corresponding to channels 1-13 set to 1.
                        If 0x0000 is returned, no channels can be used and wireless features are therefore prohibited.
                        
                        Also, in case the function failed, such as when it is not yet initialized, 0x8000 is returned.
 *---------------------------------------------------------------------------*/
u16     WM_GetAllowedChannel(void);

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
/*---------------------------------------------------------------------------*
  Name:         WM_IsExistAllowedChannel

  Description:  Checks if the channel that was permitted to be used for communication actually exists.
                Possible to determine it properly even before the initialization of WM library.

  Arguments:    None.

  Returns:      BOOL: Returns TRUE if you are permitted to use some channels, and FALSE otherwise.
                            
 *---------------------------------------------------------------------------*/
BOOL    WM_IsExistAllowedChannel(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         WM_SetEntry

  Description:  Switches between accepting and refusing a connection from a child as a parent.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                enabled:        Entry permitted / not permitted flag. TRUE: permitted, FALSE: not permitted.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetEntry(WMCallbackFunc callback, BOOL enabled);

/*---------------------------------------------------------------------------*
  Name:         WM_GetLinkLevel

  Description:  Gets link level during communication. Synchronous function.

  Arguments:    None.

  Returns:      WMLinkLevel -   Returns the link level rated in four levels.
 *---------------------------------------------------------------------------*/
WMLinkLevel WM_GetLinkLevel(void);


/*---------------------------------------------------------------------------*
  Name:         WMi_SetBeaconPeriod

  Description:  Changes the beacon period.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                beaconPeriod -  Beacon period (10 to 1000 TU (1024 microseconds))

  Returns:      WMErrCode   -   Returns the processing result.
                                Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SetBeaconPeriod(WMCallbackFunc callback, u16 beaconPeriod);
/* As of 2004/10/18, this function was changed to undisclosed due to the problem found when the beacon period was changed while communicating.
    */


/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionBeaconPeriod

  Description:  Obtains the beacon period value that should be set when operating as a parent.
                Synchronous function.

  Arguments:    None.

  Returns:      u16 -   Beacon period value that should be set (ms).
 *---------------------------------------------------------------------------*/
u16     WM_GetDispersionBeaconPeriod(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionScanPeriod

  Description:  Obtains the search limit time that should be set for a child to search for a parent.
                Synchronous function.

  Arguments:    None.

  Returns:      u16 -   Search limit time that should be set (ms).
 *---------------------------------------------------------------------------*/
u16     WM_GetDispersionScanPeriod(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetOtherElements

  Description:  Gets the extended elements in the beacon.
                Synchronous function.

  Arguments:    bssDesc - Parent information structure.
                          Specifies the structure obtained with WM_StartScan(Ex).

  Returns:      WMOtherElements - Extended element structure.
 *---------------------------------------------------------------------------*/
WMOtherElements WM_GetOtherElements(WMBssDesc *bssDesc);

/*---------------------------------------------------------------------------*
  Name:         WM_GetNextTgid

  Description:  Gets the automatically generated, unique TGID value.
                Synchronous function.
                This function internally calls RTC_Init().

  Arguments:    None.

  Returns:      The first time it is called, it returns a TGID that was generated based on the RTC; thereafter, it returns the value returned the previous time incremented by 1.
                
 *---------------------------------------------------------------------------*/
u16     WM_GetNextTgid(void);

/*---------------------------------------------------------------------------*
  Name:         WM_SetPowerSaveMode()

  Description:  Changes the power save mode.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                powerSave   -   When using power save mode, TRUE. When not using it, FALSE.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPowerSaveMode(WMCallbackFunc callback, BOOL powerSave);

/*---------------------------------------------------------------------------*
  Name:         WMi_IsMP

  Description:  Gets current MP communication state.

  Arguments:    None.

  Returns:      TRUE if in MP communication state.
 *---------------------------------------------------------------------------*/
BOOL    WMi_IsMP(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetAID

  Description:  Gets current AID.
                Returns a valid value only when the state is one of PARENT, MP_PARENT, CHILD, or MP_CHILD.
                

  Arguments:    None.

  Returns:      AID
 *---------------------------------------------------------------------------*/
u16     WM_GetAID(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetConnectedAIDs

  Description:  Gets the currently connected partners in bitmap format.
                Returns a valid value only when the state is one of PARENT, MP_PARENT, CHILD, or MP_CHILD.
                
                For a child device, returns 0x0001 when the child is connected to a parent.

  Arguments:    None.

  Returns:      Bitmap of AIDs of connected partners.
 *---------------------------------------------------------------------------*/
u16     WM_GetConnectedAIDs(void);

/*---------------------------------------------------------------------------*
  Name:         WMi_GetMPReadyAIDs

  Description:  From among the currently connected parties, gets a list in bitmap format of the AIDs of parties which can receive MP.
                
                Returns a valid value only when the state is one of PARENT, MP_PARENT, CHILD, or MP_CHILD.
                
                For a child device, returns 0x0001 when the child is connected to a parent.

  Arguments:    None.

  Returns:      Bitmap of the AIDs of partners with which MP is starting.
 *---------------------------------------------------------------------------*/
u16     WMi_GetMPReadyAIDs(void);

/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCommFlag

  Description:  Checks whether wireless communications are currently in a usable state.

  Arguments:    None.

  Returns:      WM_WIRELESS_COMM_FLAG_OFF: Wireless communications are not permitted
                WM_WIRELESS_COMM_FLAG_ON: Wireless communications are permitted
                WM_WIRELESS_COMM_FLAG_UNKNOWN: Indeterminate because the program is running on a DS 
 *---------------------------------------------------------------------------*/
u8      WM_GetWirelessCommFlag(void);

// for debugging
void    WMi_DebugPrintSendQueue(WMPortSendQueue *queue);
void    WMi_DebugPrintAllSendQueue(void);
const WMStatus *WMi_GetStatusAddress(void);
BOOL    WMi_CheckMpPacketTimeRequired(u16 parentSize, u16 childSize, u8 childs);


/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         WM_SIZE_MP_PARENT_RECEIVE_BUFFER

  Description:  Calculates parent receive buffer size.

  Arguments:    childMaxSize:   Maximum number of bytes of data you want to receive from a child.
                maxEntry:      Number of children you want to have connected.
                ksFlag:        Boolean value for whether to perform key sharing.

  Returns:      int:       Size of the receive buffer that must be passed to the WM_StartMP.
 *---------------------------------------------------------------------------*/
#define WM_SIZE_MP_PARENT_RECEIVE_BUFFER( childMaxSize, maxEntry, ksFlag ) \
            (((sizeof( WMMpRecvHeader ) - sizeof( WMMpRecvData ) + \
            ( ( sizeof( WMMpRecvData ) + (childMaxSize) + WM_HEADER_CHILD_MAX_SIZE - 2 + 2/*MACBUG*/ + ((ksFlag) ? WM_SIZE_KS_CHILD_DATA + WM_SIZE_MP_CHILD_PADDING : 0) ) * (maxEntry) ) \
             + 31) & ~0x1f) * 2)

/*---------------------------------------------------------------------------*
  Name:         WM_SIZE_MP_CHILD_RECEIVE_BUFFER

  Description:  Calculates the size of the child receive buffer.

  Arguments:    parentMaxSize:   Maximum number of bytes of data you want to receive from the parent.
                ksFlag:        Boolean value for whether to perform key sharing.

  Returns:      int:       Size of the receive buffer that must be passed to the WM_StartMP.
 *---------------------------------------------------------------------------*/
#define WM_SIZE_MP_CHILD_RECEIVE_BUFFER( parentMaxSize, ksFlag ) \
            (((sizeof( WMMpRecvBuf ) + (parentMaxSize) + WM_HEADER_PARENT_MAX_SIZE - 4 + ((ksFlag) ? WM_SIZE_KS_PARENT_DATA + WM_SIZE_MP_PARENT_PADDING : 0) + 31) & ~0x1f) * 2)

/*---------------------------------------------------------------------------*
  Name:         WM_SIZE_MP_PARENT_SEND_BUFFER

  Description:  Calculates the size of the parent send buffer.

  Arguments:    parentMaxSize:   Maximum number of bytes of data you want to send.
                ksFlag:        Boolean value for whether to perform key sharing.

  Returns:      int:      Size of send buffer that must be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
#define WM_SIZE_MP_PARENT_SEND_BUFFER( parentMaxSize, ksFlag ) \
            (((parentMaxSize) + WM_HEADER_PARENT_MAX_SIZE + ((ksFlag) ? WM_SIZE_KS_PARENT_DATA + WM_SIZE_MP_PARENT_PADDING : 0) + 31) & ~0x1f)

/*---------------------------------------------------------------------------*
  Name:         WM_SIZE_MP_CHILD_SEND_BUFFER

  Description:  Calculates the size of the child send buffer.

  Arguments:    childMaxSize:   Maximum number of bytes of data you want to send.
                ksFlag:        Boolean value for whether to perform key sharing.

  Returns:      int:      Size of send buffer that must be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
#define WM_SIZE_MP_CHILD_SEND_BUFFER( childMaxSize, ksFlag ) \
            (((childMaxSize) + WM_HEADER_CHILD_MAX_SIZE + ((ksFlag) ? WM_SIZE_KS_CHILD_DATA + WM_SIZE_MP_CHILD_PADDING : 0) + 31) & ~0x1f)

/*---------------------------------------------------------------------------*
  Name:         WM_SIZE_DCF_CHILD_SEND_BUFFER

  Description:  Calculates the size of the DCF child's send buffer.

  Arguments:    childMaxSize:   Maximum number of bytes of data you want to send.

  Returns:      int:      Size of send buffer that must be passed to WM_StartDCF.
 *---------------------------------------------------------------------------*/
#define WM_SIZE_DCF_CHILD_SEND_BUFFER( childMaxSize ) \
            (((childMaxSize) + WM_SIZE_MADATA_HEADER + 31) & ~0x1f)

#define WM_SIZE_CHILD_SEND_BUFFER WM_SIZE_MP_CHILD_SEND_BUFFER
#define WM_SIZE_CHILD_RECEIVE_BUFFER WM_SIZE_MP_CHILD_RECEIVE_BUFFER
#define WM_SIZE_PARENT_SEND_BUFFER WM_SIZE_MP_PARENT_SEND_BUFFER
#define WM_SIZE_PARENT_RECEIVE_BUFFER WM_SIZE_MP_PARENT_RECEIVE_BUFFER


/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         WM_ConvGgid32to16

  Description:  Converts game group ID from "32-bit type" to "16-bit x 2 type."

  Arguments:    src:        Pointer to the conversion source GGID expressed in 32 bits.
                dest:      Pointer to the buffer that will store the converted GGID.
                            A 4-byte piece of data will be written beginning from this address.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WM_ConvGgid32to16(u32 *src, u16 *dst)
{
    dst[0] = (u16)(*src & 0x0000ffff);
    dst[1] = (u16)(*src >> 16);
}

/*---------------------------------------------------------------------------*
  Name:         WM_ConvGgid16to32

  Description:  Converts game group ID from 16-bit x 2 type to 32-bit type.

  Arguments:    src:       Pointer to the conversion source GGID that is expressed by 16-bit arrays.
                dest:      Pointer to the buffer that will store the converted GGID.
                            A 4-byte piece of data will be written beginning from this address.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WM_ConvGgid16to32(u16 *src, u32 *dst)
{
    *dst = ((u32)src[1] << 16) | src[0];
}

/*---------------------------------------------------------------------------*
  Name:         WM_IsBssidEqual

  Description:  Checks whether 2 BSSIDs are equivalent.

  Arguments:    idp1:       Pointer to a BSSID that will be compared.
                idp2:       Pointer to a BSSID that will be compared.

  Returns:      Returns TRUE if they are equivalent.
                Returns FALSE if they differ.
 *---------------------------------------------------------------------------*/
static inline BOOL WM_IsBssidEqual(const u8 *idp1, const u8 *idp2)
{
    return ((*idp1 == *idp2) &&
            (*(idp1 + 1) == *(idp2 + 1)) &&
            (*(idp1 + 2) == *(idp2 + 2)) &&
            (*(idp1 + 3) == *(idp2 + 3)) &&
            (*(idp1 + 4) == *(idp2 + 4)) && (*(idp1 + 5) == *(idp2 + 5)));
}


/*---------------------------------------------------------------------------*
  Name:         WM_IsBssidEqual16

  Description:  Checks whether 2 BSSIDs are equivalent, in 2-byte chunks.

  Arguments:    idp1:       Pointer to a BSSID that will be compared. Must be two-byte aligned.
                idp2:       Pointer to a BSSID that will be compared. Must be two-byte aligned.

  Returns:      Returns TRUE if they are equivalent.
                Returns FALSE if they differ.
 *---------------------------------------------------------------------------*/
static inline BOOL WM_IsBssidEqual16(const u8 *idp1, const u8 *idp2)
{
    SDK_ALIGN2_ASSERT(idp1);
    SDK_ALIGN2_ASSERT(idp2);

    return ((*(u16 *)idp1 == *(u16 *)idp2) &&
            (*(u16 *)(idp1 + 2) == *(u16 *)(idp2 + 2)) &&
            (*(u16 *)(idp1 + 4) == *(u16 *)(idp2 + 4)));
}

/*---------------------------------------------------------------------------*
  Name:         WM_CopyBssid

  Description:  Copies a BSSID.

  Arguments:    src:        Pointer to the BSSID copy source.
                dest:       Pointer to the BSSID copy destination.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WM_CopyBssid(const u8 *src, u8 *dst)
{
    *dst = *src;
    *(dst + 1) = *(src + 1);
    *(dst + 2) = *(src + 2);
    *(dst + 3) = *(src + 3);
    *(dst + 4) = *(src + 4);
    *(dst + 5) = *(src + 5);
}

/*---------------------------------------------------------------------------*
  Name:         WM_CopyBssid16

  Description:  Copies a BSSID in 2-byte chunks.

  Arguments:    src:        Pointer to the BSSID copy source. Must be two-byte aligned.
                dest:       Pointer to the BSSID copy destination. Must be two-byte aligned.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WM_CopyBssid16(const u8 *src, u8 *dst)
{
    SDK_ALIGN2_ASSERT(src);
    SDK_ALIGN2_ASSERT(dst);

    *(u16 *)dst = *(u16 *)src;
    *(u16 *)(dst + 2) = *(u16 *)(src + 2);
    *(u16 *)(dst + 4) = *(u16 *)(src + 4);
}

/*---------------------------------------------------------------------------*
  Name:         WM_IsValidGameInfo

  Description:  Checks whether a GameInfo is supported.

  Arguments:    gameInfo:   Pointer to WMGameInfo structure.
                gameInfoLength:   Length of data at location specified by gameInfo.

  Returns:      Returns TRUE if the GameInfo is supported.
 *---------------------------------------------------------------------------*/
static inline BOOL WM_IsValidGameInfo(const WMGameInfo *gameInfo, u16 gameInfoLength)
{
    return (  gameInfoLength >= WM_GAMEINFO_LENGTH_MIN
            && gameInfoLength <= WM_SIZE_GAMEINFO
            && gameInfoLength == WM_SIZE_SYSTEM_GAMEINFO + gameInfo->userGameInfoLength
            && gameInfo->magicNumber == WM_GAMEINFO_MAGIC_NUMBER );
}

/*---------------------------------------------------------------------------*
  Name:         WM_IsValidGameBeacon

  Description:  Checks whether a beacon is from a parent device for DS wireless play.

  Arguments:    bssDesc:   Pointer to WMBssDesc structure that contains the beacon information.

  Returns:      If the beacon is from a DS wireless play parent, TRUE.
 *---------------------------------------------------------------------------*/
static inline BOOL WM_IsValidGameBeacon(const WMBssDesc *bssDesc)
{
    return (  WM_IsValidGameInfo(&bssDesc->gameInfo, bssDesc->gameInfoLength)
            && bssDesc->length == ( (sizeof(WMBssDesc) - WM_SIZE_GAMEINFO) + (bssDesc->gameInfoLength+1) )/2);
}

/*---------------------------------------------------------------------------*
  Name:         WMi_IsDisconnectReasonFromMyself

  Description:  Determines whether the reason for WM_STATECODE_DISCONNECTED is that the unit initiated the disconnection itself.
                

  Arguments:    reason:   The reason member of WMStartParentCallback or WMPortRecvCallback.

  Returns:      True if this device caused disconnection itself by calling a WM function; FALSE if it was disconnected by the other party or was disconnected due to a communication error.
                
 *---------------------------------------------------------------------------*/
static inline BOOL WMi_IsDisconnectReasonFromMyself(u16 reason)
{
    return (reason >= WM_DISCONNECT_REASON_FROM_MYSELF);
}


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_WM_ARM9_WM_API_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
