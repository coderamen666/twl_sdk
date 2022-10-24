/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_wm.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#include <nitro.h>
#include "mb_common.h"
#include "mb_wm.h"
#include "mb_child.h"
#include "mb_wm_base.h"
#include "mb_block.h"

//===========================================================================
// Prototype Declarations
//===========================================================================

static BOOL IsSendEnabled(void);
static void MBi_WMStateOutStartConnect(void *arg);
static void ChildStateOutStartMP(void *arg);
static void ChildPortCallback(void *arg);
static void StateOutMPSendToParent(void *arg);

static void MBi_WMStateOutStartConnect(void *arg);

static void MBi_WMStateOutEndMP(void *arg);

static void MBi_WMStateOutDisconnect(void *arg);
static void MBi_WMStateInDisconnect(void);

static void MBi_WMStateOutReset(void *arg);

static void MBi_WMSendCallback(u16 type, void *arg);
static void MBi_WMErrorCallback(u16 apiid, u16 error_code);
static void MBi_WMApiErrorCallback(u16 apiid, u16 error_code);


//===========================================================================
// Variable Declarations
//===========================================================================

static MBWMWork *wmWork = NULL;


//===========================================================================
// Function Definitions
//===========================================================================

/*---------------------------------------------------------------------------*
  Name:         MBi_WMSetBuffer

  Description:  Sets the buffer that MB_WM uses for MP communication.
                You must provide only the size of MBWMWork.

  Arguments:    buf:     Pointer to the region used as a work buffer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMSetBuffer(void *buf)
{
    SDK_NULL_ASSERT(buf);
    SDK_ASSERT(((u32)buf & 0x1f) == 0); // Whether 32-byte aligned or not.

    wmWork = (MBWMWork *) buf;
    wmWork->start_mp_busy = 0;         // Prevent multiple parent StartMP calls.
    wmWork->mpStarted = 0;
    wmWork->child_bitmap = 0;
    wmWork->mpBusy = 0;
    wmWork->endReq = 0;
    wmWork->sendBuf = NULL;
    wmWork->recvBuf = NULL;
    wmWork->mpCallback = NULL;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMSetCallback

  Description:  Sets a callback.

  Arguments:    callback:    Callback function.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMSetCallback(MBWMCallbackFunc callback)
{
    OSIntrMode enabled = OS_DisableInterrupts();

    wmWork->mpCallback = callback;

    (void)OS_RestoreInterrupts(enabled);
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStartConnect

  Description:  Begins a connection to a parent device.

  Arguments:    bssDesc:   BssDesc of the parent device to connect.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMStartConnect(WMBssDesc *bssDesc)
{
    WMErrCode result;

    wmWork->mpStarted = 0;
    wmWork->endReq = 0;

    wmWork->sendBufSize = (u16)WM_SIZE_MP_CHILD_SEND_BUFFER(bssDesc->gameInfo.childMaxSize, FALSE);
    wmWork->recvBufSize =
        (u16)WM_SIZE_MP_CHILD_RECEIVE_BUFFER(bssDesc->gameInfo.parentMaxSize, FALSE);
    wmWork->pSendLen = bssDesc->gameInfo.parentMaxSize;
    wmWork->pRecvLen = bssDesc->gameInfo.childMaxSize;
    wmWork->blockSizeMax = (u16)MB_COMM_CALC_BLOCK_SIZE(wmWork->pSendLen);
    MBi_SetChildMPMaxSize(wmWork->pRecvLen);

    result = WM_StartConnect(MBi_WMStateOutStartConnect, bssDesc, NULL);

    if (result != WM_ERRCODE_OPERATING)
    {
        MBi_WMSendCallback(MB_CALLBACK_CONNECT_FAILED, NULL);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStateOutStartConnect

  Description:  Connection callback to the parent.

  Arguments:    arg:     WM_StartConnect callback argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_WMStateOutStartConnect(void *arg)
{
    WMStartConnectCallback *cb = (WMStartConnectCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        MBi_WMSendCallback(MB_CALLBACK_CONNECT_FAILED, arg);
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_BEACON_LOST:
        break;
    case WM_STATECODE_CONNECT_START:
        break;
    case WM_STATECODE_DISCONNECTED:
        MBi_WMSendCallback(MB_CALLBACK_DISCONNECTED_FROM_PARENT, NULL);
        break;
    case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        // Do nothing if it disconnects itself.
        break;
    case WM_STATECODE_CONNECTED:
        // When authentication is complete.
        MBi_WMSendCallback(MB_CALLBACK_CONNECTED_TO_PARENT, arg);
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_ChildStartMP

  Description:  Start MP communication

  Arguments:    sendBuf:     Pointer to the region to set as the send buffer.
                recvBuf:     Pointer to the region to set as the receive buffer.

  Returns:      Error code, normally WM_ERRCODE_OPERATING.
 *---------------------------------------------------------------------------*/
void MBi_ChildStartMP(u16 *sendBuf, u16 *recvBuf)
{
    WMErrCode result;

    wmWork->sendBuf = (u32 *)sendBuf;
    wmWork->recvBuf = (u32 *)recvBuf;

    result = WM_SetPortCallback(WM_PORT_BT, ChildPortCallback, NULL);
    if (result != WM_ERRCODE_SUCCESS)
    {
        MBi_WMApiErrorCallback(WM_APIID_START_MP, result);
        return;
    }

    result = WM_StartMPEx(ChildStateOutStartMP, recvBuf, wmWork->recvBufSize, sendBuf, wmWork->sendBufSize, 1,  // mpFreq
                          0,           // defaultRetryCount
                          FALSE,       // minPollBmpMode
                          FALSE,       // singlePacketMode
                          TRUE,        // fixFrameMode
                          TRUE);       // ignoreFatalError

    if (result != WM_ERRCODE_OPERATING)
    {
        MBi_WMApiErrorCallback(WM_APIID_START_MP, result);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ChildStateOutStartMP

  Description:  Child WM_StartMPEx callback function.

  Arguments:    arg:     WM_StartMP callback argument.

  Returns:      Error code, normally WM_ERRCODE_OPERATING.
 *---------------------------------------------------------------------------*/
static void ChildStateOutStartMP(void *arg)
{
    WMStartMPCallback *cb = (WMStartMPCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        // End when the error notification does not require error handling.
        if (cb->errcode == WM_ERRCODE_SEND_FAILED)
        {
            return;
        }
        else if (cb->errcode == WM_ERRCODE_TIMEOUT)
        {
            return;
        }
        else if (cb->errcode == WM_ERRCODE_INVALID_POLLBITMAP)
        {
            return;
        }

        MBi_WMErrorCallback(cb->apiid, cb->errcode);
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_MP_START:
        wmWork->mpStarted = 1;         // Set the flag that indicates MP has started.
        wmWork->mpBusy = 0;
        wmWork->child_bitmap = 0;
        MBi_WMSendCallback(MB_CALLBACK_MP_STARTED, NULL);
        {
            // MP send-enabled callback.
            MBi_WMSendCallback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
        }
        break;

    case WM_STATECODE_MP_IND:
        // None.
        break;

    case WM_STATECODE_MPACK_IND:
        // None.
        break;

    case WM_STATECODE_MPEND_IND:      // Only occurs on a parent device.
    default:
        MBi_WMErrorCallback(cb->apiid, WM_ERRCODE_FAILED);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBi_WMDisconnect

  Description:  Child MP disconnect processing. Disconnects from the parent after WM_EndMP completes.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMDisconnect(void)
{
    WMErrCode result;

    wmWork->endReq = 1;

    result = WM_EndMP(MBi_WMStateOutEndMP);

    if (result != WM_ERRCODE_OPERATING)
    {
        wmWork->endReq = 0;
        MBi_WMApiErrorCallback(WM_APIID_END_MP, result);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStateOutEndMP

  Description:  The callback function for WM_EndMP.

  Arguments:    arg:     The callback argument for WM_EndMP.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_WMStateOutEndMP(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        wmWork->endReq = 0;
        MBi_WMErrorCallback(cb->apiid, cb->errcode);
        return;
    }

    wmWork->mpStarted = 0;
    MBi_WMStateInDisconnect();
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStateInDisconnect

  Description:  Disconnects a child from the parent and transitions to the IDLE state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_WMStateInDisconnect(void)
{
    WMErrCode result;

    result = WM_Disconnect(MBi_WMStateOutDisconnect, 0);

    if (result != WM_ERRCODE_OPERATING)
    {
        wmWork->endReq = 0;
        MBi_WMApiErrorCallback(WM_APIID_DISCONNECT, result);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStateInDisconnect

  Description:  WM_Disconnect callback argument.

  Arguments:    arg:  WM_Disconnect callback argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_WMStateOutDisconnect(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;

    wmWork->endReq = 0;
    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        MBi_WMErrorCallback(cb->apiid, cb->errcode);
        return;
    }
    MBi_WMSendCallback(MB_CALLBACK_DISCONNECT_COMPLETE, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMReset

  Description:  Resets the child, and causes a transition to the IDLE state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMReset(void)
{
    WMErrCode result;

    result = WM_Reset(MBi_WMStateOutReset);
    if (result != WM_ERRCODE_OPERATING)
    {
        MBi_WMApiErrorCallback(WM_APIID_RESET, result);
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMStateOutReset

  Description:  Resets the child, and causes a transition to the IDLE state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MBi_WMStateOutReset(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        MBi_WMErrorCallback(cb->apiid, cb->errcode);
        return;
    }
    // Reset sets to the idling (standby) state without starting the next state.
    MBi_WMSendCallback(MB_CALLBACK_DISCONNECT_COMPLETE, NULL);
}


/*
 * Check MP transmit permission
   
   Added the mpBusy flag, which is set when SetMP is executed, to the determination elements, so that MP will not be set again after SetMP and before the callback returns.
  
 
 */
/*---------------------------------------------------------------------------*
  Name:         IsSendEnabled

  Description:  This function determines whether or not it is OK to set new MP data at the present time.
                Added the mpBusy flag, which is set when SetMP is executed, to the determination elements, so that MP will not be set again after SetMP and before the callback returns.
                

  Arguments:    None.

  Returns:      TRUE if it is OK to set new data.
                Otherwise returns FALSE.
 *---------------------------------------------------------------------------*/
static BOOL IsSendEnabled(void)
{
    return (wmWork->mpStarted == 1) && (wmWork->mpBusy == 0) && (wmWork->endReq == 0);
}


/*---------------------------------------------------------------------------*
  Name:         ChildPortCallback

  Description:  Child MP port callback function.

  Arguments:    arg:     Port callback argument for MP communication.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChildPortCallback(void *arg)
{
    WMPortRecvCallback *cb = (WMPortRecvCallback *)arg;

    if (cb->errcode != WM_ERRCODE_SUCCESS)
    {
        return;
    }

    switch (cb->state)
    {
    case WM_STATECODE_PORT_RECV:
        // Notify the program that data was received.
        MBi_WMSendCallback(MB_CALLBACK_MP_CHILD_RECV, cb);
        break;
    case WM_STATECODE_CONNECTED:
        // Connection notification
        break;
    case WM_STATECODE_PORT_INIT:
    case WM_STATECODE_DISCONNECTED:
    case WM_STATECODE_DISCONNECTED_FROM_MYSELF:
        break;
    }
}


/*---------------------------------------------------------------------------*
  Name:         MBi_MPSendToParent

  Description:  Sends buffer contents to the parent.

  Arguments:    body_len:  Data size.
                pollbmp:  Poll bitmap of the other party (irrelevant for a child).
                sendbuf:  Pointer to send data.
  Returns:      If send processing started successfully, returns WM_ERRCODE_OPERATING. If it failed, some other code is returned.
                
 *---------------------------------------------------------------------------*/
WMErrCode MBi_MPSendToParent(u32 body_len, u16 pollbmp, u32 *sendbuf)
{
    WMErrCode result;

    // 32 byte align check
    SDK_ASSERT(((u32)sendbuf & 0x1F) == 0);

    DC_FlushRange(sendbuf, sizeof(body_len));
    DC_WaitWriteBufferEmpty();

    if (!IsSendEnabled())
    {
        return WM_ERRCODE_FAILED;
    }

    result = WM_SetMPDataToPort(StateOutMPSendToParent,
                                (u16 *)sendbuf,
                                (u16)body_len, pollbmp, WM_PORT_BT, WM_PRIORITY_LOW);
    if (result != WM_ERRCODE_OPERATING)
    {
        return result;
    }

    wmWork->mpBusy = 1;
    return WM_ERRCODE_OPERATING;
}


/*---------------------------------------------------------------------------*
  Name:         StateOutMPSendToParent

  Description:  Notification callback for a completed MP transmission.

  Arguments:    arg:     WM_SetMPDataToPort callback argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StateOutMPSendToParent(void *arg)
{
    WMCallback *cb = (WMCallback *)arg;

    wmWork->mpBusy = 0;
    if (cb->errcode == WM_ERRCODE_SUCCESS)
    {
        MBi_WMSendCallback(MB_CALLBACK_MP_CHILD_SENT, arg);
    }
    else if (cb->errcode == WM_ERRCODE_TIMEOUT)
    {
        MBi_WMSendCallback(MB_CALLBACK_MP_CHILD_SENT_TIMEOUT, arg);
    }
    else
    {
        MBi_WMSendCallback(MB_CALLBACK_MP_CHILD_SENT_ERR, arg);
    }
    // Allow the next transmission.
    MBi_WMSendCallback(MB_CALLBACK_MP_SEND_ENABLE, NULL);
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMSendCallback

  Description:  Performs callback notification in the WM layer.

  Arguments:    type  The callback type
                arg :     Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBi_WMSendCallback(u16 type, void *arg)
{
    if (wmWork->mpCallback == NULL)
    {
        return;
    }
    wmWork->mpCallback(type, arg);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_WMErrorCallback

  Description:  Performs error notification in the WM layer.

  Arguments:    apiid:       WM_APIID that was the cause.
                error_code  Error code

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBi_WMErrorCallback(u16 apiid, u16 error_code)
{
    MBErrorCallback arg;

    if (wmWork->mpCallback == NULL)
    {
        return;
    }

    arg.apiid = apiid;
    arg.errcode = error_code;

    wmWork->mpCallback(MB_CALLBACK_ERROR, &arg);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_WMApiErrorCallback

  Description:  Performs a notification when an error occurs with the return value of a WM API call.

  Arguments:    apiid:       WM_APIID that was the cause.
                error_code  Error code

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void MBi_WMApiErrorCallback(u16 apiid, u16 error_code)
{
    MBErrorCallback arg;

    if (wmWork->mpCallback == NULL)
    {
        return;
    }

    arg.apiid = apiid;
    arg.errcode = error_code;

    wmWork->mpCallback(MB_CALLBACK_API_ERROR, &arg);
}


/*---------------------------------------------------------------------------*
  Name:         MBi_WMApiErrorCallback

  Description:  Performs a notification when an error occurs with the return value of a WM API call.

  Arguments:    apiid:       WM_APIID that was the cause.
                error_code  Error code

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_WMClearCallback(void)
{
    (void)WM_SetPortCallback(WM_PORT_BT, NULL, NULL);
}
