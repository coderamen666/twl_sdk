/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_driver.c

  Copyright 2005-2009 Nintendo. All rights reserved.

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

#include <nitro/wxc/common.h>
#include <nitro/wxc/driver.h>


/*****************************************************************************/
/* Declaration */

/* Error check function group */
static void WXCi_RecoverWmApiFailure(WXCDriverWork *, WMApiid id, WMErrCode err);
static BOOL WXCi_CheckWmApiResult(WXCDriverWork * driver, WMApiid id, WMErrCode err);
static BOOL WXCi_CheckWmCallbackResult(WXCDriverWork *, void *arg);
static void WXCi_ErrorQuit(WXCDriverWork *);

/*
 * Continuous transition procedure between wireless states.
 * It is first called with an argument of NULL and then makes repeated callbacks to the same function.
 * An event notification callback occurs when the intended state is reached.
 *
 * Note:
 *   These functions have been designed to ultimately make threading easy, so during maintenance you should not split up individual WM function calls within these functions nor separate them into StateIn- and StateOut-.
 *   
 *   
 */
static void WXCi_InitProc(void *arg);  /* (end) -> STOP             */
static void WXCi_StartProc(void *arg); /* STOP -> IDLE              */
static void WXCi_StopProc(void *arg);  /* IDLE -> STOP              */
static void WXCi_EndProc(void *arg);   /* STOP  -> (end)            */
static void WXCi_ResetProc(void *arg); /* (any) -> IDLE             */
static void WXCi_StartParentProc(void *arg);    /* IDLE  -> MP_PARENT        */
static void WXCi_StartChildProc(void *arg);     /* IDLE  -> MP_CHILD         */
static void WXCi_ScanProc(void *arg);  /* IDLE -> SCAN -> IDLE      */
static void WXCi_MeasureProc(void *arg);        /* IDLE -> (measure) -> IDLE */

/* State transition control */
static void WXCi_OnStateChanged(WXCDriverWork *, WXCDriverState state, void *arg);

/* Other callbacks or indicators */
static void WXCi_IndicateCallback(void *arg);
static void WXCi_PortCallback(void *arg);
static void WXCi_MPSendCallback(void *arg);
static void WXCi_ParentIndicate(void *arg);
static void WXCi_ChildIndicate(void *arg);

/*****************************************************************************/
/* Constants */

/* In SDK 3.0 RC and later, behavior was changed so that an indicator is generated even for the local host's disconnection. */
#define VERSION_TO_INT(major, minor, relstep) \
    (((major) << 24) | ((minor) << 16) | ((relstep) << 0))
#if VERSION_TO_INT(SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_RELSTEP) < VERSION_TO_INT(3, 0, 20100)
#define WM_STATECODE_DISCONNECTED_FROM_MYSELF ((WMStateCode)26)
#endif


/*****************************************************************************/
/* Variables */

/*
 * Work memory for the wireless driver.
 * - This variable has been provided to use as "this" only in WM callbacks.
 *   (Normally, WXCDriverWork is taken as an argument to explicitly specify the invocation target whenever possible.)
 * - Because directly referencing a global pointer causes it to be treated as semi-volatile and is inefficient, it is better to temporarily copy this pointer into a local variable in functions that use it frequently.
 *   
 */
static WXCDriverWork *work;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXCi_ErrorQuit

  Description:  Resets when an error is detected.

  Arguments:    driver: WXCDriverWork structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ErrorQuit(WXCDriverWork * driver)
{
    /* This usually ends up as an abnormal termination, so ignore BUSY */
    if (driver->state == WXC_DRIVER_STATE_BUSY)
    {
        driver->state = driver->target;
    }
    WXCi_End(driver);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_RecoverWmApiFailure

  Description:  Attempts state recovery after WM function errors.

  Arguments:    driver: WXCDriverWork structure
                id: WM function types
                err: Error code returned from the function

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_RecoverWmApiFailure(WXCDriverWork * driver, WMApiid id, WMErrCode err)
{
    /* Just in case, success is also determined here */
    if (err == WM_ERRCODE_SUCCESS)
    {
        return;
    }

    switch (id)
    {
    default:
        /* Functions which should not be used in WXC */
        OS_TWarning("WXC library error (unknown WM API : %d)\n", id);
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_ENABLE:
    case WM_APIID_POWER_ON:
    case WM_APIID_POWER_OFF:
    case WM_APIID_DISABLE:
    case WM_APIID_RESET:
        /* If it failed to even initialize, end it */
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_MEASURE_CHANNEL:
    case WM_APIID_SET_P_PARAM:
    case WM_APIID_START_PARENT:
    case WM_APIID_START_SCAN_EX:
    case WM_APIID_END_SCAN:
    case WM_APIID_DISCONNECT:
        /* If an unexpected failure occurred in a major state transition function, end */
        WXCi_ErrorQuit(driver);
        break;

    /* If the parent disconnected at the WM_StartMP stage, WM_PowerOff will unfortunately be called without the system going into the IDLE state, so forcefully change the WXC state so that WM_Reset is run.
        */
    case WM_APIID_START_MP:
        driver->state = WXC_DRIVER_STATE_CHILD; // Cancel BUSY
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_START_CONNECT:
        /* It is possible for connection to fail when StartConnect completes, so go to IDLE again */
        if ((err == WM_ERRCODE_FAILED) ||
            (err == WM_ERRCODE_NO_ENTRY) || (err == WM_ERRCODE_OVER_MAX_ENTRY))
        {
            /* Only for this moment, cancel BUSY */
            driver->state = WXC_DRIVER_STATE_CHILD;
            WXCi_Reset(driver);
        }
        /* Judge anything else to be a fatal error, and end */
        else
        {
            WXCi_ErrorQuit(driver);
        }
        break;

    case WM_APIID_INDICATION:
        /* If a fatal notification such as FIFO_ERROR or FLASH_ERROR is received, end */
        WXCi_ErrorQuit(driver);
        break;

    case WM_APIID_SET_MP_DATA:
    case WM_APIID_PORT_SEND:
        /*
         * The following are cases in which SetMPData fails.
         *   ILLEGAL_STATE, INVALID_PARAM, FIFO_ERROR: Always
         *   NO_CHILD: When the function is called
         *   SEND_QUEUE_FULL, SEND_FAILED: During callback
         * This problem is resolvable by retrying, so ignore it.
         */
        break;

    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CheckWmApiResult

  Description:  Evaluates the return value of WM functions.

  Arguments:    driver: WXCDriverWork structure
                id: WM function types
                err: Error code returned from the function

  Returns:      If WM_ERRCODE_SUCCESS: TRUE.
                Otherwise, output error: FALSE.
 *---------------------------------------------------------------------------*/
static BOOL WXCi_CheckWmApiResult(WXCDriverWork * driver, WMApiid id, WMErrCode err)
{
    BOOL    ret = WXC_CheckWmApiResult(id, err);
    /* Recovery operation for each API when there has been an error */
    if (!ret)
    {
        WXCi_RecoverWmApiFailure(driver, id, err);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CheckWmCallbackResult

  Description:  Evaluates the return value of WM callbacks.

  Arguments:    driver: WXCDriverWork structure
                arg: Argument returned from the function

  Returns:      If WM_ERRCODE_SUCCESS: TRUE.
                Otherwise, output error: FALSE.
 *---------------------------------------------------------------------------*/
static BOOL WXCi_CheckWmCallbackResult(WXCDriverWork * driver, void *arg)
{
    BOOL    ret = WXC_CheckWmCallbackResult(arg);
    /* Recovery operation for each API when there has been an error */
    if (!ret)
    {
        const WMCallback *cb = (const WMCallback *)arg;
        WXCi_RecoverWmApiFailure(driver, (WMApiid)cb->apiid, (WMErrCode)cb->errcode);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CallDriverEvent

  Description:  Notifies of a wireless driver event.

  Arguments:    driver: WXCDriverWork structure
                event: Notified event
                arg: Argument defined corresponding to event

  Returns:      u32-type event result value, defined corresponding to each event.
 *---------------------------------------------------------------------------*/
static inline u32 WXCi_CallDriverEvent(WXCDriverWork * driver, WXCDriverEvent event, void *arg)
{
    u32     result = 0;
    if (driver->callback)
    {
        result = (*driver->callback) (event, arg);
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_CallSendEvent

  Description:  Callback function for the data send callback event.

  Arguments:    driver: WXCDriverWork structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_CallSendEvent(WXCDriverWork * driver)
{
    if (!driver->send_busy)
    {
        /*
         * Set information for the send packet buffer
         * TODO:
         * Would this be more efficient if it saved once when MP communications are established?
         * The same process actually exists in wxc_api.c.
         */
        const u16 max_length = (u16)((driver->own_aid == 0) ?
            driver->parent_param->parentMaxSize : driver->target_bss->gameInfo.childMaxSize);
        WXCPacketInfo packet;
        packet.bitmap = driver->peer_bitmap;
        packet.length = max_length;
        packet.buffer = driver->current_send_buf;
        /* Have the user set send data */
        (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DATA_SEND, &packet);
        /* Send the data */
        if ((packet.length <= max_length)&&((driver->state==WXC_DRIVER_STATE_PARENT)||(driver->state==WXC_DRIVER_STATE_CHILD)))
        {
            WMErrCode ret;
            ret = WM_SetMPDataToPort(WXCi_MPSendCallback,
                                     (u16 *)packet.buffer, packet.length,
                                     packet.bitmap, WXC_DEFAULT_PORT, WXC_DEFAULT_PORT_PRIO);
            driver->send_busy = WXCi_CheckWmApiResult(driver, WM_APIID_SET_MP_DATA, ret);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_OnStateChanged

  Description:  Process when system is stable in the specified state.

  Arguments:    driver: WXCDriverWork structure
                state: Transition is the defined state
                arg: Argument for each event

  Returns:      u32-type event result value, defined corresponding to each event.
 *---------------------------------------------------------------------------*/
static void WXCi_OnStateChanged(WXCDriverWork * driver, WXCDriverState state, void *arg)
{
    driver->state = state;

    /* Notify the user if the current state is the transition target */
    if (driver->target == state)
    {
        switch (state)
        {
        case WXC_DRIVER_STATE_END:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_END, NULL);
            break;

        case WXC_DRIVER_STATE_STOP:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_STOP, NULL);
            break;

        case WXC_DRIVER_STATE_IDLE:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_IDLE, NULL);
            break;

        case WXC_DRIVER_STATE_PARENT:
            driver->send_busy = FALSE;
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_PARENT, NULL);
            break;

        case WXC_DRIVER_STATE_CHILD:
            driver->send_busy = FALSE;
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_STATE_CHILD, NULL);
            /* Here, the connection with the parent is notified */
            driver->peer_bitmap |= (u16)(1 << 0);
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTED, arg);
            WXCi_CallSendEvent(driver);
            break;
        }
    }

    /*
     * Otherwise, continue transitioning
     * +-----+      +------+       +------+                     +--------+
     * |     > Init |      > Start >      > Measure/StartParent > PARENT |
     * |     |      |      |       |      <               Reset <        |
     * |     |      |      |       |      |                     +--------+
     * | END |      | STOP |       | IDLE > Scan/StartChild     > CHILD  |
     * |     |      |      |       |      <               Reset <        |
     * |     |      |      <  Stop <      |                     +--------+
     * |     <  End <      <   End <      |
     * +-----+      +------+       +------+
     */
    else
    {
        switch (state)
        {

        case WXC_DRIVER_STATE_END:
            WXCi_InitProc(NULL);
            break;

        case WXC_DRIVER_STATE_STOP:
            switch (driver->target)
            {
            case WXC_DRIVER_STATE_END:
                WXCi_EndProc(NULL);
                break;
            case WXC_DRIVER_STATE_IDLE:
            case WXC_DRIVER_STATE_PARENT:
            case WXC_DRIVER_STATE_CHILD:
                WXCi_StartProc(NULL);
                break;
            }
            break;

        case WXC_DRIVER_STATE_IDLE:
            switch (driver->target)
            {
            case WXC_DRIVER_STATE_END:
            case WXC_DRIVER_STATE_STOP:
                WXCi_StopProc(NULL);
                break;
            case WXC_DRIVER_STATE_PARENT:
                driver->need_measure_channel = TRUE;
                if (driver->need_measure_channel)
                {
                    WXCi_MeasureProc(NULL);
                }
                break;
            case WXC_DRIVER_STATE_CHILD:
                WXCi_ScanProc(NULL);
                break;
            }
            break;

        case WXC_DRIVER_STATE_PARENT:
        case WXC_DRIVER_STATE_CHILD:
            WXCi_ResetProc(NULL);
            break;

        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MPSendCallback

  Description:  The callback function to WM_SetMPData.

  Arguments:    arg: Pointer to the callback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MPSendCallback(void *arg)
{
    WXCDriverWork *const driver = work;

    (void)WXCi_CheckWmCallbackResult(driver, arg);
    driver->send_busy = FALSE;
    if (driver->peer_bitmap != 0)
    {
        WXCi_CallSendEvent(driver);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IndicateCallback

  Description:  The callback function called when Indicate occurs.

  Arguments:    arg: Pointer to the callback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_IndicateCallback(void *arg)
{
    /* TODO: Can't this be integrated with CheckError? */
    WXCDriverWork *const driver = work;
    WMIndCallback *cb = (WMIndCallback *)arg;
    if (cb->errcode == WM_ERRCODE_FIFO_ERROR)
    {
        WXC_DRIVER_LOG("WM_ERRCODE_FIFO_ERROR Indication!\n");
        /* Unrecoverable error */
        driver->target = WXC_DRIVER_STATE_ERROR;
        driver->state = WXC_DRIVER_STATE_ERROR;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PortCallback

  Description:  The reception notification to the port.

  Arguments:    arg: Pointer to the callback structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_PortCallback(void *arg)
{
    WXCDriverWork *const driver = work;

    if (WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;
        switch (cb->state)
        {
        case WM_STATECODE_PORT_RECV:
            {
                WXCPacketInfo packet;
                packet.bitmap = (u16)(1 << cb->aid);
                packet.length = cb->length;
                packet.buffer = (u8 *)cb->data;
                (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DATA_RECV, &packet);
            }
            break;
        case WM_STATECODE_CONNECTED:
            break;
        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            WXC_DRIVER_LOG("disconnected(%02X-=%02X)\n", driver->peer_bitmap, (1 << cb->aid));
            driver->peer_bitmap &= (u16)~(1 << cb->aid);
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_DISCONNECTED,
                                       (void *)(1 << cb->aid));
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitProc

  Description:  Successive transition from READY -> STOP.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_InitProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* (outof) -> STOP */
        if (!arg)
        {
            /* Currently, MeasureChannel only runs once at startup */
            driver->need_measure_channel = TRUE;
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Init(driver->wm_work, driver->wm_dma);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_INITIALIZE, wmResult);
            wmResult = WM_Enable(WXCi_InitProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_ENABLE, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_ENABLE)
        {
            /* Receive each type of notification */
            wmResult = WM_SetIndCallback(WXCi_IndicateCallback);
            if (WXCi_CheckWmApiResult(driver, WM_APIID_INDICATION, wmResult))
            {
                /* Sets the port callback */
                wmResult = WM_SetPortCallback(WXC_DEFAULT_PORT, WXCi_PortCallback, NULL);
                if (WXCi_CheckWmApiResult(driver, WM_APIID_PORT_SEND, wmResult))
                {
                    WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_STOP, NULL);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartProc

  Description:  Successive transition from STOP -> IDLE.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* STOP -> IDLE */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_PowerOn(WXCi_StartProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_POWER_ON, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_POWER_ON)
        {
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StopProc

  Description:  Successive transition from IDLE -> STOP.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StopProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> STOP */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_PowerOff(WXCi_StopProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_POWER_OFF, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_POWER_OFF)
        {
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_STOP, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_EndProc

  Description:  Successive transition from STOP -> READY.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_EndProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* STOP -> READY */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Disable(WXCi_EndProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_DISABLE, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_DISABLE)
        {
            /* Notification of a complete release of wireless */
            wmResult = WM_Finish();
            if (WXCi_CheckWmApiResult(driver, WM_APIID_END, wmResult))
            {
                work = NULL;
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_END, NULL);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ResetProc

  Description:  Successive transition from (any) -> IDLE.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ResetProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* (any) -> IDLE */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_Reset(WXCi_ResetProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_RESET, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_RESET)
        {
            driver->own_aid = 0;
            WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ParentIndicate

  Description:  Parent device's StartParent indicator.

  Arguments:    arg: Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ParentIndicate(void *arg)
{
    WXCDriverWork *const driver = work;
    WMStartParentCallback *cb = (WMStartParentCallback *)arg;

    /* When making just a state transition, ignore this and continue the sequence */
    if (cb->state == WM_STATECODE_PARENT_START)
    {
        WXCi_StartParentProc(arg);
    }
    /* Otherwise, they're indicators, so hook to wxc_api.c */
    else if (cb->errcode == WM_ERRCODE_SUCCESS)
    {
        switch (cb->state)
        {
        case WM_STATECODE_PARENT_START:
            break;
        case WM_STATECODE_CONNECTED:
            {
                BOOL    mp_start = (driver->peer_bitmap == 0);
                WXC_DRIVER_LOG("connected(%02X+=%02X)\n", driver->peer_bitmap, (1 << cb->aid));
                driver->peer_bitmap |= (u16)(1 << cb->aid);
                (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTED, cb);
                /* If it's the first child device, notify of the send timing */
                if (mp_start)
                {
                    WXCi_CallSendEvent(driver);
                }
            }
            break;
        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            /* Disconnect notification operations were unified in PortCallback */
            break;
        case WM_STATECODE_BEACON_SENT:
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_BEACON_SEND, driver->parent_param);
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartParentProc

  Description:  Successive transition from IDLE -> PARENT -> MP_PARENT.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartParentProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> IDLE (WM_SetParentParameter) */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            /* Updates channels and startup information */
            driver->parent_param->channel = driver->current_channel;
            driver->parent_param->tgid = WM_GetNextTgid();
            WXC_DRIVER_LOG("start parent. (%2dch, TGID=%02X, GGID=%04X)\n",
                           driver->current_channel, driver->parent_param->tgid,
                           driver->parent_param->ggid);
            wmResult = WM_SetParentParameter(WXCi_StartParentProc, driver->parent_param);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_SET_P_PARAM, wmResult);
        }
        /* IDLE -> PARENT */
        else if (cb->apiid == WM_APIID_SET_P_PARAM)
        {
            /*
             * Because of the indicator, redirect the callback to WXCi_ParentIndicate()
             * 
             */
            wmResult = WM_StartParent(WXCi_ParentIndicate);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_PARENT, wmResult);
        }
        /* PARENT -> MP_PARENT */
        else if (cb->apiid == WM_APIID_START_PARENT)
        {
            /*
             * WM_STATECODE_PARENT_START is always the only notification sent from WXCi_ParentIndicate() here
             * 
             */
            driver->own_aid = 0;
            driver->peer_bitmap = 0;
            wmResult = WM_StartMP(WXCi_StartParentProc,
                                  (u16 *)driver->mp_recv_work, driver->recv_size_max,
                                  (u16 *)driver->mp_send_work, driver->send_size_max,
                                  (u16)(driver->parent_param->CS_Flag ? 0 : 1));
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_MP, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_PARENT, NULL);
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ChildIndicate

  Description:  Child device's StartConnect indicator.

  Arguments:    arg: Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ChildIndicate(void *arg)
{
    WXCDriverWork *const driver = work;

    if (WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
        switch (cb->state)
        {
        case WM_STATECODE_CONNECT_START:
        case WM_STATECODE_BEACON_LOST:
            break;

        case WM_STATECODE_CONNECTED:
            /* During state transitions, ignore this and continue the sequence */
            if (driver->state != WXC_DRIVER_STATE_CHILD)
            {
                WXCi_StartChildProc(arg);
            }
            break;

        case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        case WM_STATECODE_DISCONNECTED:
            /* If a transition is not currently in progress, reset here */
            if (driver->state != WXC_DRIVER_STATE_BUSY)
            {
                driver->target = WXC_DRIVER_STATE_PARENT;
                WXCi_ResetProc(NULL);
            }
            else
            {
                driver->target = WXC_DRIVER_STATE_IDLE;
            }
            break;

        default:
            WXCi_ErrorQuit(driver);
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_StartChildProc

  Description:  Successive transition from IDLE -> CHILD -> MP_CHILD.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_StartChildProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;
        /* IDLE -> CHILD */
        if (!arg)
        {
            /*
             * This is a hook invocation in order to set the SSID and other information before connecting.
             */
            u8      ssid_bak[WM_SIZE_BSSID];
            MI_CpuCopy8(driver->target_bss->ssid, ssid_bak, sizeof(ssid_bak));
            (void)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_CONNECTING, driver->target_bss);
            MI_CpuCopy8(driver->target_bss->ssid + 8, driver->ssid, WM_SIZE_CHILD_SSID);
            MI_CpuCopy8(ssid_bak, driver->target_bss->ssid, sizeof(ssid_bak));
            /*
             * Because of the indicator, redirect the callback to WXCi_ChildIndicate()
             * 
             */
            driver->state = WXC_DRIVER_STATE_BUSY;
            wmResult = WM_StartConnect(WXCi_ChildIndicate, driver->target_bss, driver->ssid);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_CONNECT, wmResult);
        }
        /* CHILD -> MP_CHILD */
        else if (cb->apiid == WM_APIID_START_CONNECT)
        {
            WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;
            /*
             * WM_STATECODE_CONNECTED is always the only notification sent from WXCi_ChildIndicate() here
             * 
             */
            driver->own_aid = cb->aid;
            wmResult = WM_StartMP(WXCi_StartChildProc,
                                  (u16 *)driver->mp_recv_work, driver->recv_size_max,
                                  (u16 *)driver->mp_send_work, driver->send_size_max,
                                  (u16)(driver->parent_param->CS_Flag ? 0 : 1));
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_MP, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb = (WMStartMPCallback *)arg;
            switch (cb->state)
            {
            case WM_STATECODE_MP_START:
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_CHILD, cb);
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MeasureProc

  Description:  Successive transition from IDLE -> (measure) -> IDLE.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MeasureProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMMeasureChannelCallback *cb = (WMMeasureChannelCallback *)arg;
    u16     channel = 0;

    /* Initializes the measurement value */
    if (!arg)
    {
        driver->state = WXC_DRIVER_STATE_BUSY;
        driver->current_channel = 0;
        driver->measure_ratio_min = WXC_MAX_RATIO + 1;
    }
    else if (WXCi_CheckWmCallbackResult(driver, cb))
        /* Measurement complete callback */
    {
        channel = cb->channel;
        /* Get a channel with a lower usage rate (initial value is 101%, so be sure to select the top) */
        if (driver->measure_ratio_min > cb->ccaBusyRatio)
        {
            driver->measure_ratio_min = cb->ccaBusyRatio;
            driver->current_channel = channel;
        }
        /* All channels measurement completed */
        if (channel == (32 - MATH_CountLeadingZeros(WM_GetAllowedChannel())))
        {
            driver->need_measure_channel = FALSE;
            /* If PARENT is still currently the target, transition to connection */
            //WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
            WXCi_StartParentProc(NULL);
            return;
        }
    }
    /* Measurement failure (go to error shutdown) */
    else
    {
        driver->need_measure_channel = FALSE;
    }

    /* Measures all channels in order from the last */
    if (driver->need_measure_channel)
    {
        /* The time interval in ms for picking up the signal for making one communication for one frame */
        const u16 WH_MEASURE_TIME = 30;
        /* Bitwise OR of the carrier sense and the ED value */
        const u16 WH_MEASURE_CS_OR_ED = 3;
        /* The recommended ED threshold value that has been empirically shown to be effective */
        const u16 WH_MEASURE_ED_THRESHOLD = 17;
        WMErrCode ret;

        channel = WXC_GetNextAllowedChannel(channel);
        ret = WM_MeasureChannel(WXCi_MeasureProc,
                                WH_MEASURE_CS_OR_ED, WH_MEASURE_ED_THRESHOLD,
                                channel, WH_MEASURE_TIME);
        (void)WXCi_CheckWmApiResult(driver, WM_APIID_MEASURE_CHANNEL, ret);
    }

}

/*---------------------------------------------------------------------------*
  Name:         WXCi_ScanProc

  Description:  Successive transition from IDLE -> SCAN -> IDLE.

  Arguments:    arg: Callback argument (specify NULL on invocation)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_ScanProc(void *arg)
{
    WXCDriverWork *const driver = work;
    WMCallback *cb = (WMCallback *)arg;
    
    static u16 scan_channelList;

    if (!arg || WXCi_CheckWmCallbackResult(driver, arg))
    {
        WMErrCode wmResult = WM_ERRCODE_SUCCESS;

        /* IDLE -> SCAN */
        if (!arg)
        {
            driver->state = WXC_DRIVER_STATE_BUSY;
            driver->scan_found_num = 0;
            /* Searches for all channels using the broadcast addresses */
            driver->scan_param->scanBuf = (WMBssDesc *)driver->scan_buf;
            driver->scan_param->scanBufSize = WM_SIZE_SCAN_EX_BUF;
            
            /* Set to single channel per search */
            scan_channelList = WM_GetAllowedChannel();
            driver->scan_param->channelList = (u16)MATH_GetLeastSignificantBit(scan_channelList);
            scan_channelList = (u16)(scan_channelList ^ MATH_GetLeastSignificantBit(scan_channelList));
            
            driver->scan_param->maxChannelTime = WXC_SCAN_TIME_MAX;
            MI_CpuFill8(driver->scan_param->bssid, 0xFF, sizeof(driver->scan_param->bssid));
            driver->scan_param->scanType = WM_SCANTYPE_PASSIVE;
            driver->scan_param->ssidLength = 0;
            MI_CpuFill8(driver->scan_param->ssid, 0xFF, sizeof(driver->scan_param->ssid));
            wmResult = WM_StartScanEx(WXCi_ScanProc, driver->scan_param);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_SCAN_EX, wmResult);
        }
        /* SCAN -> IDLE */
        else if (cb->apiid == WM_APIID_START_SCAN_EX)
        {
            WMStartScanExCallback *cb = (WMStartScanExCallback *)arg;
            /* Saves if the beacon is detected */
            if (cb->state == WM_STATECODE_PARENT_FOUND)
            {
                DC_InvalidateRange(driver->scan_buf, WM_SIZE_SCAN_EX_BUF);
                driver->scan_found_num = cb->bssDescCount;
            }
            wmResult = WM_EndScan(WXCi_ScanProc);
            (void)WXCi_CheckWmApiResult(driver, WM_APIID_END_SCAN, wmResult);
        }
        /* End */
        else if (cb->apiid == WM_APIID_END_SCAN)
        {
            BOOL    ret = FALSE;
            /* Evaluate BssDesc if the target is still CHILD */
            if (driver->target == WXC_DRIVER_STATE_CHILD)
            {
                int     i;
                const u8 *scan_buf = driver->scan_buf;

                WXC_DRIVER_LOG("found:%d beacons\n", driver->scan_found_num);
                for (i = 0; i < driver->scan_found_num; ++i)
                {
                    const WMBssDesc *p_desc = (const WMBssDesc *)scan_buf;
                    const int len = p_desc->length * 2;
                    BOOL    is_valid;
                    is_valid = WM_IsValidGameBeacon(p_desc);
                    WXC_DRIVER_LOG("   GGID=%08X(%2dch:%3dBYTE)\n",
                                   is_valid ? p_desc->gameInfo.ggid : 0xFFFFFFFF,
                                   p_desc->channel, len);
                    if (is_valid)
                    {
                        /* Callback for each BssDesc */
                        ret =
                            (BOOL)WXCi_CallDriverEvent(driver, WXC_DRIVER_EVENT_BEACON_RECV,
                                                       (void *)p_desc);
                        if (ret)
                        {
                            WXC_DRIVER_LOG("     -> matched!\n");
                            MI_CpuCopy8(p_desc, driver->target_bss, sizeof(WMBssDesc));
                            break;
                        }
                    }
                    scan_buf += MATH_ROUNDUP(len, 4);
                }
                /* If the target is not yet decided and there are unsearched channels, search them. */
                if((ret == FALSE)&&(MATH_GetLeastSignificantBit(scan_channelList) != 0))
                {
                    driver->scan_found_num = 0;
                    driver->scan_param->channelList = (u16)MATH_GetLeastSignificantBit(scan_channelList);
                    scan_channelList = (u16)(scan_channelList ^ MATH_GetLeastSignificantBit(scan_channelList));
                    wmResult = WM_StartScanEx(WXCi_ScanProc, driver->scan_param);
                    (void)WXCi_CheckWmApiResult(driver, WM_APIID_START_SCAN_EX, wmResult);
                    return;
                }
            }
            /* If there is a target, start connection */
            if (ret)
            {
                WXCi_StartChildProc(NULL);
            }
            /* If there is none, select the next startup mode */
            else
            {
                if (driver->target == WXC_DRIVER_STATE_CHILD)
                {
                    driver->target = WXC_DRIVER_STATE_IDLE;
                }
                WXCi_OnStateChanged(driver, WXC_DRIVER_STATE_IDLE, NULL);
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_InitDriver

  Description:  Initializes wireless and starts the transition to IDLE.

  Arguments:    driver: Used as an internal work memory buffer
                              Pointer to WXCDriverWork structure
                              This must be 32-byte aligned.
                pp: Parent parameters
                func: Event notification callback
                dma: DMA channel assigned to wireless

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InitDriver(WXCDriverWork * driver, WMParentParam *pp, WXCDriverEventFunc func, u32 dma)
{
    /* Gets the initial TGID outside of the interrupt handler */
    {
        OSIntrMode bak_cpsr = OS_EnableInterrupts();
        (void)WM_GetNextTgid();
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    /* Initialize work variables */
    work = driver;
    MI_CpuClear32(driver, sizeof(WXCDriverWork));
    driver->own_aid = 0;
    driver->send_busy = TRUE;
    driver->callback = func;
    driver->wm_dma = (u16)dma;
    driver->send_size_max = (u16)sizeof(driver->mp_send_work);
    driver->recv_size_max = (u16)sizeof(driver->mp_recv_work);
    driver->state = WXC_DRIVER_STATE_END;
    driver->parent_param = pp;
    driver->parent_param->entryFlag = 1;
    driver->parent_param->beaconPeriod = WXC_BEACON_PERIOD;
    driver->parent_param->channel = 1;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverTarget

  Description:  Starts transition from a specific state to a target state.

  Arguments:    driver: WXCDriverWork structure
                target: State of the transition target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetDriverTarget(WXCDriverWork * driver, WXCDriverState target)
{
    driver->target = target;
    /* If the system is stable in some other state, start transition here */
    if ((driver->state != WXC_DRIVER_STATE_BUSY) && (driver->state != driver->target))
    {
        WXCi_OnStateChanged(driver, driver->state, NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetDriverSsid

  Description:  Configures the SSID at connection.

  Arguments:    driver: WXCDriverWork structure
                buffer: SSID data to configure
                length: SSID data length
                              Must be no more than WM_SIZE_CHILD_SSID.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetDriverSsid(WXCDriverWork * driver, const void *buffer, u32 length)
{
    length = (u32)MATH_MIN(length, WM_SIZE_CHILD_SSID);
    MI_CpuCopy8(buffer, driver->ssid, length);
    MI_CpuFill8(driver->ssid + length, 0x00, (u32)(WM_SIZE_CHILD_SSID - length));
}


/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
