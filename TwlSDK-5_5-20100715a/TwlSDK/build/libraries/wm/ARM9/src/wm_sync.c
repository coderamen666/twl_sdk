/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_sync.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include    <nitro/wm.h>
#include    "wm_arm9_private.h"


/*---------------------------------------------------------------------------*
  Name:         WM_SetIndCallback

  Description:  Sets a function that is called to the status notification from WM7.

  Arguments:    callback: Callback function called during status notification from the wireless firmware
                                

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetIndCallback(WMCallbackFunc callback)
{
    WMErrCode result;
    OSIntrMode e;

    // Prohibit interrupts
    e = OS_DisableInterrupts();

    // Confirm that initialized
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        // End prohibiting of interrupts
        (void)OS_RestoreInterrupts(e);
        return result;
    }

    // Set the callback for Indication
    WMi_GetSystemWork()->indCallback = callback;
    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetPortCallback

  Description:  Sets a function that is called for the communication frame reception notification from WM7.

  Arguments:    port: Port number
                callback: Callback function invoked when there is a receive notification
                arg: Argument passed to the callback function as WMPortRecvCallback.arg
                                

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_SetPortCallback(u16 port, WMCallbackFunc callback, void *arg)
{
    WMErrCode result;
    OSIntrMode e;
    WMPortRecvCallback cb_Port;

    // Check parameters
#ifdef SDK_DEBUG
    if (port >= WM_NUM_OF_PORT)
    {
        WM_WARNING("Parameter \"port\" must be less than 16.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
#endif

    if (callback != NULL)
    {
        MI_CpuClear8(&cb_Port, sizeof(WMPortRecvCallback));
        cb_Port.apiid = WM_APIID_PORT_RECV;
        cb_Port.errcode = WM_ERRCODE_SUCCESS;
        cb_Port.state = WM_STATECODE_PORT_INIT;
        cb_Port.port = port;
        cb_Port.recvBuf = NULL;
        cb_Port.data = NULL;
        cb_Port.length = 0;
        cb_Port.seqNo = 0xffff;
        cb_Port.arg = arg;
        cb_Port.aid = 0;
        OS_GetMacAddress(cb_Port.macAddress);
    }

    // Prohibit interrupts
    e = OS_DisableInterrupts();
    // Confirm that initialized
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        // End prohibiting of interrupts
        (void)OS_RestoreInterrupts(e);
        return result;
    }

    // Set the callback for data reception
    {
        WMArm9Buf *p = WMi_GetSystemWork();

        p->portCallbackTable[port] = callback;
        p->portCallbackArgument[port] = arg;
    }

    if (callback != NULL)
    {
        cb_Port.connectedAidBitmap = WM_GetConnectedAIDs();
        cb_Port.myAid = WM_GetAID();
        (*callback) ((void *)&cb_Port);
    }

    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_ReadStatus

  Description:  Gets the structure that indicates the status of the wireless library.

  Arguments:    statusBuf: Pointer to the variable that gets the status

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_ReadStatus(WMStatus *statusBuf)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Confirm that initialized
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // Check parameters
    if (statusBuf == NULL)
    {
        WM_WARNING("Parameter \"statusBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }

    // Copy the WMStatus structure to the CPU
    DC_InvalidateRange(p->status, sizeof(WMStatus));
    MI_CpuCopyFast(p->status, statusBuf, sizeof(WMStatus));

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPSendBufferSize

  Description:  Calculates the size of the send buffer for MP communications.
                At this time, StartParent and StartConnect must be finished.

  Arguments:    None.

  Returns:      int: Send buffer size to be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
int WM_GetMPSendBufferSize(void)
{
    WMErrCode result;
    int     maxSendSize;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_CHILD);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }

    // Confirm MP status
    DC_InvalidateRange(&(p->status->mp_flag), 4);       // Invalidates the ARM7 status region cache
    if (p->status->mp_flag == TRUE)
    {
        WM_WARNING("Already started MP protocol. So can't execute request.\n");
        return 0;
    }

    // Reference the MWStatus structure for information needed for the calculation
    DC_InvalidateRange(&(p->status->mp_maxSendSize), 4);
    maxSendSize = p->status->mp_maxSendSize;

    return ((maxSendSize + 31) & ~0x1f);
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetMPReceiveBufferSize

  Description:  Calculates the size of the Receive buffer for MP communications.
                At this time, StartParent and StartConnect must be finished.

  Arguments:    None.

  Returns:      int: Receive buffer size to be passed to WM_StartMP.
 *---------------------------------------------------------------------------*/
int WM_GetMPReceiveBufferSize(void)
{
    WMErrCode result;
    BOOL    isParent;
    int     maxReceiveSize;
    int     maxEntry;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckStateEx(2, WM_STATE_PARENT, WM_STATE_CHILD);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }

    // Confirm MP status
    DC_InvalidateRange(&(p->status->mp_flag), 4);
    if (p->status->mp_flag == TRUE)
    {
        WM_WARNING("Already started MP protocol. So can't execute request.\n");
        return 0;
    }

    // Reference the MWStatus structure for information needed for the calculation
    DC_InvalidateRange(&(p->status->aid), 2);
    isParent = (p->status->aid == 0) ? TRUE : FALSE;
    DC_InvalidateRange(&(p->status->mp_maxRecvSize), 2);
    maxReceiveSize = p->status->mp_maxRecvSize;
    if (isParent == TRUE)
    {
        DC_InvalidateRange(&(p->status->pparam.maxEntry), 2);
        maxEntry = p->status->pparam.maxEntry;
        return (int)((sizeof(WMmpRecvHeader) - sizeof(WMmpRecvData) +
                      ((sizeof(WMmpRecvData) + maxReceiveSize - 2 + 2 /*MACBUG*/) * maxEntry)
                      + 31) & ~0x1f) * 2;
    }
    else
    {
        return (int)((sizeof(WMMpRecvBuf) + maxReceiveSize - 4 + 31) & ~0x1f) * 2;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WM_ReadMPData

  Description:  Extracts only the data of the specified child from the entire received data.

  Arguments:    header: Pointer that indicates the entire received data
                aid: AID of the child to extract data

  Returns:      WMMpRecvData*: Returns a pointer to the data received from the corresponding child.
                                Returns NULL if extraction fails.
 *---------------------------------------------------------------------------*/
WMMpRecvData *WM_ReadMPData(const WMMpRecvHeader *header, u16 aid)
{
    int     i;
    WMErrCode result;
    WMMpRecvData *recvdata_p[WM_NUM_MAX_CHILD]; // Array of pointers to data starting position for all child devices (up to 15 devices)
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check if initialized
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return NULL;
    }

    // Check parameters
    if ((aid < 1) || (aid > WM_NUM_MAX_CHILD))
    {
        WM_WARNING("Parameter \"aid\" must be between 1 and %d.\n", WM_NUM_MAX_CHILD);
        return NULL;
    }

    // Does child with that AID exist?
    DC_InvalidateRange(&(p->status->child_bitmap), 2);
    if (0 == (p->status->child_bitmap & (0x0001 << aid)))
    {
        return NULL;
    }

    // Is there any received data? || Is the 'count' field value erroneous?
    if (header->count == 0 || header->count > WM_NUM_MAX_CHILD)
    {
        return NULL;
    }

    // Calculate starting position for child data
    recvdata_p[0] = (WMMpRecvData *)(header->data);

    i = 0;
    do
    {
        // If it is the specified AID's child data, it returns its pointer
        if (recvdata_p[i]->aid == aid)
        {
            return recvdata_p[i];
        }
        ++i;
        recvdata_p[i] = (WMMpRecvData *)((u32)(recvdata_p[i - 1]) + header->length);
    }
    while (i < header->count);

    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetAllowedChannel

  Description:  Gets the channel that was permitted to use for the communication.

  Arguments:    None.

  Returns:      u16: Returns the bit field of the permitted channel. The least significant bit indicates channel 1, and the most significant bit indicates channel 16.
                        A channel can be used if its corresponding bit is set to 1 and is prohibited from being used if its corresponding bit is set to 0.
                        Typically, a value is returned with several of the bits corresponding to channels 1-13 set to 1.
                        When 0x0000 is returned, no channels can be used and wireless features are themselves prohibited.
                        Also, in case the function failed, such as when it is not yet initialized, 0x8000 is returned.
                        
                        
 *---------------------------------------------------------------------------*/
u16 WM_GetAllowedChannel(void)
{
#ifdef WM_PRECALC_ALLOWEDCHANNEL
    WMErrCode result;

    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return 0;
        }
    }
    #endif
    
    // Check if initialized
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_GET_ALLOWED_CHANNEL_BEFORE_INIT;
    }
    return *((u16 *)((u32)(OS_GetSystemWork()->nvramUserInfo) +
                     ((sizeof(NVRAMConfig) + 3) & ~0x00000003) + 6));
#else
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();
    
    #ifdef SDK_TWL // Return an error if the TWL System Settings are configured not to use wireless
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() == FALSE )
        {
            return 0;
        }
    }
    #endif
    
    // Confirm that wireless hardware has started
    result = WMi_CheckIdle();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_GET_ALLOWED_CHANNEL_BEFORE_INIT;
    }

    DC_InvalidateRange(&(p->status->allowedChannel), 2);
    return p->status->allowedChannel;
#endif
}

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
/*---------------------------------------------------------------------------*
  Name:         WM_IsExistAllowedChannel

  Description:  Checks whether the channel that was permitted to be used for communication actually exists.
                Possible to determine it properly even before the initialization of WM library.

  Arguments:    None.

  Returns:      BOOL: Returns TRUE if there is a usable channel and FALSE otherwise.
                            
 *---------------------------------------------------------------------------*/
BOOL WM_IsExistAllowedChannel(void)
{
    u16     allowedChannel;

    allowedChannel = *((u16 *)((u32)(OS_GetSystemWork()->nvramUserInfo) +
                     ((sizeof(NVRAMConfig) + 3) & ~0x00000003) + 6));
    if (allowedChannel)
    {
        return TRUE;
    }
    return FALSE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         WM_GetLinkLevel

  Description:  Gets link level during communication. Synchronous function.

  Arguments:    None.

  Returns:      WMLinkLevel: Link level rated in four levels.
 *---------------------------------------------------------------------------*/
WMLinkLevel WM_GetLinkLevel(void)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check if initialized
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        return WM_LINK_LEVEL_0;        // Lowest radio strength
    }

    DC_InvalidateRange(&(p->status->state), 2);
    switch (p->status->state)
    {
    case WM_STATE_MP_PARENT:
        // For parent
        DC_InvalidateRange(&(p->status->child_bitmap), 2);
        if (p->status->child_bitmap == 0)
        {
            // When there is no child
            return WM_LINK_LEVEL_0;    // Lowest radio strength
        }
    case WM_STATE_MP_CHILD:
    case WM_STATE_DCF_CHILD:
        // For child
        DC_InvalidateRange(&(p->status->linkLevel), 2);
        return (WMLinkLevel)(p->status->linkLevel);
    }

    // When unconnected
    return WM_LINK_LEVEL_0;            // Lowest radio strength
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionBeaconPeriod

  Description:  Gets the beacon interval value that should be set when operating as a parent.

  Arguments:    None.

  Returns:      u16: Beacon interval value that should be set (ms).
 *---------------------------------------------------------------------------*/
u16 WM_GetDispersionBeaconPeriod(void)
{
    u8      mac[6];
    u16     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 7;
    return (u16)(WM_DEFAULT_BEACON_PERIOD + (ret % 20));
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetDispersionScanPeriod

  Description:  Gets the search limit time that should be set for a child to search for a parent.

  Arguments:    None.

  Returns:      u16: Search limit time that should be set (ms).
 *---------------------------------------------------------------------------*/
u16 WM_GetDispersionScanPeriod(void)
{
    u8      mac[6];
    u16     ret;
    s32     i;

    OS_GetMacAddress(mac);
    for (i = 0, ret = 0; i < 6; i++)
    {
        ret += mac[i];
    }
    ret += OS_GetVBlankCount();
    ret *= 13;
    return (u16)(WM_DEFAULT_SCAN_PERIOD + (ret % 10));
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetOtherElements

  Description:  Gets the extended elements in the beacon.
                Synchronous function.

  Arguments:    bssDesc: Parent information structure.
                          Specifies the structure obtained with WM_StartScan(Ex).

  Returns:      WMOtherElements: Extended element structure.
 *---------------------------------------------------------------------------*/
WMOtherElements WM_GetOtherElements(WMBssDesc *bssDesc)
{
    WMOtherElements elems;
    u8     *p_elem;
    int     i;
    u16      curr_elem_len;
    u16      elems_len;                 // Length of all elements
    u16      elems_len_counter;         // Counter for checking the length of elements

    // Ends when gameInfo is included
    if (bssDesc->gameInfoLength != 0)
    {
        elems.count = 0;
        return elems;
    }

    // Get otherElement count and terminate if 0
    elems.count = (u8)(bssDesc->otherElementCount);
    if (elems.count == 0)
        return elems;

    // The maximum allowed number of elems is limited to WM_SCAN_OTHER_ELEMENT_MAX
    if (elems.count > WM_SCAN_OTHER_ELEMENT_MAX)
        elems.count = WM_SCAN_OTHER_ELEMENT_MAX;

    // First set the start of elements into gameInfo
    p_elem = (u8 *)&(bssDesc->gameInfo);

    // Get length of all elements and initialize counter used for checking
    elems_len = (u16)((bssDesc->length * sizeof(u16)) - 64);
    elems_len_counter = 0;

    // Loop only 'elems' times
    for (i = 0; i < elems.count; ++i)
    {
        elems.element[i].id = p_elem[0];
        elems.element[i].length = p_elem[1];
        elems.element[i].body = (u8 *)&(p_elem[2]);

        // Calculate current element length and add to the check counter
        curr_elem_len = (u16)(elems.element[i].length + 2);
        elems_len_counter += curr_elem_len;

        // OS_TPrintf("eles_len        =%d\n", elems_len);
        // OS_TPrintf("eles_len_counter=%d\n", elems_len_counter);

        // An error results if the length of all elements is exceeded, and notify that there were no element
        // 
        if (elems_len_counter > elems_len)
        {
            WM_WARNING("Elements length error.\n");
            elems.count = 0;
            return elems;
        }

        // Calculate the lead address of the next element
        p_elem = (u8 *)(p_elem + curr_elem_len);
    }

    return elems;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetNextTgid

  Description:  Gets the automatically generated, unique TGID value.
                Synchronous function.

  Arguments:    None.

  Returns:      The first time it is called, it returns a TGID that was generated based on the RTC; thereafter, it returns the value returned the previous time incremented by 1.
                
 *---------------------------------------------------------------------------*/
u16 WM_GetNextTgid(void)
{
    enum
    { TGID_DEFAULT = (1 << 16) };
    static u32 tgid_bak = (u32)TGID_DEFAULT;
    /* Use the RTC time value the first time so as to preserve the integrity of the unit's own time */
    if (tgid_bak == (u32)TGID_DEFAULT)
    {
        RTCTime rt[1];
        RTC_Init();
        if (RTC_GetTime(rt) == RTC_RESULT_SUCCESS)
        {
            tgid_bak = (u16)(rt->second + (rt->minute << 8));
        }
        else
        {
            OS_TWarning("failed to get RTC-data to create unique TGID!\n");
        }
    }
    /* Use the unique value and increment each time */
    tgid_bak = (u16)(tgid_bak + 1);
    return (u16)tgid_bak;
}


/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
