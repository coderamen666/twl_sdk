/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol_impl_wxc.c

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
#include <nitro/wxc/protocol.h>
#include <nitro/wxc/wxc_protocol_impl_wxc.h>


/*****************************************************************************/
/* Declaration */

static void WXCi_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param);
static BOOL WXCi_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc);
static void WXCi_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet);
static BOOL WXCi_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet);
static void WXCi_InitSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max);
static BOOL WXCi_AddData(WXCProtocolContext * protocol, const void *send_buf, u32 send_size,
                         void *recv_buf, u32 recv_max);
static BOOL WXCi_IsExecuting(WXCProtocolContext * protocol);


/*****************************************************************************/
/* Variables */

/* Work buffer for WXC protocol */
static WXCImplWorkWxc impl_wxc_work;

/* WXC protocol interface */
static WXCProtocolImpl impl_wxc = {
    "WXC",
    WXCi_BeaconSendHook,
    WXCi_BeaconRecvHook,
    NULL,
    WXCi_PacketSendHook,
    WXCi_PacketRecvHook,
    WXCi_InitSequence,
    WXCi_AddData,
    WXCi_IsExecuting,
    &impl_wxc_work,
};


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetProtocolImplWXC

  Description:  Gets the interface for the WXC chance encounter communication protocol.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl* WXCi_GetProtocolImplWXC(void)
{
    return &impl_wxc;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_BeaconSendHook

  Description:  Hook called at beacon update.

  Arguments:    protocol: WXCProtocolContext structure
                p_param: WMParentParam structure used for next beacon
                              Change inside the function as necessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param)
{
#pragma unused(protocol)
#pragma unused(p_param)

    /*
     * UserGameInfo is not currently used in the WXC library.
     * This is planned to be extended as necessary in the future when GGID net masking, common GGID mode, and other features are adopted.
     * 
     */
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_BeaconRecvHook

  Description:  Hook called for individual scanned beacons.

  Arguments:    protocol: WXCProtocolContext structure
                p_desc: Scanned WMBssDesc structure

  Returns:      If it is seen as connection target, return TRUE. Otherwise, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc)
{
#pragma unused(protocol)
#pragma unused(p_desc)

    /* Only the "local mode," which distinguishes only based on GGID */

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PacketSendHook

  Description:  Hook called at MP packet transmission.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring transmission packet information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    WXCBlockHeader *p_header = (WXCBlockHeader *) packet->buffer;

    WXC_PACKET_LOG("--SEND:ACK=(%3d,%d,%04X),REQ=(%3d,%d,%04X)\n",
                   wxc_work->ack.phase, wxc_work->ack.command, wxc_work->ack.index,
                   wxc_work->req.phase, wxc_work->req.command, wxc_work->req.index);

    /* Set packet header to the send buffer */
    p_header->req = wxc_work->req;
    p_header->ack = wxc_work->ack;

    /* Send response data only when the phase matches */
    if (wxc_work->ack.phase == wxc_work->req.phase)
    {
        u8     *p_body = packet->buffer + sizeof(WXCBlockHeader);

        switch (wxc_work->ack.command)
        {
        case WXC_BLOCK_COMMAND_INIT:
            /* Send initial information (size + checksum) */
            WXC_PACKET_LOG("       INIT(%6d)\n", protocol->send.length);
            *(u16 *)(p_body + 0) = (u16)protocol->send.length;
            *(u16 *)(p_body + 2) = protocol->send.checksum;
            break;
        case WXC_BLOCK_COMMAND_SEND:
            /* Send last requested index */
            {
                int     offset = (wxc_work->ack.index * wxc_work->send_unit);
                u32     len = (u32)(protocol->send.length - offset);
                if (len > wxc_work->send_unit)
                {
                    len = wxc_work->send_unit;
                }
                MI_CpuCopy8((const u8 *)protocol->send.buffer + offset, p_body, len);
            }
            break;
        }
    }

    /* Specify packet size */
    packet->length = (u16)MATH_ROUNDUP(sizeof(WXCBlockHeader) + wxc_work->send_unit, 2);
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_MergeBlockData

  Description:  Saves the received block data fragment.

  Arguments:    protocol: WXCProtocolContext structure
                index: Receive data index
                src: Receive data buffer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WXCi_MergeBlockData(WXCProtocolContext * protocol, int index, const void *src)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    if (index < wxc_work->recv_total)
    {
        u32    *bmp = wxc_work->recv_bitmap_buf + (index >> 5);
        u32     bit = (u32)(1 << (index & 31));
        if ((*bmp & bit) == 0)
        {
            int     offset = (index * wxc_work->recv_unit);
            u32     len = (u32)(protocol->recv.length - offset);
            if (len > wxc_work->recv_unit)
            {
                len = wxc_work->recv_unit;
            }
            /* Don't save if recv.buffer is NULL */
            if (protocol->recv.buffer != NULL)
            {
                MI_CpuCopy8(src, (u8 *)protocol->recv.buffer + offset, len);
            }
            *bmp |= bit;
            /* If data reception is finished, set as DONE */
            if (--wxc_work->recv_rest == 0)
            {
                wxc_work->req.command = WXC_BLOCK_COMMAND_DONE;
            }
            /* If not, search for the non-received index */
            else
            {
                int     i;
                /* Use the previously requested index as a base point */
                int     count = wxc_work->recent_index[0];
                int     last_count = count;
                if (last_count >= wxc_work->recv_total)
                {
                    last_count = (int)wxc_work->recv_total - 1;
                }
                for (;;)
                {
                    /* Loop to the beginning once the end point is reached */
                    if (++count >= wxc_work->recv_total)
                    {
                        count = 0;
                    }
                    /* Reselect from history once the search finishes a single loop */
                    if (count == last_count)
                    {
                        count = wxc_work->recent_index[WXC_RECENT_SENT_LIST_MAX - 1];
                        break;
                    }
                    /* Determine if it is received yet */
                    if ((*(wxc_work->recv_bitmap_buf + (count >> 5)) & (1 << (count & 31))) == 0)
                    {
                        /* Determine whether it is a recently requested index */
                        for (i = 0; i < WXC_RECENT_SENT_LIST_MAX; ++i)
                        {
                            if (count == wxc_work->recent_index[i])
                            {
                                break;
                            }
                        }
                        if (i >= WXC_RECENT_SENT_LIST_MAX)
                        {
                            break;
                        }
                    }
                }
                /* Update the index request history */
                for (i = WXC_RECENT_SENT_LIST_MAX; --i > 0;)
                {
                    wxc_work->recent_index[i] = wxc_work->recent_index[i - 1];
                }
                wxc_work->recent_index[0] = (u16)count;
                wxc_work->req.index = wxc_work->recent_index[0];
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_PacketRecvHook

  Description:  Hook called at MP packet reception.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring reception packet information

  Returns:      If a single data exchange is completed, return TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXCi_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    int     ret = FALSE;

    WXCBlockHeader *p_header = (WXCBlockHeader *) packet->buffer;

    if (packet->length >= wxc_work->recv_unit)
    {
        WXC_PACKET_LOG("--RECV:REQ=(%3d,%d,%04X),ACK=(%3d,%d,%04X)\n",
                       p_header->req.phase, p_header->req.command, p_header->req.index,
                       p_header->ack.phase, p_header->ack.command, p_header->ack.index);

        /* Save requested data only for targets with a same phase */
        if (p_header->req.phase == wxc_work->req.phase)
        {
            wxc_work->ack = p_header->req;
        }

        /* View response data only for targets with a same phase */
        if (p_header->ack.phase == wxc_work->req.phase)
        {
            u8     *p_body = packet->buffer + sizeof(WXCBlockHeader);

            /* Data receive process for each command */
            switch (p_header->ack.command)
            {
            case WXC_BLOCK_COMMAND_QUIT:
                /* Disconnect protocol */
                wxc_work->executing = FALSE;
                break;
            case WXC_BLOCK_COMMAND_INIT:
                /* Receive initial information (size + checksum) */
                protocol->recv.length = *(u16 *)(p_body + 0);
                protocol->recv.checksum = *(u16 *)(p_body + 2);
                wxc_work->recv_total =
                    (u16)((protocol->recv.length + wxc_work->recv_unit - 1) / wxc_work->recv_unit);
                wxc_work->recv_rest = wxc_work->recv_total;
                wxc_work->req.index = 0;
                /* Transition to WXC_BLOCK_COMMAND_DONE if size is 0 */
                if (wxc_work->recv_total > 0)
                {
                    wxc_work->req.command = WXC_BLOCK_COMMAND_SEND;
                }
                else
                {
                    wxc_work->req.command = WXC_BLOCK_COMMAND_DONE;
                }
                WXC_PACKET_LOG("       INIT(%6d)\n", protocol->recv.length);
                break;
            case WXC_BLOCK_COMMAND_SEND:
                /* Store the received data of an actual response */
                WXCi_MergeBlockData(protocol, p_header->ack.index, p_body);
                break;
            }
        }

        /* Data exchange completed in both directions */
        if ((p_header->ack.phase == wxc_work->req.phase) &&
            (p_header->ack.command == WXC_BLOCK_COMMAND_DONE) &&
            (wxc_work->ack.command == WXC_BLOCK_COMMAND_DONE))
        {
            /* Increment data exchange phase and reinitialize */
            ++wxc_work->req.phase;
            /* Have so that it will become !executing if the communication is continued */
            wxc_work->req.command = WXC_BLOCK_COMMAND_QUIT;
            ret = TRUE;
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitSequence

  Description:  Reinitializes the WXC library protocol.

  Arguments:    protocol: WXCProtocolContext structure
                send_max: Maximum send packet size
                recv_max: Maximum receive packet size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_InitSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* Initialize send buffer */
    protocol->send.buffer = NULL;
    protocol->send.length = 0;
    protocol->send.checksum = 0;
    protocol->recv.buffer = NULL;
    protocol->recv.length = 0;
    protocol->recv.buffer_max = 0;

    /* Calculate the split size for the block transfer */
    wxc_work->send_unit = (u16)(send_max - sizeof(WXCBlockHeader));
    wxc_work->recv_unit = (u16)(recv_max - sizeof(WXCBlockHeader));

    /* Initialize the send management information */
    {
        int     i;
        for (i = 0; i < WXC_RECENT_SENT_LIST_MAX; ++i)
        {
            wxc_work->recent_index[i] = 0;
        }
    }
    /* Initialize the data receive management information */
    wxc_work->req.phase = 0;
    wxc_work->recv_total = 0;

    /*
     * Initialize this so that if communication is started without even calling the AddData function, the !IsExecuting function will become true
     * 
     */
    wxc_work->req.command = WXC_BLOCK_COMMAND_QUIT;
    /*
     * If this is done for the ACK signal to the communication partner, however, the process will shut down immediately. As a result, set IDLE (no response)
     * 
     */
    wxc_work->ack.phase = 0;
    wxc_work->ack.command = WXC_BLOCK_COMMAND_IDLE;

    MI_CpuClear32(wxc_work->recv_bitmap_buf, sizeof(wxc_work->recv_bitmap_buf));

    wxc_work->executing = TRUE;

}

/*---------------------------------------------------------------------------*
  Name:         WXCi_AddData

  Description:  Configures the block data exchange.

  Arguments:    protocol: WXCProtocolContext structure
                send_buf: Send buffer
                send_size: Send buffer size
                recv_buf: Receive buffer
                recv_max: Receive buffer size

  Returns:      If registration is possible, return TRUE after configuration.
 *---------------------------------------------------------------------------*/
BOOL WXCi_AddData(WXCProtocolContext * protocol, const void *send_buf, u32 send_size,
                  void *recv_buf, u32 recv_max)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    if (wxc_work->req.command == WXC_BLOCK_COMMAND_QUIT)
    {
        wxc_work->req.command = WXC_BLOCK_COMMAND_INIT;

        protocol->send.buffer = (void *)send_buf;
        protocol->send.length = (u16)send_size;
        protocol->send.checksum = MATH_CalcChecksum8(send_buf, send_size);

        protocol->recv.buffer = recv_buf;
        protocol->recv.buffer_max = (u16)recv_max;
        MI_CpuClear32(wxc_work->recv_bitmap_buf, sizeof(wxc_work->recv_bitmap_buf));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsExecuting

  Description:  Checks whether data is currently being exchanged.

  Arguments:    None.

  Returns:      Return TRUE if data is currently being exchanged.
 *---------------------------------------------------------------------------*/
BOOL WXCi_IsExecuting(WXCProtocolContext * protocol)
{
    WXCImplWorkWxc *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    return wxc_work->executing;
}
