/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_system.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "nwm_arm9_private.h"
#include "nwm_common_private.h"

NWMArm9Buf *nwm9buf = NULL;
static PMSleepCallbackInfo sleepCbInfo;  //Sleep callback information to register with the PM library

#ifdef NWM_SUPPORT_HWRESET
PMExitCallbackInfo hwResetCbInfo;
#endif

static void NwmSleepCallback(void *);

/*---------------------------------------------------------------------------*
  Name:         NWMi_GetSystemWork

  Description:  Gets a pointer to the start of the buffer used internally by the new WM library.

  Arguments:    None.

  Returns:      NWMArm9Buf*: Returns a pointer to the internal work buffer.
 *---------------------------------------------------------------------------*/
NWMArm9Buf *NWMi_GetSystemWork(void)
{
//    SDK_NULL_ASSERT(nwm9buf);
    return nwm9buf;
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_ClearFifoRecvFlag

  Description:  Notifies the new WM7 that access to FIFO data used by a callback from the new WM7 is complete.
                
                When using a FIFO in a new WM7 callback, wait for this flag to unlock before editing the next callback.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NWMi_ClearFifoRecvFlag(void)
{
    NWMArm7Buf *w7b;

    if (nwm9buf == NULL) {
        return;
    }
    
    w7b = nwm9buf->NWM7;

    // The ARM7 will read the updated value, so invalidate the ARM9 cache
    DC_InvalidateRange(&w7b->callbackSyncFlag, 1);
    
    if (w7b->callbackSyncFlag & NWM_EXCEPTION_CB_MASK)
    {
        // Clears the CB exclusion flag
        w7b->callbackSyncFlag &= ~NWM_EXCEPTION_CB_MASK;
        // Store cache immediately
        DC_StoreRange(&w7b->callbackSyncFlag, 1);
    }
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_ReceiveFifo9

  Description:  Receives a callback from the new WM7 through a FIFO.

  Arguments:    tag: Unused
                fifo_buf_adr: Pointer to the callback parameter group
                err: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/

void NWMi_ReceiveFifo9(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err)
{
    #pragma unused( tag )
    NWMCallback *pCallback = (NWMCallback *)fifo_buf_adr;
    NWMCallbackFunc callback;
    NWMArm9Buf *w9b = nwm9buf;

    if (w9b == NULL) {
        return;
    }
    
    if (err) {
        NWM_WARNING("NWM9 FIFO receive error. :%d\n", err);
        return;
    }

    if (!fifo_buf_adr) {
        NWM_WARNING("NWM9 FIFO receive error.(NULL address) :%d\n", err);
        return;
    }
    
    // When 'apiid' is an unexpected value (NWM_APIID_ASYNC_KIND_MAX or more)
    if(pCallback->apiid >= NWM_APIID_ASYNC_KIND_MAX)
    {
        NWM_WARNING("Receive Unknown APIID(%d)\n",pCallback->apiid);
        return;
    }
    
    DC_InvalidateRange(w9b->fifo7to9, NWM_APIFIFO_BUF_SIZE);
    DC_InvalidateRange(w9b->status, NWM_STATUS_BUF_SIZE);
    if ((u32)pCallback != (u32)(w9b->fifo7to9))
    {
        DC_InvalidateRange(pCallback, NWM_APIFIFO_BUF_SIZE);
    }

    NWM_DPRINTF("APIID%04x\n", pCallback->apiid);

    // Remove the sleep callback if NWM_LoadDevice failed or WM_UnloadDevice succeeded.
    if( (pCallback->apiid == NWM_APIID_LOAD_DEVICE   && pCallback->retcode != NWM_RETCODE_SUCCESS) ||
        (pCallback->apiid == NWM_APIID_UNLOAD_DEVICE && pCallback->retcode == NWM_RETCODE_SUCCESS) ||
         pCallback->apiid == NWM_APIID_INSTALL_FIRMWARE )
    {
        NWMi_DeleteSleepCallback();
    }

    // Callback processing according to apiid (does nothing if callback not set (NULL))
    {
        NWMSendFrameCallback *pSfcb = (NWMSendFrameCallback *)pCallback;
        NWMRetCode result = NWM_RETCODE_FAILED;
        
        if (pCallback->apiid == NWM_APIID_SEND_FRAME)
        {
            if (pCallback->retcode != NWM_RETCODE_INDICATION && NULL != pSfcb->callback)
            {
                NWM_DPRINTF("Execute CallbackFunc APIID 0x%04x\n", pCallback->apiid);
                (pSfcb->callback)((void *)pCallback);
            }

        }
    }

    // In the case of several special APIs
    if (pCallback->apiid == NWM_APIID_SEND_FRAME) {
        NWMSendFrameCallback *pSfcb = (NWMSendFrameCallback *)pCallback;

    } else {

#ifdef NWM_SUPPORT_HWRESET
        if (pCallback->apiid == NWM_APIID_UNLOAD_DEVICE
            || pCallback->apiid == NWM_APIID_INSTALL_FIRMWARE
            || (pCallback->apiid == NWM_APIID_LOAD_DEVICE
                && pCallback->retcode != NWM_RETCODE_SUCCESS)) {
            // Delete HW reset callback
            PM_DeletePostExitCallback(&hwResetCbInfo);
        }
#endif
        
        // Other APIs
        callback = w9b->callbackTable[pCallback->apiid];

        // In case START_SCAN callback, scan buffer cache must be invalidated.
        if (pCallback->apiid == NWM_APIID_START_SCAN) {
            NWMStartScanCallback *psscb = (NWMStartScanCallback *)pCallback;

            DC_InvalidateRange(psscb->bssDesc[0], psscb->allBssDescSize);
        }

        if (NULL != callback)
        {
            NWM_DPRINTF("Execute CallbackFunc APIID 0x%04x\n", pCallback->apiid);
            (callback) ((void *)pCallback);
        }

    }

    MI_CpuClear8(pCallback, NWM_APIFIFO_BUF_SIZE);
    DC_StoreRange(pCallback, NWM_APIFIFO_BUF_SIZE);
    if (w9b) { // NWM might be terminated after callback
        NWMi_ClearFifoRecvFlag();
    }

}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetState

  Description:  Checks the internal state of the NWM library.
                The purpose for using this is similar to NWMi_CheckState, but its application is largely different so it is provided separately.
                

  Arguments:    None.

  Returns:      u16: Integer that indicates the internal NWM state
 *---------------------------------------------------------------------------*/
u16 NWM_GetState(void)
{
    NWMStatus*  nwmStatus;
    NWMArm9Buf* sys = NWMi_GetSystemWork();
    u16 state = NWM_STATE_NONE;

    if (sys) {
        nwmStatus = sys->status;
        DC_InvalidateRange(nwmStatus, 2);
        state = nwmStatus->state;
    }

    return state;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_CheckState

  Description:  Checks the internal state of the NWM library.
                Specifies the WMState type parameters showing the permitted state by enumerating them.

  Arguments:    paramNum: Number of virtual arguments
                ...: Virtual argument

  Returns:      int: Returns the result of the process as an NWM_RETCODE_* value.
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_CheckState(s32 paramNum, ...)
{
    NWMRetCode result;
    u16     now;
    u32     temp;
    va_list vlist;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    // Check if initialized
    result = NWMi_CheckInitialized();
    NWM_CHECK_RESULT(result);

    // Gets the current state
    DC_InvalidateRange(&(sys->status->state), 2);
    now = sys->status->state;

    // Match confirmation
    result = NWM_RETCODE_ILLEGAL_STATE;
    va_start(vlist, paramNum);
    for (; paramNum; paramNum--)
    {
        temp = va_arg(vlist, u32);
        if (temp == now)
        {
            result = NWM_RETCODE_SUCCESS;
        }
    }
    va_end(vlist);

    if (result == NWM_RETCODE_ILLEGAL_STATE)
    {
        NWM_WARNING("New WM state is \"0x%04x\" now. So can't execute request.\n", now);
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetAllowedChannel

  Description:  Gets the IEEE 802.11b/g channel list that can be used with the new wireless functions.

  Arguments:    None.

  Returns:      u16: Returns a channel bitmap in the same format as the WM library.
 *---------------------------------------------------------------------------*/
u16 NWM_GetAllowedChannel(void)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    return sys->status->allowedChannel;
}

/*---------------------------------------------------------------------------*
  Name:         NWM_CalcLinkLevel

  Description:  Calculates the link level from the threshold value defined in nwm_common_private.h.

  Arguments:    s16: RSSI value sent with a notification from the Atheros driver

  Returns:      u16: The same link level as the WM library.
 *---------------------------------------------------------------------------*/
u16 NWM_CalcLinkLevel(s16 rssi)
{

/* [TODO] Does this need to be switched depending on the operating mode? If this is necessary, the conditional statement must be revised. */

    if(1) /* Infra Structure Mode */
    {
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_1)
        {
            return WM_LINK_LEVEL_0;
        }
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_2)
        {
            return WM_LINK_LEVEL_1;
        }
        if(rssi < NWM_RSSI_INFRA_LINK_LEVEL_3)
        {
            return WM_LINK_LEVEL_2;
        }

        return WM_LINK_LEVEL_3;
    }
    else if(0) /*Ad Hoc Mode*/
    {
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_1)
        {
            return WM_LINK_LEVEL_0;
        }
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_2)
        {
            return WM_LINK_LEVEL_1;
        }
        if(rssi < NWM_RSSI_ADHOC_LINK_LEVEL_3)
        {
            return WM_LINK_LEVEL_2;
        }
        return WM_LINK_LEVEL_3;
    }

}

/*---------------------------------------------------------------------------*
  Name:         NWM_GetDispersionScanPeriod

  Description:  Gets the time limit that should be set on searching for an AP or DS parent device as an STA.

  Arguments:    u16 scanType: Scan type, either NWM_SCANTYPE_PASSIVE or NWM_SCANTYPE_ACTIVE
  
  Returns:      u16: Search limit time that should be set (ms).
 *---------------------------------------------------------------------------*/
u16 NWM_GetDispersionScanPeriod(u16 scanType)
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
    
    if( scanType == NWM_SCANTYPE_ACTIVE )
    {
        ret = (u16)(NWM_DEFAULT_ACTIVE_SCAN_PERIOD + (ret % 10));
    }
    else /* An unknown scan type is treated in the same way as a passive scan */
    {
        ret = (u16)(NWM_DEFAULT_PASSIVE_SCAN_PERIOD + (ret % 10));
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_RegisterSleepCallback

  Description:  Registers the callback function that will be run when shifting to Sleep Mode.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_RegisterSleepCallback(void)
{
    PM_SetSleepCallbackInfo(&sleepCbInfo, NwmSleepCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCbInfo, PM_CALLBACK_PRIORITY_NWM );
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_DeleteSleepCallback

  Description:  Deletes the callback function that is run when shifting to Sleep Mode.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_DeleteSleepCallback(void)
{
    PM_DeletePreSleepCallback( &sleepCbInfo );
}

/*---------------------------------------------------------------------------*
  Name:         NwmSleepCallback

  Description:  Prevents the program from entering Sleep Mode during wireless communications.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void NwmSleepCallback(void *)
{
    /* ---------------------------------------------- *
     * As described in the Programming Guidelines, it is prohibited to run the OS_GoSleepMode function during wireless communications. *
     *                                                *
     *                                                *
     * ---------------------------------------------- */
    OS_TPanic("Could not sleep during wireless communications.");
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
