/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_dcf.c

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
  Name:         WM_StartDCF

  Description:  Starts infrastructure-mode communications.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                recvBuf     -   Pointer to the data receive buffer.
                                Pay attention to cache because ARM7 writes out data directly.
                recvBufSize -   Size of data receive buffer.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_StartDCF(WMCallbackFunc callback, WMDcfRecvBuf *recvBuf, u16 recvBufSize)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckState(WM_STATE_CHILD);
    WM_CHECK_RESULT(result);

    // Confirm DCF state
    DC_InvalidateRange(&(p->status->dcf_flag), 4);
    if (p->status->dcf_flag == TRUE)
    {
        WM_WARNING("Already DCF mode. So can't start DCF again.\n");
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    // Check parameters
    if (recvBufSize < 16)
    {
        WM_WARNING("Parameter \"recvBufSize\" must not be less than %d.\n", 16);
        return WM_ERRCODE_INVALID_PARAM;
    }
    if (recvBuf == NULL)
    {
        WM_WARNING("Parameter \"recvBuf\" must not be NULL.\n");
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)recvBuf & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"recvBuf\" is not 32-byte aligned.\n");
    }

    // Write out the specified buffer's cache
    DC_StoreRange(recvBuf, recvBufSize);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_START_DCF, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_START_DCF, 2, (u32)recvBuf, (u32)recvBufSize);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_SetDCFData

  Description:  Reserves data for sending by infrastructure-mode communication.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.
                destAdr     -   Pointer to the buffer that shows the MAC address of the communication partner.
                sendData    -   Pointer to the data to reserve send.
                                NOTE: The instance of the data reserved for sending is forcibly stored in cache.
                                
                sendDataSize -  Size of the data to reserve send.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode
WM_SetDCFData(WMCallbackFunc callback, const u8 *destAdr, const u16 *sendData, u16 sendDataSize)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();
    u32     wMac[2];

    // Check the state
    result = WMi_CheckState(WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    // Confirm DCF state
    DC_InvalidateRange(&(p->status->dcf_flag), 4);
    if (p->status->dcf_flag == FALSE)
    {
        WM_WARNING("It is not DCF mode now.\n");
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    // Check parameters
    if (sendDataSize > WM_DCF_MAX_SIZE)
    {
        WM_WARNING("Parameter \"sendDataSize\" is over %d.\n", WM_DCF_MAX_SIZE);
        return WM_ERRCODE_INVALID_PARAM;
    }
    if ((u32)sendData & 0x01f)
    {
        // Alignment check is a warning only, not an error
        WM_WARNING("Parameter \"sendData\" is not 32-byte aligned.\n");
    }

    // Write out the specified buffer's cache
    DC_StoreRange((void *)sendData, sendDataSize);

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_SET_DCF_DATA, callback);

    // Copy MAC address
    MI_CpuCopy8(destAdr, wMac, 6);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_SET_DCF_DATA, 4,
                             (u32)wMac[0], (u32)wMac[1], (u32)sendData, (u32)sendDataSize);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         WM_EndDCF

  Description:  Stops infrastructure-mode communications.

  Arguments:    callback    -   Callback function that is called when the asynchronous process completes.

  Returns:      WMErrCode   -   Returns the processing result. Returns WM_ERRCODE_OPERATING if asynchronous processing started successfully. Afterwards, the asynchronous processing results will be passed to the callback.
                                
                                
 *---------------------------------------------------------------------------*/
WMErrCode WM_EndDCF(WMCallbackFunc callback)
{
    WMErrCode result;
    WMArm9Buf *p = WMi_GetSystemWork();

    // Check the state
    result = WMi_CheckState(WM_STATE_DCF_CHILD);
    WM_CHECK_RESULT(result);

    // Confirm DCF state
    DC_InvalidateRange(&(p->status->dcf_flag), 4);
    if (p->status->dcf_flag == FALSE)
    {
        WM_WARNING("It is not DCF mode now.\n");
        return WM_ERRCODE_ILLEGAL_STATE;
    }

    // Register callback function
    WMi_SetCallbackTable(WM_APIID_END_DCF, callback);

    // Notify ARM7 with FIFO
    result = WMi_SendCommand(WM_APIID_END_DCF, 0);
    WM_CHECK_RESULT(result);

    return WM_ERRCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
