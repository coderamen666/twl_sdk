/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_system.c

  Copyright 2003-2009 Nintendo. All rights reserved.

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

#ifdef SDK_TWL /* Strengthens checks during TWL initialization */
#include "wm_common.h"
#endif //SDK_TWL

/*---------------------------------------------------------------------------*
    Constant Definitions
 *---------------------------------------------------------------------------*/
#define     WM_BUF_MSG_NUM                  10


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u16 wmInitialized = 0;          // WM initialization flag
static WMArm9Buf *wm9buf;
static u32 fifoBuf[WM_BUF_MSG_NUM][WM_FIFO_BUF_SIZE / sizeof(u32)] ATTRIBUTE_ALIGN(32);
static OSMessageQueue bufMsgQ;         // Request queue for WM7
static OSMessage bufMsg[WM_BUF_MSG_NUM];        // Request queue buffer for WM7
static PMSleepCallbackInfo sleepCbInfo;  //Sleep callback information to register with the PM library


/*---------------------------------------------------------------------------*
    Internal Function Definitions
 *---------------------------------------------------------------------------*/
static void WmReceiveFifo(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err);
static void WmClearFifoRecvFlag(void);
static WMErrCode WmInitCore(void *wmSysBuf, u16 dmaNo, u32 bufSize);
static u32 *WmGetCommandBuffer4Arm7(void);
static void WmSleepCallback(void *);

/*---------------------------------------------------------------------------*
  Name:         WM_Init

  Description:  Performs the initialization process for the WM library.
                Synchronous function that only initializes ARM9.

  Arguments:    wmSysBuf: Pointer to the buffer allocated by the caller.
                          The required buffer size is only WM_SYSTEM_BUF_SIZE.
                dmaNo: DMA number used by WM

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_Init(void *wmSysBuf, u16 dmaNo)
{
    WMErrCode result;

    result = WmInitCore(wmSysBuf, dmaNo, WM_SYSTEM_BUF_SIZE);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }
    wm9buf->scanOnlyFlag = FALSE;
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_InitForScan

  Description:  Performs initialization processing for WM library when using only scan.
                Synchronous function that only initializes ARM9.

  Arguments:    wmSysBuf: Pointer to the buffer allocated by the caller.
                          The required buffer size is only WM_SYSTEM_BUF_SIZE_FOR_SCAN.
                dmaNo: DMA number used by WM

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_InitForScan(void *wmSysBuf, u16 dmaNo);
WMErrCode WMi_InitForScan(void *wmSysBuf, u16 dmaNo)
{
#define WM_STATUS_BUF_SIZE_FOR_SCAN      768
#define WM_ARM9WM_BUF_SIZE_FOR_SCAN      320
#define WM_SYSTEM_BUF_SIZE_FOR_SCAN      (WM_ARM9WM_BUF_SIZE_FOR_SCAN + WM_ARM7WM_BUF_SIZE + WM_STATUS_BUF_SIZE_FOR_SCAN + WM_FIFO_BUF_SIZE + WM_FIFO_BUF_SIZE)

    WMErrCode result;
    result = WmInitCore(wmSysBuf, dmaNo, WM_SYSTEM_BUF_SIZE_FOR_SCAN);
    if (result != WM_ERRCODE_SUCCESS)
    {
        return result;
    }
    wm9buf = (WMArm9Buf *)wmSysBuf;
    wm9buf->WM7 = (WMArm7Buf *)((u32)wm9buf + WM_ARM9WM_BUF_SIZE_FOR_SCAN);
    wm9buf->status = (WMStatus *)((u32)(wm9buf->WM7) + WM_ARM7WM_BUF_SIZE);
    wm9buf->fifo9to7 = (u32 *)((u32)(wm9buf->status) + WM_STATUS_BUF_SIZE_FOR_SCAN);
    wm9buf->fifo7to9 = (u32 *)((u32)(wm9buf->fifo9to7) + WM_FIFO_BUF_SIZE);
    wm9buf->dmaNo = dmaNo;
    wm9buf->scanOnlyFlag = TRUE;

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WmInitCore

  Description:  Library internal use function that performs WM library initialization process.
                Synchronous function that only initializes ARM9.

  Arguments:    wmSysBuf: Pointer to the buffer allocated by the caller
                dmaNo: DMA number used by WM
                bufSize: Buffer size allocated to WM library

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
static WMErrCode WmInitCore(void *wmSysBuf, u16 dmaNo, u32 bufSize)
{
    OSIntrMode e;

    // Checks if the structure size exceeds the expected value
    SDK_COMPILER_ASSERT(sizeof(WMArm9Buf) <= WM_ARM9WM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(WMArm7Buf) <= WM_ARM7WM_BUF_SIZE);
    SDK_COMPILER_ASSERT(sizeof(WMStatus) <= WM_STATUS_BUF_SIZE);

    e = OS_DisableInterrupts();

#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        /* Check that the NWM library has not been initialized */
        if (NWM_CheckInitialized() != NWM_RETCODE_ILLEGAL_STATE)
        {
            WM_WARNING("NWM has already been initialized.\n");
            (void)OS_RestoreInterrupts(e);
            return WM_ERRCODE_ILLEGAL_STATE;
        }
    }
#endif

    /* Confirms that the WM library initialization is complete. */
    if (wmInitialized)
    {
        WM_WARNING("WM has already been initialized.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_ILLEGAL_STATE;        // Initialization complete
    }

    // Check parameters
    if (wmSysBuf == NULL)
    {
        WM_WARNING("Parameter \"wmSysBuf\" must not be NULL.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
        // Because of WM PXI usage, wmSysBuf cannot be placed at an address of 0x04000000 or higher
#ifdef SDK_TWL
    if( OS_IsOnVram(wmSysBuf) || ( OS_IsRunOnTwl() && OS_IsOnExtendedMainMemory(wmSysBuf) ) )
#else
    if( OS_IsOnVram(wmSysBuf) )
#endif
    {
        WM_WARNING("Parameter \"wmSysBuf\" must be alocated less than 0x04000000.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
    
    if (dmaNo > MI_DMA_MAX_NUM)
    {
        WM_WARNING("Parameter \"dmaNo\" is over %d.\n", MI_DMA_MAX_NUM);
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)wmSysBuf & 0x01f)
    {
        WM_WARNING("Parameter \"wmSysBuf\" must be 32-byte aligned.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_INVALID_PARAM;
    }

    // Confirms whether the WM library has started on the ARM7
    PXI_Init();
    if (!PXI_IsCallbackReady(PXI_FIFO_TAG_WM, PXI_PROC_ARM7))
    {
        WM_WARNING("WM library on ARM7 side is not ready yet.\n");
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_WM_DISABLE;
    }

    /* If there is excess data remaining in the cache, a write-back could occur unexpectedly and interfere with state management on the ARM7.
       
       As a countermeasure, delete the cache of all work buffers at this point. */
    DC_InvalidateRange(wmSysBuf, bufSize);
    // Initialization for various types of buffers
    MI_DmaClear32(dmaNo, wmSysBuf, bufSize);
    wm9buf = (WMArm9Buf *)wmSysBuf;
    wm9buf->WM7 = (WMArm7Buf *)((u32)wm9buf + WM_ARM9WM_BUF_SIZE);
    wm9buf->status = (WMStatus *)((u32)(wm9buf->WM7) + WM_ARM7WM_BUF_SIZE);
    wm9buf->fifo9to7 = (u32 *)((u32)(wm9buf->status) + WM_STATUS_BUF_SIZE);
    wm9buf->fifo7to9 = (u32 *)((u32)(wm9buf->fifo9to7) + WM_FIFO_BUF_SIZE);

    // Clears the FIFO buffer writable flag
    WmClearFifoRecvFlag();

    // Initializes the various types of variables
    wm9buf->dmaNo = dmaNo;
    wm9buf->connectedAidBitmap = 0x0000;
    wm9buf->myAid = 0;

    // Initializes the callback table for each port
    {
        s32     i;

        for (i = 0; i < WM_NUM_OF_PORT; i++)
        {
            wm9buf->portCallbackTable[i] = NULL;
            wm9buf->portCallbackArgument[i] = NULL;
        }
    }

    // Initializes the queue for entry registration
    {
        s32     i;

        OS_InitMessageQueue(&bufMsgQ, bufMsg, WM_BUF_MSG_NUM);
        for (i = 0; i < WM_BUF_MSG_NUM; i++)
        {
            // Clears the ring buffer to the usable state
            *((u16 *)(fifoBuf[i])) = WM_API_REQUEST_ACCEPTED;
            DC_StoreRange(fifoBuf[i], 2);
            (void)OS_SendMessage(&bufMsgQ, fifoBuf[i], OS_MESSAGE_BLOCK);
        }
    }

    // Sets FIFO receive function
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WM, WmReceiveFifo);

    wmInitialized = 1;
    (void)OS_RestoreInterrupts(e);
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WM_Finish

  Description:  Performs termination processing for WM library. Synchronous function.
                Restores the state before WM_Init function was called.

  Arguments:    None.

  Returns:      WMErrCode: Returns the processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WM_Finish(void)
{
    OSIntrMode e;
    WMErrCode result;

    e = OS_DisableInterrupts();
    // Cannot execute if initialization is incomplete
    result = WMi_CheckInitialized();
    if (result != WM_ERRCODE_SUCCESS)
    {
        (void)OS_RestoreInterrupts(e);
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    // Cannot execute if the state is not READY
    result = WMi_CheckState(WM_STATE_READY);
    WM_CHECK_RESULT(result);

    // Close WM library
    WmClearFifoRecvFlag();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WM, NULL);
    wm9buf = 0;

    wmInitialized = 0;
    (void)OS_RestoreInterrupts(e);
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SetCallbackTable

  Description:  Registers the callback function for each asynchronous function.

  Arguments:    id: Asynchronous function's API ID
                callback: Callback function to be registered

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_SetCallbackTable(WMApiid id, WMCallbackFunc callback)
{
    SDK_NULL_ASSERT(wm9buf);

    wm9buf->CallbackTable[id] = callback;
}

/*---------------------------------------------------------------------------*
  Name:         WmGetCommandBuffer4Arm7

  Description:  Reserves from the pool a buffer for commands directed to ARM7.

  Arguments:    None.

  Returns:      If it can be reserved, it is that value; otherwise, NULL.
 *---------------------------------------------------------------------------*/
u32    *WmGetCommandBuffer4Arm7(void)
{
    u32    *tmpAddr;

    if (FALSE == OS_ReceiveMessage(&bufMsgQ, (OSMessage *)&tmpAddr, OS_MESSAGE_NOBLOCK))
    {
        return NULL;
    }

    // Check if the ring buffer is available (queue is not full)
    DC_InvalidateRange(tmpAddr, 2);
    if ((*((u16 *)tmpAddr) & WM_API_REQUEST_ACCEPTED) == 0)
    {
        // Reserve again at the beginning of the queue (because this should be first to come available)
        (void)OS_JamMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);
        // End with an error
        return NULL;
    }

    return tmpAddr;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SendCommand

  Description:  Transmits a request to the ARM7 via the FIFO.
                For commands accompanied by some number of u32-type parameters, specify the parameters by enumerating them.
                

  Arguments:    id: API ID that corresponds to the request
                paramNum: Number of virtual arguments
                ...: Virtual argument

  Returns:      int: Returns a WM_ERRCODE_* type processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SendCommand(WMApiid id, u16 paramNum, ...)
{
    va_list vlist;
    s32     i;
    int     result;
    u32    *tmpAddr;

    SDK_NULL_ASSERT(wm9buf);

    // Reserves a buffer for command sending
    tmpAddr = WmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        return WM_ERRCODE_FIFO_ERROR;
    }

    // API ID
    *(u16 *)tmpAddr = (u16)id;

    // Adds the specified number of arguments
    va_start(vlist, paramNum);
    for (i = 0; i < paramNum; i++)
    {
        tmpAddr[i + 1] = va_arg(vlist, u32);
    }
    va_end(vlist);

    DC_StoreRange(tmpAddr, WM_FIFO_BUF_SIZE);

    // Notification with FIFO
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WM, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        WM_WARNING("Failed to send command through FIFO.\n");
        return WM_ERRCODE_FIFO_ERROR;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_SendCommandDirect

  Description:  Transmits a request to the ARM7 via the FIFO.
                Directly specifies the command sent to the ARM7.

  Arguments:    data: Command sent to the ARM7
                length: Size of the command sent to the ARM7

  Returns:      int: Returns a WM_ERRCODE_* type processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_SendCommandDirect(void *data, u32 length)
{
    int     result;
    u32    *tmpAddr;

    SDK_NULL_ASSERT(wm9buf);
    SDK_ASSERT(length <= WM_FIFO_BUF_SIZE);

    // Reserves a buffer for command sending
    tmpAddr = WmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        return WM_ERRCODE_FIFO_ERROR;
    }

    // Copies to a buffer specifically for commands sent to the ARM7
    MI_CpuCopy8(data, tmpAddr, length);

    DC_StoreRange(tmpAddr, length);

    // Notification with FIFO
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WM, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&bufMsgQ, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        WM_WARNING("Failed to send command through FIFO.\n");
        return WM_ERRCODE_FIFO_ERROR;
    }

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetSystemWork

  Description:  Gets the pointer to the front of the buffer used internally by the WM library.

  Arguments:    None.

  Returns:      WMArm9Buf*: Returns a pointer to the internal work buffer.
 *---------------------------------------------------------------------------*/
WMArm9Buf *WMi_GetSystemWork(void)
{
//    SDK_NULL_ASSERT(wm9buf);
    return wm9buf;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckInitialized

  Description:  Confirms that the WM library initialization is complete.

  Arguments:    None.

  Returns:      int: Returns a WM_ERRCODE_* type processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckInitialized(void)
{
    // Check if initialized
    if (!wmInitialized)
    {
        return WM_ERRCODE_ILLEGAL_STATE;
    }
    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckIdle

  Description:  Confirms the internal state of the WM library, and confirms that the wireless hardware has already started.
                

  Arguments:    None.

  Returns:      int: Returns a WM_ERRCODE_* type processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckIdle(void)
{
    WMErrCode result;

    // Check if initialized
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // Confirms the current state
    DC_InvalidateRange(&(wm9buf->status->state), 2);
    if ((wm9buf->status->state == WM_STATE_READY) || (wm9buf->status->state == WM_STATE_STOP))
    {
        WM_WARNING("WM state is \"%d\" now. So can't execute request.\n", wm9buf->status->state);
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    return WM_ERRCODE_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckStateEx

  Description:  Confirms the internal state of the WM library.
                Specifies the WMState type parameters showing the permitted state by enumerating them.

  Arguments:    paramNum: Number of virtual arguments
                ...: Virtual argument

  Returns:      int: Returns a WM_ERRCODE_* type processing result.
 *---------------------------------------------------------------------------*/
WMErrCode WMi_CheckStateEx(s32 paramNum, ...)
{
    WMErrCode result;
    u16     now;
    u32     temp;
    va_list vlist;

    // Check if initialized
    result = WMi_CheckInitialized();
    WM_CHECK_RESULT(result);

    // Gets the current state
    DC_InvalidateRange(&(wm9buf->status->state), 2);
    now = wm9buf->status->state;

    // Match confirmation
    result = WM_ERRCODE_ILLEGAL_STATE;
    va_start(vlist, paramNum);
    for (; paramNum; paramNum--)
    {
        temp = va_arg(vlist, u32);
        if (temp == now)
        {
            result = WM_ERRCODE_SUCCESS;
        }
    }
    va_end(vlist);

    if (result == WM_ERRCODE_ILLEGAL_STATE)
    {
        WM_WARNING("WM state is \"%d\" now. So can't execute request.\n", now);
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WmReceiveFifo

  Description:  Receives a callback from WM7 via FIFO.

  Arguments:    tag: Unused
                fifo_buf_adr: Pointer to the callback parameter group
                err: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmReceiveFifo(PXIFifoTag tag, u32 fifo_buf_adr, BOOL err)
{
#pragma unused( tag )

    WMCallback *callback_p = (WMCallback *)fifo_buf_adr;
    WMArm9Buf *w9b = wm9buf;           // Notify compiler that it is not volatile
#ifdef WM_DEBUG_CALLBACK
    int     beginVcount = GX_GetVCount();
#endif

    if (err)
        return;                        // The WM_sp_init PXI handler is not ready

    // Invalidates the FIFO command buffer (9 <- 7) cache
    DC_InvalidateRange(w9b->fifo7to9, WM_FIFO_BUF_SIZE);
    if (!w9b->scanOnlyFlag)
    {
        DC_InvalidateRange(w9b->status, WM_STATUS_BUF_SIZE);
    }

    // Checks if the response buffer is a different buffer from fifo7to9
    if ((u32)callback_p != (u32)(w9b->fifo7to9))
    {
        DC_InvalidateRange(callback_p, WM_FIFO_BUF_SIZE);
    }


    if (callback_p->apiid >= WM_NUM_OF_CALLBACK)
    {
        if (callback_p->apiid == WM_APIID_INDICATION)
        {
            if (callback_p->errcode == WM_ERRCODE_FLASH_ERROR)
            {
                // ARM9 freeze
                OS_Terminate();        // It does not return
            }
            if (w9b->indCallback != NULL)
            {
                w9b->indCallback((void *)callback_p);
            }
        }
        else if (callback_p->apiid == WM_APIID_PORT_RECV)
        {
            // Performs callback processing according to the port number
            WMPortRecvCallback *cb_Port = (WMPortRecvCallback *)callback_p;
            SDK_ASSERT(cb_Port->port < 16);
            // Notify if a callback has been registered
            if (w9b->portCallbackTable[cb_Port->port] != NULL)
            {
                cb_Port->arg = w9b->portCallbackArgument[cb_Port->port];
                cb_Port->connectedAidBitmap = (u16)w9b->connectedAidBitmap;
                DC_InvalidateRange(cb_Port->recvBuf, w9b->status->mp_recvBufSize);      // TODO: Too much is being invalidated here
                (w9b->portCallbackTable[cb_Port->port]) ((void *)cb_Port);
            }
#ifdef WM_DEBUG
            else
            {
                WM_DPRINTF("Warning: no callback function on port %d { %04x %04x ... }\n",
                           cb_Port->port, cb_Port->data[0], cb_Port->data[1]);
            }
#endif
        }
        else if (callback_p->apiid == WM_APIID_PORT_SEND)
        {
            // Performs the callback specified when the data is set
            WMPortSendCallback *cb_Port = (WMPortSendCallback *)callback_p;
            callback_p->apiid = WM_APIID_SET_MP_DATA;   // Camouflages
            if (cb_Port->callback != NULL)
            {
                (cb_Port->callback) ((void *)cb_Port);
            }
        }
        else
        {
            OS_TPrintf("ARM9: no callback function\n");

        }
    }
    else
    {
        // Invalidates the cache of the receive buffer
        // When PORT_RECV is called after MPEND_IND and MP_IND, InvalidateRange has already been done, so there is no need to be concerned with the cache again.
        // 
        if (callback_p->apiid == WM_APIID_START_MP)
        {
            WMStartMPCallback *cb_StartMP = (WMStartMPCallback *)callback_p;
            if (cb_StartMP->state == WM_STATECODE_MPEND_IND
                || cb_StartMP->state == WM_STATECODE_MP_IND)
            {
                if (cb_StartMP->errcode == WM_ERRCODE_SUCCESS)
                {
                    DC_InvalidateRange(cb_StartMP->recvBuf, w9b->status->mp_recvBufSize);
                }
            }
        }

        // Remove the sleep callback if WM_Enable failed or WM_Disable succeeded
        if( ( (callback_p->apiid == WM_APIID_DISABLE  || callback_p->apiid == WM_APIID_END)      && callback_p->errcode == WM_ERRCODE_SUCCESS) ||
            ( (callback_p->apiid == WM_APIID_ENABLE || callback_p->apiid == WM_APIID_INITIALIZE) && callback_p->errcode != WM_ERRCODE_SUCCESS) )
        {
            WMi_DeleteSleepCallback();
        }
        
        // WM_End post-processing
        if (callback_p->apiid == WM_APIID_END)
        {
            if (callback_p->errcode == WM_ERRCODE_SUCCESS)
            {
                WMCallbackFunc cb = w9b->CallbackTable[callback_p->apiid];
                // Close WM library
                (void)WM_Finish();
                if (cb != NULL)
                {
                    cb((void *)callback_p);
                }

                WM_DLOGF_CALLBACK("Cb[%x]", callback_p->apiid);

                return;
            }
        }

#if 0
        // Post-processing for WM_Reset, WM_Disconnect, and WM_EndParent
        if (callback_p->apiid == WM_APIID_RESET
            || callback_p->apiid == WM_APIID_DISCONNECT || callback_p->apiid == WM_APIID_END_PARENT)
        {
            if (w9b->connectedAidBitmap != 0)
            {
                WM_WARNING("connectedAidBitmap should be 0, but %04x", w9b->connectedAidBitmap);
            }
            w9b->myAid = 0;
            w9b->connectedAidBitmap = 0;
        }
#endif

        // Callback processing according to apiid (does nothing if callback not set (NULL))
        if (NULL != w9b->CallbackTable[callback_p->apiid])
        {
            (w9b->CallbackTable[callback_p->apiid]) ((void *)callback_p);
            /* If WM_Finish was called in user callback and working memory was cleared, end here */
            if (!wmInitialized)
            {
                return;
            }
        }
        // Post-processing
        // Process the messages so that the callback for the port can also be notified regarding connections/disconnections.
        if (callback_p->apiid == WM_APIID_START_PARENT
            || callback_p->apiid == WM_APIID_START_CONNECT)
        {
            u16     state, aid, myAid, reason;
            u8     *macAddress;
            u8     *ssid;
            u16     parentSize, childSize;

            if (callback_p->apiid == WM_APIID_START_PARENT)
            {
                WMStartParentCallback *cb = (WMStartParentCallback *)callback_p;
                state = cb->state;
                aid = cb->aid;
                myAid = 0;
                macAddress = cb->macAddress;
                ssid = cb->ssid;
                reason = cb->reason;
                parentSize = cb->parentSize;
                childSize = cb->childSize;
            }
            else if (callback_p->apiid == WM_APIID_START_CONNECT)
            {
                WMStartConnectCallback *cb = (WMStartConnectCallback *)callback_p;
                state = cb->state;
                aid = 0;
                myAid = cb->aid;
                macAddress = cb->macAddress;
                ssid = NULL;
                reason = cb->reason;
                parentSize = cb->parentSize;
                childSize = cb->childSize;
            }
            if (state == WM_STATECODE_CONNECTED ||
                state == WM_STATECODE_DISCONNECTED
                || state == WM_STATECODE_DISCONNECTED_FROM_MYSELF)
            {
                // Because this is inside the interrupt handler, there's no problem if it is static
                static WMPortRecvCallback cb_Port;
                u16     iPort;

                // Change the connection status being managed on the WM9 side
                if (state == WM_STATECODE_CONNECTED)
                {
#ifdef WM_DEBUG
                    if (w9b->connectedAidBitmap & (1 << aid))
                    {
                        WM_DPRINTF("Warning: someone is connecting to connected aid: %d (%04x)",
                                   aid, w9b->connectedAidBitmap);
                    }
#endif
                    WM_DLOGF_AIDBITMAP("aid(%d) connected: %04x -> %04x", aid,
                                       w9b->connectedAidBitmap,
                                       w9b->connectedAidBitmap | (1 << aid));
                    w9b->connectedAidBitmap |= (1 << aid);
                }
                else                   // WM_STATECODE_DISCONNECTED || WM_STATECODE_DISCONNECTED_FROM_MYSELF
                {
#ifdef WM_DEBUG
                    if (!(w9b->connectedAidBitmap & (1 << aid)))
                    {
                        WM_DPRINTF
                            ("Warning: someone is disconnecting to disconnected aid: %d (%04x)",
                             aid, w9b->connectedAidBitmap);
                    }
#endif
                    WM_DLOGF_AIDBITMAP("aid(%d) disconnected: %04x -> %04x", aid,
                                       w9b->connectedAidBitmap,
                                       w9b->connectedAidBitmap & ~(1 << aid));
                    w9b->connectedAidBitmap &= ~(1 << aid);
                }
                w9b->myAid = myAid;

                MI_CpuClear8(&cb_Port, sizeof(WMPortRecvCallback));
                cb_Port.apiid = WM_APIID_PORT_RECV;
                cb_Port.errcode = WM_ERRCODE_SUCCESS;
                cb_Port.state = state;
                cb_Port.recvBuf = NULL;
                cb_Port.data = NULL;
                cb_Port.length = 0;
                cb_Port.aid = aid;
                cb_Port.myAid = myAid;
                cb_Port.connectedAidBitmap = (u16)w9b->connectedAidBitmap;
                cb_Port.seqNo = 0xffff;
                cb_Port.reason = reason;
                MI_CpuCopy8(macAddress, cb_Port.macAddress, WM_SIZE_MACADDR);
                if (ssid != NULL)
                {
                    MI_CpuCopy16(ssid, cb_Port.ssid, WM_SIZE_CHILD_SSID);
                }
                else
                {
                    MI_CpuClear16(cb_Port.ssid, WM_SIZE_CHILD_SSID);
                }
                cb_Port.maxSendDataSize = (u16)((myAid == 0) ? parentSize : childSize);
                cb_Port.maxRecvDataSize = (u16)((myAid == 0) ? childSize : parentSize);

                // Notify all ports of connections/disconnections
                for (iPort = 0; iPort < WM_NUM_OF_PORT; iPort++)
                {
                    cb_Port.port = iPort;
                    if (w9b->portCallbackTable[iPort] != NULL)
                    {
                        cb_Port.arg = w9b->portCallbackArgument[iPort];
                        (w9b->portCallbackTable[iPort]) ((void *)&cb_Port);
                    }
                }
            }
        }
    }
    // The fifo7to9 region has been written with PORT_SEND and PORT_RECV, so the cache is invalidated so as to not be written back
    // 
    DC_InvalidateRange(w9b->fifo7to9, WM_FIFO_BUF_SIZE);
    WmClearFifoRecvFlag();

    // If the response buffer is a different buffer from fifo7to9, camouflage the request reception completion flag
    if ((u32)callback_p != (u32)(w9b->fifo7to9))
    {
        callback_p->apiid |= WM_API_REQUEST_ACCEPTED;
        DC_StoreRange(callback_p, WM_FIFO_BUF_SIZE);
    }

    WM_DLOGF2_CALLBACK(beginVcount, "[CB](%x)", callback_p->apiid);

    return;
}

/*---------------------------------------------------------------------------*
  Name:         WmClearFifoRecvFlag

  Description:  Notifies WM7 that reference to the FIFO data used in the WM7 callback is complete.
                
                When using FIFO for callback in WM7, edit the next callback after waiting for this flag to unlock.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmClearFifoRecvFlag(void)
{
    if (OS_GetSystemWork()->wm_callback_control & WM_EXCEPTION_CB_MASK)
    {
        // Clears the CB exclusion flag
        OS_GetSystemWork()->wm_callback_control &= ~WM_EXCEPTION_CB_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WMi_DebugPrintSendQueue

  Description:  Print outputs the content of the port send queue.

  Arguments:    queue: Pointer to the port send queue

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DebugPrintSendQueue(WMPortSendQueue *queue)
{
    WMstatus *status = wm9buf->status;
    WMPortSendQueueData *queueData;
    u16     index;

    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // Invalidates the ARM7 status region cache
    queueData = status->sendQueueData;

    OS_TPrintf("head = %d, tail = %d, ", queue->head, queue->tail);
    if (queue->tail != WM_SEND_QUEUE_END)
    {
        OS_TPrintf("%s", (queueData[queue->tail].next == WM_SEND_QUEUE_END) ? "valid" : "invalid");
    }
    OS_TPrintf("\n");
    for (index = queue->head; index != WM_SEND_QUEUE_END; index = queueData[index].next)
    {
        WMPortSendQueueData *data = &(queueData[index]);

        OS_TPrintf("queueData[%d] -> %d { port=%d, destBitmap=%x, size=%d } \n", index, data->next,
                  data->port, data->destBitmap, data->size);
    }

}

/*---------------------------------------------------------------------------*
  Name:         WMi_DebugPrintAllSendQueue

  Description:  Print outputs the content of all port send queues.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DebugPrintAllSendQueue(void)
{
    WMstatus *status = wm9buf->status;
#if 0
    int     iPrio;

    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // Invalidates the ARM7 status region cache
    for (iPrio = 0; iPrio < WM_PRIORITY_LEVEL; iPrio++)
    {
        OS_TPrintf("== send queue [%d]\n", iPrio);
        WMi_DebugPrintSendQueue(&status->sendQueue[iPrio]);
    }
    for (iPrio = 0; iPrio < WM_PRIORITY_LEVEL; iPrio++)
    {
        OS_TPrintf("== ready queue [%d]\n", iPrio);
        WMi_DebugPrintSendQueue(&status->readyQueue[iPrio]);
    }
    OS_TPrintf("== free queue\n");
    OS_TPrintf(" head: %d, tail: %d\n", status->sendQueueFreeList.head,
              status->sendQueueFreeList.tail);
//    WMi_DebugPrintSendQueue( &status->sendQueueFreeList );
#else
    DC_InvalidateRange(wm9buf->status, WM_STATUS_BUF_SIZE);     // Invalidates the ARM7 status region cache
    OS_TPrintf("== ready queue [2]\n");
    OS_TPrintf(" head: %d, tail: %d\n", status->readyQueue[2].head, status->readyQueue[2].tail);
#endif

}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetStatusAddress

  Description:  Gets the pointer to the status structure managed internally by WM.
                This structure is directly operated by ARM7, so writing by ARM9 is prohibited.
                Also, note that the cache line containing the portion to be referenced needs to be deleted before referencing the contents.
                

  Arguments:    None.

  Returns:      const WMStatus*: Returns a pointer to the status structure.
 *---------------------------------------------------------------------------*/
const WMStatus *WMi_GetStatusAddress(void)
{
    // Initialization check
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return NULL;
    }

    return wm9buf->status;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckMpPacketTimeRequired

  Description:  Confirms that no more than 5,600 microseconds are needed to send a single MP communication packet.
                

  Arguments:    parentSize: Size of parent transfer data
                childSize: Size of child transfer data
                childs: Number of child devices to communicate with

  Returns:      BOOL: If within the acceptable range, returns TRUE.
                      If it exceeds 5600 microseconds, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL WMi_CheckMpPacketTimeRequired(u16 parentSize, u16 childSize, u8 childs)
{
    s32     mp_time;

    // Calculate the time it takes for one MP communication in units of microseconds
    mp_time = ((                       // --- Parent send portion ---
                   96                  // Preamble
                   + (24               // 802.11 Header
                      + 4              // TXOP + PollBitmap
                      + 2              // wmHeader
                      + parentSize + 4 // wmFooter( parent )
                      + 4              // FCS
                   ) * 4               // 4 microseconds per byte
               ) + (                   // --- Child send portion ---
                       (10             // SIFS
                        + 96           // Preamble
                        + (24          // 802.11 Header
                           + 2         // wmHeader
                           + childSize + 2      // wmFooter( child )
                           + 4         // FCS
                        ) * 4          // 4 microseconds per byte
                        + 6            // time to spare
                       ) * childs) + ( // --- MP ACK send portion ---
                                         10     // SIFS
                                         + 96   // Preamble
                                         + (24  // 802.11 Header
                                            + 4 // ACK
                                            + 4 // FCS
                                         ) * 4  // 4microseconds per byte
               ));

    if (mp_time > WM_MAX_MP_PACKET_TIME)
    {
        OS_TWarning
            ("It is required %dus to transfer each MP packet.\nThat should not exceed %dus.\n",
             mp_time, WM_MAX_MP_PACKET_TIME);
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_IsMP

  Description:  Gets current MP communication state.

  Arguments:    None.

  Returns:      TRUE if in MP communication state.
 *---------------------------------------------------------------------------*/
BOOL WMi_IsMP(void)
{
    WMStatus *status;
    BOOL    isMP;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // Initialization check
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return FALSE;
    }
#endif

    // Prohibit interrupts
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        status = wm9buf->status;
        DC_InvalidateRange(&(status->mp_flag), 4);
        isMP = status->mp_flag;
    }
    else
    {
        isMP = FALSE;
    }

    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

    return isMP;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetAID

  Description:  Gets current AID.
                Return a valid value only when the state is either PARENT, MP_PARENT, CHILD, or MP_CHILD.
                

  Arguments:    None.

  Returns:      AID
 *---------------------------------------------------------------------------*/
u16 WM_GetAID(void)
{
    u16     myAid;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // Initialization check
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }
#endif

    // Prohibit interrupts
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        myAid = wm9buf->myAid;
    }
    else
    {
        myAid = 0;
    }

    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

    return myAid;
}

/*---------------------------------------------------------------------------*
  Name:         WM_GetConnectedAIDs

  Description:  Gets the currently connected partners in bitmap format.
                Return a valid value only when the state is either PARENT, MP_PARENT, CHILD, or MP_CHILD.
                
                For a child device, returns 0x0001 when the child is connected to a parent.

  Arguments:    None.

  Returns:      Bitmap of AIDs of connected partners.
 *---------------------------------------------------------------------------*/
u16 WM_GetConnectedAIDs(void)
{
    u32     connectedAidBitmap;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // Initialization check
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return 0;
    }
#endif

    // Prohibit interrupts
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        connectedAidBitmap = wm9buf->connectedAidBitmap;
    }
    else
    {
        connectedAidBitmap = 0;
    }

    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

#ifdef WM_DEBUG
/*
    if (WMi_CheckStateEx(4, WM_STATE_PARENT, WM_STATE_CHILD, WM_STATE_MP_PARENT, WM_STATE_MP_CHILD)
        != WM_ERRCODE_SUCCESS && connectedAidBitmap != 0)
    {
        WM_WARNING("connectedAidBitmap should be 0, but %04x", connectedAidBitmap);
    }
*/
#endif

    return (u16)connectedAidBitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_GetMPReadyAIDs

  Description:  From among the currently connected parties, gets a list in bitmap format of the AIDs of parties which can receive MP.
                
                Return a valid value only when the state is either PARENT, MP_PARENT, CHILD, or MP_CHILD.
                
                For a child device, returns 0x0001 when the child is connected to a parent.

  Arguments:    None.

  Returns:      Bitmap of the AIDs of partners with which MP is started.
 *---------------------------------------------------------------------------*/
u16 WMi_GetMPReadyAIDs(void)
{
    WMStatus *status;
    u16     mpReadyAidBitmap;
    OSIntrMode e;

#ifdef SDK_DEBUG
    // Initialization check
    if (WMi_CheckInitialized() != WM_ERRCODE_SUCCESS)
    {
        return FALSE;
    }
#endif

    // Prohibit interrupts
    e = OS_DisableInterrupts();

    if (wm9buf != NULL)
    {
        status = wm9buf->status;
        DC_InvalidateRange(&(status->mp_readyBitmap), 2);
        mpReadyAidBitmap = status->mp_readyBitmap;
    }
    else
    {
        mpReadyAidBitmap = FALSE;
    }

    // End prohibiting of interrupts
    (void)OS_RestoreInterrupts(e);

    return mpReadyAidBitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_RegisterSleepCallback

  Description:  Registers the callback function that will be run when shifting to Sleep Mode.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_RegisterSleepCallback(void)
{
    PM_SetSleepCallbackInfo(&sleepCbInfo, WmSleepCallback, NULL);
    PMi_InsertPreSleepCallbackEx(&sleepCbInfo, PM_CALLBACK_PRIORITY_WM );
}

/*---------------------------------------------------------------------------*
  Name:         WMi_DeleteSleepCallback

  Description:  Deletes the callback function that is run when shifting to Sleep Mode.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WMi_DeleteSleepCallback(void)
{
    PM_DeletePreSleepCallback( &sleepCbInfo );
}

/*---------------------------------------------------------------------------*
  Name:         WmSleepCallback

  Description:  Prevents the program from entering Sleep Mode during wireless communications.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WmSleepCallback(void *)
{
    /* ---------------------------------------------- *
     * As described in section 6.5 of the Programming Guidelines, it is prohibited to run the OS_GoSleepMode function during wireless communications
     * 
     * 
     * ---------------------------------------------- */
    OS_TPanic("Could not sleep during wireless communications.");
}

#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         WM_GetWirelessCommFlag

  Description:  Checks whether wireless communications can be performed now.

  Arguments:    None.

  Returns:      WM_WIRELESS_COMM_FLAG_OFF: Wireless communications are not allowed.
                WM_WIRELESS_COMM_FLAG_ON: Wireless communications are allowed.
                WM_WIRELESS_COMM_FLAG_UNKNOWN: This cannot be determined because the program is running on a Nintendo DS system.
 *---------------------------------------------------------------------------*/
u8 WM_GetWirelessCommFlag( void )
{
    u8 result = WM_WIRELESS_COMM_FLAG_UNKNOWN;
    
    if( OS_IsRunOnTwl() )
    {
        if( WMi_CheckEnableFlag() )
        {
            result = WM_WIRELESS_COMM_FLAG_ON;
        }
        else
        {
            result = WM_WIRELESS_COMM_FLAG_OFF;
        }
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WMi_CheckEnableFlag

  Description:  Checks the flag that allows wireless use, configurable from the TWL System Settings.

  Arguments:    None.

  Returns:      TRUE if wireless features can be used and FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL WMi_CheckEnableFlag(void)
{

    if( OS_IsAvailableWireless() == TRUE && OS_IsForceDisableWireless() == FALSE )
    {
        return TRUE;
    }
    return FALSE;
}
#endif

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
