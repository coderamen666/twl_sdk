/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol.c

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


/*****************************************************************************/
/* Variables */

/* Chance encounter communications protocol added up to this point */
static WXCProtocolImpl *impl_list;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocol

  Description:  Initializes the WXC library protocol.

  Arguments:    protocol: WXCProtocolContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InitProtocol(WXCProtocolContext * protocol)
{
    protocol->current_block = NULL;
    MI_CpuClear32(protocol, sizeof(protocol));
}

/*---------------------------------------------------------------------------*
  Name:         WXC_InstallProtocolImpl

  Description:  Adds a new selectable protocol.

  Arguments:    impl: Pointer to the WXCProtocolImpl structure.
                      This structure is used internally until the library is closed.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InstallProtocolImpl(WXCProtocolImpl * impl)
{
    WXCProtocolImpl *p = impl_list;
    if (!p)
    {
        impl_list = impl;
        impl->next = NULL;
    }
    else
    {
        for (;; p = p->next)
        {
            if (p == impl)
            {
                break;
            }
            else if (!p->next)
            {
                p->next = impl;
                impl->next = NULL;
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindProtocolImpl

  Description:  Searches for a protocol with the specified name.

  Arguments:    name: Protocol name

  Returns:      If a protocol with the specified name is found, its pointer is returned.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl *WXC_FindProtocolImpl(const char *name)
{
    WXCProtocolImpl *p = impl_list;
    for (; p; p = p->next)
    {
        if (STD_CompareString(name, p->name) == 0)
        {
            break;
        }
    }
    return p;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_ResetSequence

  Description:  Reinitializes the WXC library protocol.

  Arguments:    protocol: WXCProtocolContext structure
                send_max: Maximum send packet size
                recv_max: Maximum receive packet size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_ResetSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max)
{
    WXC_GetCurrentBlockImpl(protocol)->Init(protocol, send_max, recv_max);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_AddBlockSequence

  Description:  Sets block data exchange.

  Arguments:    protocol: WXCProtocolContext structure
                send_buf: Send buffer
                send_size: Send buffer size
                recv_buf: Receive buffer
                recv_max: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_AddBlockSequence(WXCProtocolContext * protocol,
                          const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max)
{
    int     result;
    result =
        WXC_GetCurrentBlockImpl(protocol)->AddData(protocol, send_buf, send_size, recv_buf,
                                                   recv_max);
    SDK_TASSERTMSG(result, "sequence is now busy.");
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindNextBlock

  Description:  Specifies a GGID and searches for block data.

  Arguments:    protocol: WXCProtocolContext structure
                from: Starting search location.
                      If NULL, search from the beginning. Otherwise, start searching from the block after the specified one.
                              
                ggid: GGID associated with the search block data.
                      0 indicates an empty block.
                match: Search condition.
                       If TRUE, data with matching GGID will be search target.
                       If FALSE, those with non-matching GGID will be search target.

  Returns:      Returns a pointer to the corresponding block data if it exists and NULL otherwise.
                
 *---------------------------------------------------------------------------*/
WXCProtocolRegistry *WXC_FindNextBlock(WXCProtocolContext * protocol,
                                       const WXCProtocolRegistry * from, u32 ggid, BOOL match)
{
    WXCProtocolRegistry *target;

    /* Search from the top if NULL */
    if (!from)
    {
        from = &protocol->data_array[WXC_REGISTER_DATA_MAX - 1];
    }

    target = (WXCProtocolRegistry *) from;
    for (;;)
    {
        BOOL    eq;
        /* Loop when the next element reach the end of the array */
        if (++target >= &protocol->data_array[WXC_REGISTER_DATA_MAX])
        {
            target = &protocol->data_array[0];
        }
        /* Close if a block matching the search criteria is found */
        eq = (target->ggid == ggid);
        if ((match && eq) || (!match && !eq))
        {
            break;
        }
        /* Close if all elements are searched */
        if (target == from)
        {
            target = NULL;
            break;
        }
    }
    return target;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconSendHook

  Description:  Hook called at beacon update.

  Arguments:    protocol: WXCProtocolContext structure
                p_param: WMParentParam structure used for next beacon
                         Change inside the function as necessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param)
{
    WXC_GetCurrentBlockImpl(protocol)->BeaconSend(protocol, p_param);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconRecvHook

  Description:  Hook called for individual scanned beacons.

  Arguments:    protocol: WXCProtocolContext structure
                p_desc: Scanned WMBssDesc structure

  Returns:      If it's seen as connection target, return TRUE. Otherwise, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL WXC_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc)
{
    BOOL    ret = FALSE;

    /* GGID 0 is treated as an empty block, so exclude */
    u32 ggid = p_desc->gameInfo.ggid;
    if (ggid != 0)
    {
        /* Searches for a match from the registered GGIDs */
        WXCProtocolRegistry *found = NULL;
        int i;
        for (i = 0; i < WXC_REGISTER_DATA_MAX; ++i)
        {
            WXCProtocolRegistry *p = &protocol->data_array[i];
            /* Strict match */
            if (p->ggid == ggid)
            {
                found = p;
                break;
            }
            /* Both are shared chance encounter */
            else if (WXC_IsCommonGgid(ggid) && WXC_IsCommonGgid(p->ggid))
            {
                /* Match if any are relay points */
                const BOOL is_target_any = (ggid == WXC_GGID_COMMON_PARENT);
                const BOOL is_current_any = (p->ggid == WXC_GGID_COMMON_ANY);
                if (is_target_any)
                {
                    if (!is_current_any)
                    {
                        ggid = p->ggid;
                        found = p;
                        break;
                    }
                }
                else
                {
                    if (is_current_any)
                    {
                        found = p;
                        break;
                    }
                }
            }
        }
        /* If a match, perform decision process inside the protocol as well */
        if (found)
        {
            ret = found->impl->BeaconRecv(protocol, p_desc);
            /* If both GGID and protocol matches, change selection */
            if (ret)
            {
                WXC_SetCurrentBlock(protocol, found);
                protocol->target_ggid = ggid;
            }
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketSendHook

  Description:  Hook called at MP packet transmission.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring transmission packet information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet)
{
    WXC_GetCurrentBlockImpl(protocol)->PacketSend(protocol, packet);
}

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketRecvHook

  Description:  Hook called at MP packet reception.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring reception packet information

  Returns:      If a single data exchange is completed, return TRUE.
 *---------------------------------------------------------------------------*/
BOOL WXC_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet)
{
    int     ret = FALSE;

    ret = WXC_GetCurrentBlockImpl(protocol)->PacketRecv(protocol, packet);
    /*
     * A single data exchange has completed.
     * If communication continues here unchanged, !IsExecuting() should be true before long.
     */
    if (ret)
    {
        /* Notify the user callback of transmission completion */
        WXCCallback callback = protocol->current_block->callback;
        if (callback)
        {
            (*callback) (WXC_STATE_EXCHANGE_DONE, &protocol->recv);
        }
        /*
         * If AddData() is run in the callback, communication should continue.
         * Otherwise, processing will advance until !IsExecuting() is true as mentioned before.
         */
    }

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_ConnectHook

  Description:  Hook called at connection detection.

  Arguments:    protocol: WXCProtocolContext structure
                bitmap: AID bitmap of the connected target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_ConnectHook(WXCProtocolContext * protocol, u16 bitmap)
{
#pragma unused(protocol)
#pragma unused(bitmap)
}

/*---------------------------------------------------------------------------*
  Name:         WXC_DisconnectHook

  Description:  Hook called at disconnection detection.

  Arguments:    protocol: WXCProtocolContext structure
                bitmap: AID bitmap of the disconnected target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_DisconnectHook(WXCProtocolContext * protocol, u16 bitmap)
{
#pragma unused(protocol)
#pragma unused(bitmap)
}

/*---------------------------------------------------------------------------*
  Name:         WXC_CallPreConnectHook

  Description:  Hook called before connecting to the communication target.

  Arguments:    protocol: WXCProtocolContext structure
                p_desc: Connection target WMBssDesc structure
                ssid: Buffer that stores the SSID for configuration

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_CallPreConnectHook(WXCProtocolContext * protocol, WMBssDesc *p_desc, u8 *ssid)
{
    WXCProtocolImpl * const impl = WXC_GetCurrentBlockImpl(protocol);
    if (impl->PreConnectHook)
    {
        impl->PreConnectHook(protocol, p_desc, ssid);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocolRegistry

  Description:  Associates GGID and callback to the specified registry structure.

  Arguments:    p_data: WXCProtocolRegistry structure used for registration
                ggid: GGID to be set
                callback: Callback function to the user
                          (Cancel setting if NULL)
                impl: Communication protocol to be adopted

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_InitProtocolRegistry(WXCProtocolRegistry * p_data, u32 ggid, WXCCallback callback,
                              WXCProtocolImpl * impl)
{
    p_data->ggid = ggid;
    p_data->callback = callback;
    p_data->impl = impl;
    p_data->send.buffer = NULL;
    p_data->send.length = 0;
    p_data->recv.buffer = NULL;
    p_data->recv.length = 0;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialExchangeBuffers

  Description:  Sets automatically used data for the initial data exchange.

  Arguments:    p_data: WXCProtocolRegistry structure used for registration
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXC_SetInitialExchangeBuffers(WXCProtocolRegistry * p_data, u8 *send_ptr, u32 send_size,
                                   u8 *recv_ptr, u32 recv_size)
{
    p_data->send.buffer = send_ptr;
    p_data->send.length = (u32)send_size;

    p_data->recv.buffer = recv_ptr;
    p_data->recv.length = (u32)recv_size;
}
