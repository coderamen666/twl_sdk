/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WXC - libraries -
  File:     wxc_api.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc.h>


/*****************************************************************************/
/* Declaration */

/* All WXC library work variables */
typedef struct WXCWork
{
    u32     wm_dma;
    WXCCallback system_callback;
    WXCScheduler scheduler;
    WXCUserInfo user_info[WM_NUM_MAX_CHILD + 1];
    BOOL    stopping;
    u8      beacon_count;              /* Number of beacon sends */
    u8      padding[3];
    WMParentParam parent_param ATTRIBUTE_ALIGN(32);
    WXCDriverWork driver ATTRIBUTE_ALIGN(32);
    WXCProtocolContext protocol[1] ATTRIBUTE_ALIGN(32);
}
WXCWork;

SDK_STATIC_ASSERT(sizeof(WXCWork) <= WXC_WORK_SIZE);


/*****************************************************************************/
/* Variables */

static WXCStateCode state = WXC_STATE_END;
static WXCWork *work = NULL;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXCi_ChangeState

  Description:  Generates a callback at the same time it transitions the internal state.

  Arguments:    stat: State to transition to
                arg: Callback argument (changes depending on the state)
  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXCi_ChangeState(WXCStateCode stat, void *arg)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        state = stat;
        if (work->system_callback)
        {
            (*work->system_callback) (state, arg);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SeekNextBlock

  Description:  Searches for the next block that should be run.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_SeekNextBlock(void)
{
    /* Select the next block */
    WXCProtocolRegistry *target =
        WXC_FindNextBlock(work->protocol, work->protocol->current_block, 0, FALSE);
    if (!target)
    {
        /* If there is no data registered, we have no choice but to select an empty block */
        target = WXC_FindNextBlock(work->protocol, NULL, 0, TRUE);
    }
    // Just in case, consider cases when protocols are not installed at all
    if (target != NULL)
    {
        WXC_SetCurrentBlock(work->protocol, target);
        work->parent_param.ggid = target->ggid;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_DriverEventCallback

  Description:  Event callback from the wireless driver.

  Arguments:    event: Notified event
                arg: Argument defined corresponding to event

  Returns:      u32-type event result value, defined corresponding to each event.
 *---------------------------------------------------------------------------*/
static u32 WXCi_DriverEventCallback(WXCDriverEvent event, void *arg)
{
    u32     result = 0;

    switch (event)
    {
    case WXC_DRIVER_EVENT_BEACON_SEND:
        /*
         * Beacon update timing (argument is WMParentParam pointer)
         * If currently running, update the beacon state.
         */
        if (!work->stopping && (WXC_GetStateCode() == WXC_STATE_ACTIVE))
        {
            WXC_BeaconSendHook(work->protocol, &work->parent_param);
            /* Restart if there is no connection for a period of time */
            if (work->driver.peer_bitmap == 0)
            {
                if (++work->beacon_count > WXC_PARENT_BEACON_SEND_COUNT_OUT)
                {
                    work->beacon_count = 0;
                    /* Update the scheduler and if still in parent mode, continue */
                    if (WXCi_UpdateScheduler(&work->scheduler))
                    {
                    }
                    /* If in child mode, change state */
                    else
                    {
                        WXCi_StartChild(&work->driver);
                    }
                }
            }
        }
        break;

    case WXC_DRIVER_EVENT_BEACON_RECV:
        /*
         * Beacon detection timing (argument is WMBssDesc pointer)
         * If it is seen as connection target, return TRUE. Otherwise, return FALSE
         */
        {
            WXCBeaconFoundCallback cb;
            /* Beacon determination is first done by the protocol */
            cb.bssdesc = (const WMBssDesc *)arg;
            cb.matched = WXC_BeaconRecvHook(work->protocol, cb.bssdesc);
            /* Based on that result, the user application can also make a determination */
            (*work->system_callback) (WXC_STATE_BEACON_RECV, &cb);
            result = (u32)cb.matched;
            /* If a connection should be made based on this result, TRUE is returned here */
            if (result)
            {
                work->parent_param.ggid = WXC_GetCurrentBlock(work->protocol)->ggid;
            }
        }
        break;

    case WXC_DRIVER_EVENT_STATE_END:
        /*
         * Shut down wireless (the argument is work memory allocated to the WXC library)
         */
        work->stopping = FALSE;
        WXCi_ChangeState(WXC_STATE_END, work);
        break;

    case WXC_DRIVER_EVENT_STATE_STOP:
        /*
         * Transition to the STOP state is complete (the argument is always NULL)
         */
        work->stopping = FALSE;
        WXCi_ChangeState(WXC_STATE_READY, NULL);
        break;

    case WXC_DRIVER_EVENT_STATE_IDLE:
        /*
         * Transition to the IDLE state is complete (the argument is always NULL)
         */
        /* If preparing to end, wireless ends here */
        if (WXC_GetStateCode() != WXC_STATE_ACTIVE)
        {
            WXCi_End(&work->driver);
        }
        /* If preparing to stop, wireless stops here */
        else if (work->stopping)
        {
            WXCi_Stop(&work->driver);
        }
        /* Otherwise, switch between parent and child */
        else if (WXCi_UpdateScheduler(&work->scheduler))
        {
            /* If there is multiple data for exchange registered, GGID switches here */
            WXCi_SeekNextBlock();
            /* Initialization of parent settings compatible with GGID is done by protocol */
            WXC_BeaconSendHook(work->protocol, &work->parent_param);
            /* Parent start */
            WXCi_StartParent(&work->driver);
        }
        else
        {
            WXCi_StartChild(&work->driver);
        }
        break;

    case WXC_DRIVER_EVENT_STATE_PARENT:
    case WXC_DRIVER_EVENT_STATE_CHILD:
        /*
         * Transition to MP state is complete (the argument is always NULL)
         */
        /* If preparing to end, wireless ends here */
        if (WXC_GetStateCode() != WXC_STATE_ACTIVE)
        {
            WXCi_End(&work->driver);
        }
        /* If preparing to stop, wireless stops here */
        else if (work->stopping)
        {
            WXCi_Stop(&work->driver);
        }
        work->beacon_count = 0;
        /* Configure the local host's user settings */
        {
            WXCUserInfo *p_user = &work->user_info[work->driver.own_aid];
            p_user->aid = work->driver.own_aid;
            OS_GetMacAddress(p_user->macAddress);
        }
        break;

    case WXC_DRIVER_EVENT_CONNECTING:
        /*
         * Pre-connection notification (the argument is a WMBssDesc pointer)
         */
        {
            WMBssDesc * bss = (WMBssDesc *)arg;
            WXC_CallPreConnectHook(work->protocol, bss, &bss->ssid[8]);
        }
        break;

    case WXC_DRIVER_EVENT_CONNECTED:
        /*
         * Connection notification (the argument is a WMStartParentCallback or WMStartConnectCallback)
         */
        {
            const WXCProtocolRegistry *block = WXC_GetCurrentBlock(work->protocol);
            WMStartParentCallback *cb = (WMStartParentCallback *)arg;
            const BOOL is_parent = WXC_IsParentMode();
            const u16 parent_size =
                (u16)(is_parent ? work->parent_param.
                      parentMaxSize : WXCi_GetParentBssDesc(&work->driver)->gameInfo.parentMaxSize);
            const u16 child_size =
                (u16)(is_parent ? work->parent_param.
                      childMaxSize : WXCi_GetParentBssDesc(&work->driver)->gameInfo.childMaxSize);
            const u16 aid = (u16)(is_parent ? cb->aid : 0);
            WXCUserInfo *p_user = &work->user_info[aid];
            const BOOL is_valid_block = (!work->stopping && (WXC_GetStateCode() == WXC_STATE_ACTIVE) && block);

            /* First, copy the user information of the connection partner */
            p_user->aid = aid;
            if (is_parent)
            {
                WM_CopyBssid16(cb->macAddress, p_user->macAddress);
            }
            else
            {
                const WMBssDesc *p_bss = WXCi_GetParentBssDesc(&work->driver);
                WM_CopyBssid16(p_bss->bssid, p_user->macAddress);
            }

            /* Next, initialize 'impl' in the current MP settings */
            WXC_ResetSequence(work->protocol, parent_size, child_size);

            /*
             * If the process is not currently in shutdown and RegisterData is currently valid, automatically register data here
             * 
             */
            if (is_valid_block)
            {
                /* Data is only registered when data exists to be automatically registered */
                if ((block->send.buffer != NULL) && (block->recv.buffer != NULL))
                {
                    WXC_AddBlockSequence(work->protocol,
                                         block->send.buffer, block->send.length,
                                         block->recv.buffer, block->recv.length);
                }
            }

            /* Notify of connection to protocol */
            WXC_ConnectHook(work->protocol, (u16)(1 << aid));

            /* If necessary, callback to user application */
            if (is_valid_block)
            {
                (*work->system_callback) (WXC_STATE_CONNECTED, p_user);
            }

            /*
             * If no one has used the AddData function at this point, impl runs so that the !IsExecuting function will be true
             * 
             */
        }
        break;

    case WXC_DRIVER_EVENT_DISCONNECTED:
        /*
         * Disconnect notification
         * This function is called in three places: when WM_Disconnect completes, when indicated by WM_StartParent, and in PortRecvCallback, so there may be some overlap.
         * 
         * 
         * 
         */
        WXC_DisconnectHook(work->protocol, (u16)(u32)arg);
        /*
         * Reset when disconnecting if all data has been unregistered (for example because the WXC_End function is in the completion callback)
         * 
         * 
         */
        if (!WXC_GetUserBitmap())
        {
            if ((WXC_GetStateCode() == WXC_STATE_ACTIVE) ||
                (WXC_GetStateCode() == WXC_STATE_ENDING) || !WXC_GetCurrentBlock(work->protocol))
            {
                work->beacon_count = 0;
                WXCi_Reset(&work->driver);
            }
        }
        break;

    case WXC_DRIVER_EVENT_DATA_SEND:
        /* MP packet send hook (argument is the WXCPacketInfo pointer) */
        WXC_PacketSendHook(work->protocol, (WXCPacketInfo *) arg);
        break;

    case WXC_DRIVER_EVENT_DATA_RECV:
        /* MP packet receive hook (argument is the const WXCPacketInfo pointer) */
        (void)WXC_PacketRecvHook(work->protocol, (const WXCPacketInfo *)arg);
        /* When already disconnected in a protocol manner, reset */
        if (!work->protocol->current_block->impl->IsExecuting(work->protocol))
        {
            WXCi_Reset(&work->driver);
        }
        break;

    default:
        OS_TPanic("unchecked event implementations!");
        break;
    }

    return result;
}

/*---------------------------------------------------------------------------*
    External Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WXC_Init

  Description:  Initializes chance-encounter communications.
                Initializes the internal status.

  Arguments:    work_mem: Work memory used in the library.
                              The memory must be at least WXC_WORK_SIZE bytes and 32-byte aligned.
                              
                callback: Pointer to the callback that sends notifications for the various library system states
                              
                dma: DMA channel used in the library

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Init(void *work_mem, WXCCallback callback, u32 dma)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    // Install various protocols on the first invocation
    static BOOL initialized = FALSE;
    if (!initialized)
    {
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplCommon());
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplWPB());
        WXC_InstallProtocolImpl(WXCi_GetProtocolImplWXC());
        initialized = TRUE;
    }
    {
        if (WXC_GetStateCode() == WXC_STATE_END)
        {
            if (((u32)work_mem & 31) != 0)
            {
                OS_TPanic("WXC work buffer must be 32-byte aligned!");
            }
            work = (WXCWork *) work_mem;
            MI_CpuClear32(work, sizeof(WXCWork));
            work->wm_dma = dma;
            work->system_callback = callback;
            WXCi_InitScheduler(&work->scheduler);
            WXC_InitProtocol(work->protocol);
            /* Initialize parent settings */
            work->parent_param.maxEntry = 1;    /* Currently only supports 1 to 1 connections */
            work->parent_param.parentMaxSize = WXC_PACKET_SIZE_MAX;
            work->parent_param.childMaxSize = WXC_PACKET_SIZE_MAX;
            work->parent_param.CS_Flag = 1;     /* Continuous transmission */
            WXC_InitDriver(&work->driver, &work->parent_param, WXCi_DriverEventCallback,
                           work->wm_dma);
            WXCi_ChangeState(WXC_STATE_READY, NULL);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetParentParameter

  Description:  Specifies wireless connection settings.
                These settings are used only when in parent mode; when the local host has connected in child mode, it complies with the parent's settings.
                
                This function is optional. Call it before the WXC_Start function, and only when necessary.
                

  Arguments:    sendSize: Parent send size.
                              It must be at least WXC_PACKET_SIZE_MIN and no more than WXC_PACKET_SIZE_MAX.
                              
                recvSize: Parent receive size.
                              It must be at least WXC_PACKET_SIZE_MIN and no more than WXC_PACKET_SIZE_MAX.
                              
                maxEntry: Maximum number of connected players.
                              The current implementation supports only a specified value of 1.

  Returns:      If the settings are valid, they are applied, and TRUE is returned. Otherwise, a warning message is sent to debug output, and FALSE is returned.
                
 *---------------------------------------------------------------------------*/
BOOL WXC_SetParentParameter(u16 sendSize, u16 recvSize, u16 maxEntry)
{
    BOOL    ret = FALSE;

    /* The current implementation only supports communication in pairs */
    if (maxEntry > 1)
    {
        OS_TWarning("unsupported maxEntry. (must be 1)\n");
    }
    /* Determine packet size */
    else if ((sendSize < WXC_PACKET_SIZE_MIN) || (sendSize > WXC_PACKET_SIZE_MAX) ||
             (recvSize < WXC_PACKET_SIZE_MIN) || (recvSize > WXC_PACKET_SIZE_MAX))
    {
        OS_TWarning("packet size is out of range.\n");
    }
    /* Determine time required for MP communications */
    else
    {
        /* Evaluation for the time taken to perform MP communications once */
        int     usec = 330 + 4 * (sendSize + 38) + maxEntry * (112 + 4 * (recvSize + 32));
        const int max_time = 5600;
        if (usec >= max_time)
        {
            OS_TWarning("specified MP setting takes %d[usec]. (should be lower than %d[usec])\n",
                        usec, max_time);
        }
        else
        {
            work->parent_param.parentMaxSize = sendSize;
            work->parent_param.childMaxSize = recvSize;
            ret = TRUE;
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_Start

  Description:  Starts chance-encounter communications.
                Starts the internal wireless driver.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Start(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_END);

        if (WXC_GetStateCode() == WXC_STATE_READY)
        {
            WXCi_Reset(&work->driver);
            WXCi_ChangeState(WXC_STATE_ACTIVE, NULL);
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_Stop

  Description:  Stops chance-encounter communications.
                Stops the internal wireless driver.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_Stop(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_END);

        switch (WXC_GetStateCode())
        {

        case WXC_STATE_ACTIVE:
            if (!work->stopping)
            {
                work->stopping = TRUE;
                /* If currently in a connection, wait to complete while continually specifying a stop request */
                if (WXC_GetUserBitmap() == 0)
                {
                    WXCi_Stop(&work->driver);
                }
            }
            break;

        case WXC_STATE_READY:
        case WXC_STATE_ENDING:
        case WXC_STATE_END:
            /* If shutdown is in progress or complete, does not do anything */
            break;

        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);

}

/*---------------------------------------------------------------------------*
  Name:         WXC_End

  Description:  Ends chance-encounter communications.
                Ends the internal wireless driver.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_End(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        switch (WXC_GetStateCode())
        {

        case WXC_STATE_READY:
        case WXC_STATE_ACTIVE:
            WXCi_ChangeState(WXC_STATE_ENDING, NULL);
            /* If currently in a connection, wait to complete while continually specifying an end request */
            if (WXC_GetUserBitmap() == 0)
            {
                WXCi_End(&work->driver);
            }
            break;

        case WXC_STATE_ENDING:
        case WXC_STATE_END:
            /* If shutdown is in progress or complete, does not do anything */
            break;

        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetStateCode

  Description:  Gets the current state of the library.

  Arguments:    None.

  Returns:      WXCStateCode type that shows the current library state.
 *---------------------------------------------------------------------------*/
WXCStateCode WXC_GetStateCode(void)
{
    return state;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_IsParentMode

  Description:  Determines whether the current wireless state is parent mode.
                Only valid in the WXC_STATE_ACTIVE state.

  Arguments:    None.

  Returns:      If the wireless status is parent mode, TRUE. If it is child mode, FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_IsParentMode(void)
{
    return WXCi_IsParentMode(&work->driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentParam

  Description:  Accesses the WMParentParam structure for the current wireless state.

  Arguments:    None.

  Returns:      WMParentParam structure.
 *---------------------------------------------------------------------------*/
const WMParentParam *WXC_GetParentParam(void)
{
    return &work->parent_param;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetParentBssDesc

  Description:  Accesses the WMBssDesc structure for the current connection target.

  Arguments:    None.

  Returns:      WMBssDesc structure.
 *---------------------------------------------------------------------------*/
const WMBssDesc *WXC_GetParentBssDesc(void)
{
    return WXCi_GetParentBssDesc(&work->driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserBitmap

  Description:  Gets the status of the currently connected users with a bitmap.
                

  Arguments:    None.

  Returns:      WMBssDesc structure.
 *---------------------------------------------------------------------------*/
u16 WXC_GetUserBitmap(void)
{
    u16     bitmap = work->driver.peer_bitmap;
    if (bitmap != 0)
    {
        bitmap = (u16)(bitmap | (1 << work->driver.own_aid));
    }
    return bitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentGgid

  Description:  Gets the selected GGID in the current connection.
                If communications are not established, it returns 0.
                The current communications status can be determined by the WXC_GetUserBitmap function's return value.

  Arguments:    None.

  Returns:      The selected GGID in the current connection.
 *---------------------------------------------------------------------------*/
u32 WXC_GetCurrentGgid(void)
{
    u32     ggid = 0;
    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        WXCProtocolRegistry *current = WXC_GetCurrentBlock(work->protocol);
        if (current)
        {
            ggid = current->ggid;
            /* If 'ANY', then imitate the detected peer's GGID */
            if (ggid == WXC_GGID_COMMON_ANY)
            {
                ggid = work->protocol->target_ggid;
            }
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return ggid;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetOwnAid

  Description:  Gets the local host's own AID in the current connection.
                If communications are not established, it returns an undefined value.
                The current communications status can be determined by the WXC_GetUserBitmap function's return value.

  Arguments:    None.

  Returns:      Local host's AID in the current connection.
 *---------------------------------------------------------------------------*/
u16 WXC_GetOwnAid(void)
{
    return work->driver.own_aid;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetUserInfo

  Description:  Gets the user information of a user currently connected.
                Do not change the content indicated by the pointer that is returned.

  Arguments:    aid: AID of user whose information is obtained

  Returns:      If the specified AID is valid, user information. Otherwise, NULL.
 *---------------------------------------------------------------------------*/
const WXCUserInfo *WXC_GetUserInfo(u16 aid)
{
    const WXCUserInfo *ret = NULL;

    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        if ((aid <= WM_NUM_MAX_CHILD) && ((WXC_GetUserBitmap() & (1 << aid)) != 0))
        {
            ret = &work->user_info[aid];
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetChildMode

  Description:  Sets wireless so that it can only be started on the child side.
                This function must be called before the WXC_Start function is called.

  Arguments:    enable: If it can only be started on the child side, TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetChildMode(BOOL enable)
{
    SDK_ASSERT(WXC_GetStateCode() != WXC_STATE_ACTIVE);
    WXCi_SetChildMode(&work->scheduler, enable);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_AddData

  Description:  Sets data to add to a completed block data exchange.

  Arguments:    send_buf: Send buffer
                send_size: Send buffer size
                recv_buf: Receive buffer
                recv_max: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_AddData(const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max)
{
    WXC_AddBlockSequence(work->protocol, send_buf, send_size, recv_buf, recv_max);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_RegisterDataEx

  Description:  Registers data for exchange.

  Arguments:    ggid: GGID of the registered data
                callback: Callback function to the user (invoked when data exchange is complete)
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size
                type: Chance-encounter protocol type (PIType)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_RegisterDataEx(u32 ggid, WXCCallback callback, u8 *send_ptr, u32 send_size, u8 *recv_ptr,
                        u32 recv_size, const char *type)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /*
     * Do not allow the same GGID to be registered more than once
     * (because the child device cannot determine which one to use when connecting)
     */
    WXCProtocolRegistry *same_ggid = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    if (same_ggid != NULL)
    {
        OS_TWarning("already registered same GGID!");
    }
    /* If not, use an empty block */
    else
    {
        WXCProtocolRegistry *p_data = WXC_FindNextBlock(work->protocol, NULL, 0, TRUE);
        /* If there is no empty block, fail */
        if (!p_data)
        {
            OS_TPanic("no more memory to register data!");
        }
        else
        {
            /* Change the communication protocol to register, based on the selected protocol */
            WXCProtocolImpl *impl = WXC_FindProtocolImpl(type);
            if (!impl)
            {
                OS_TPanic("unknown protocol \"%s\"!", type);
            }
            WXC_InitProtocolRegistry(p_data, ggid, callback, impl);
            WXC_SetInitialExchangeBuffers(p_data, send_ptr, send_size, recv_ptr, recv_size);
        }
    }

    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialData

  Description:  Specifies the data block used for every exchange, based on the first exchange.

  Arguments:    ggid: GGID of the registered data
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetInitialData(u32 ggid, u8 *send_ptr, u32 send_size, u8 *recv_ptr, u32 recv_size)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /* Searches for the specified block */
    WXCProtocolRegistry *target = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    if (target)
    {
        WXC_SetInitialExchangeBuffers(target, send_ptr, send_size, recv_ptr, recv_size);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_UnregisterData

  Description:  Deletes data for exchange from registration.

  Arguments:    ggid: GGID related to the deleted block

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_UnregisterData(u32 ggid)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    /* Searches for the specified block */
    WXCProtocolRegistry *target = WXC_FindNextBlock(work->protocol, NULL, ggid, TRUE);
    /* Searches once more, including when this is a shared chance-encounter communication specification */
    if (!target)
    {
        target = WXC_FindNextBlock(work->protocol, NULL, (u32)(ggid | WXC_GGID_COMMON_BIT), TRUE);
    }
    if (target)
    {
        /* If it is being used in the current connection, fail */
        if ((WXC_GetUserBitmap() != 0) && (target == WXC_GetCurrentBlock(work->protocol)))
        {
            OS_TWarning("specified data is now using.");
        }
        else
        {
            /* Releases data */
            target->ggid = 0;
            target->callback = NULL;
            target->send.buffer = NULL;
            target->send.length = 0;
            target->recv.buffer = NULL;
            target->recv.length = 0;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetSsid

  Description:  Configures the SSID when connecting as a child.
                Internal function.

  Arguments:    buffer: SSID data to configure
                length: SSID data length
                              Must be no more than WM_SIZE_CHILD_SSID.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_SetSsid(const void *buffer, u32 length)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    WXC_SetDriverSsid(&work->driver, buffer, length);
    (void)OS_RestoreInterrupts(bak_cpsr);
}

