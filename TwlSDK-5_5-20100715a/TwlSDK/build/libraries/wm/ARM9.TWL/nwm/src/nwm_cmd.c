/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_cmd.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>

#include "nwm_common_private.h"
#include "nwm_arm9_private.h"

static u32 *NwmGetCommandBuffer4Arm7(void);

/*---------------------------------------------------------------------------*
  Name:         NWMi_SetCallbackTable

  Description:  Registers the callback function for each asynchronous function.

  Arguments:    id:              Asynchronous function's API ID.
                callback:        The callback function to be registered.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_SetCallbackTable(NWMApiid id, NWMCallbackFunc callback)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    sys->callbackTable[id] = callback;
}


/*---------------------------------------------------------------------------*
  Name:         NWMi_SetReceiveCallbackTable

  Description:  Registers a callback function for receiving data frames.

  Arguments:    id:              Asynchronous function's API ID.
                callback:        The callback function to be registered.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NWMi_SetReceiveCallbackTable(NWMFramePort port, NWMCallbackFunc callback)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_NULL_ASSERT(sys);

    sys->recvCallbackTable[port] = callback;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_SendCommand

  Description:  Transmits a request to the ARM7 via the FIFO.
                For commands accompanied by some number of u32-type parameters, specify the parameters by enumerating them.
                

  Arguments:    id:              API ID that corresponds to the request.
                paramNum:        Number of virtual arguments.
                ...:         Virtual argument.

  Returns:      int: Returns the result of the process as an NWM_RETCODE_* value.
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_SendCommand(NWMApiid id, u16 paramNum, ...)
{
    va_list vlist;
    s32     i;
    int     result;
    u32    *tmpAddr;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    // Reserves a buffer for command sending
    tmpAddr = NwmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        NWM_WARNING("Failed to get command buffer.\n");
        return NWM_RETCODE_FIFO_ERROR;
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

    DC_StoreRange(tmpAddr, NWM_APIFIFO_BUF_SIZE);

    // Notification with FIFO
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WMW, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        NWM_WARNING("Failed to send command through FIFO.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    return NWM_RETCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         NWMi_SendCommandDirect

  Description:  Transmits a request to the ARM7 via the FIFO.
                Directly specifies the command sent to the ARM7.

  Arguments:    data:            Command sent to the ARM7.
                length:          Size of the command sent to the ARM7.

  Returns:      int: Returns the result of the process as an NWM_RETCODE_* value.
 *---------------------------------------------------------------------------*/
NWMRetCode NWMi_SendCommandDirect(void *data, u32 length)
{
    int     result;
    u32    *tmpAddr;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    SDK_ASSERT(length <= NWM_APIFIFO_BUF_SIZE);

    // Reserves a buffer for command sending
    tmpAddr = NwmGetCommandBuffer4Arm7();
    if (tmpAddr == NULL)
    {
        NWM_WARNING("Failed to get command buffer.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    // Copies to a buffer specifically for commands sent to the ARM7.
    MI_CpuCopy8(data, tmpAddr, length);

    DC_StoreRange(tmpAddr, length);

    // Notification with FIFO
    result = PXI_SendWordByFifo(PXI_FIFO_TAG_WMW, (u32)tmpAddr, FALSE);

    (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);

    if (result < 0)
    {
        NWM_WARNING("Failed to send command through FIFO.\n");
        return NWM_RETCODE_FIFO_ERROR;
    }

    return NWM_RETCODE_OPERATING;
}

/*---------------------------------------------------------------------------*
  Name:         NwmGetCommandBuffer4Arm7

  Description:  Reserves from the pool a buffer for commands directed to ARM7.

  Arguments:    None.

  Returns:      If it can be reserved, it's that value; otherwise: NULL
 *---------------------------------------------------------------------------*/
u32 *NwmGetCommandBuffer4Arm7(void)
{
    u32    *tmpAddr = NULL;
    NWMArm9Buf *sys = NWMi_GetSystemWork();

    do {
        if (FALSE == OS_ReceiveMessage(&sys->apibufQ.q, (OSMessage *)&tmpAddr, OS_MESSAGE_NOBLOCK))
        {
            return NULL;
        }

        // invalidate entire apibuf
        DC_InvalidateRange(sys->apibuf, NWM_APIBUF_NUM * NWM_APIFIFO_BUF_SIZE);

        // Check if the ring buffer is available (queue is not full)
        DC_InvalidateRange(tmpAddr, sizeof(u16));

        if ((*((u16 *)tmpAddr) & NWM_API_REQUEST_ACCEPTED) == 0)
        {
            // return pending buffer to tail of queue
            (void)OS_SendMessage(&sys->apibufQ.q, tmpAddr, OS_MESSAGE_BLOCK);
            tmpAddr = NULL;
            continue;
        }
    } while(tmpAddr == NULL);

    return tmpAddr;
}
