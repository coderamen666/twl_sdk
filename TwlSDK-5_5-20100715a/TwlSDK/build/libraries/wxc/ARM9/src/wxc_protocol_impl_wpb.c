/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_protocol_impl_wpb.c

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
#include <nitro/wxc/wxc_protocol_impl_wpb.h>

#include <nitro/wxc.h>                 /* Use WXC_IsParentMode() */


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

static void pass_copy_to_structure(u8 *buf, PassBuffer * pb);


/*****************************************************************************/
/* Variables */

static WXCImplWorkWpb impl_work_wpb;

WXCProtocolImpl impl_wpb = {
    "WPB",
    WXCi_BeaconSendHook,
    WXCi_BeaconRecvHook,
    NULL,
    WXCi_PacketSendHook,
    WXCi_PacketRecvHook,
    WXCi_InitSequence,
    WXCi_AddData,
    WXCi_IsExecuting,
    &impl_work_wpb,
};


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXCi_GetProtocolImplWPB

  Description:  Gets the interface for the WPB chance encounter communication protocol.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl* WXCi_GetProtocolImplWPB(void)
{
    return &impl_wpb;
}

/*---------------------------------------------------------------------------*
  Name:         PrintPassBuffer

  Description:  Output debug of the received content.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintPassBuffer(void *buf, const char *message)
{
#ifdef SDK_FINALROM
#pragma unused(message)
#endif

    static PassBuffer pass;
    pass_copy_to_structure((u8 *)buf, &pass);

    OS_TPrintf("%s(req=%02X,res=%02X)\n", message, pass.req_count, pass.res_count);
    if (pass.req_count == pass.req_count)
    {
        pass.req_count = pass.req_count;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_init_recv_bitmap

  Description:  Initializes the receive history bitmap.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_data_init_recv_bitmap(PassCtrl * pass_ctrl)
{
    pass_ctrl->recv_bitmap = 0;
    pass_ctrl->recv_bitmap_index = 0;
}

/*---------------------------------------------------------------------------*
  Name:         pass_InitBuf

  Description:  Initializes the PassBuffer structure.

  Arguments:    pb: PassBuffer structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_InitBuf(PassBuffer * pb)
{
    pb->res_count = REQUEST_NONE;
    pb->req_count = 0;
    MI_CpuClear8(pb->buf, PASS_BUFFER_SIZE);
}

/*---------------------------------------------------------------------------*
  Name:         pass_ResetData

  Description:  Function called by xxxx.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_ResetData(WXCImplWorkWpb * wxc_work)
{
    PassCtrl *pc;
    pc = &wxc_work->pass_ctrl;

    pass_InitBuf(&(pc->send_buf));
    pass_InitBuf(&(pc->recv_buf));
    pass_data_init_recv_bitmap(pc);
    pc->pre_send_count = REQUEST_NONE;
    pc->reset_done = TRUE;
    pc->send_done = FALSE;
    pc->recv_done = FALSE;
    pc->hardware_buffer_count = 0;
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

    /* Process embedding common chance encounter information in gameInfo
     *Rewrite p_param->userGameInfo and p_param->userGameInfoLength
     */

    static u16 gameInfo[2] ATTRIBUTE_ALIGN(32);
    u16     gameInfoLength = sizeof gameInfo;

    /* The first 4 byte of gameInfo is reserved (0) */
    MI_CpuClear16(gameInfo, sizeof gameInfo);

    if (gameInfoLength > WM_SIZE_USER_GAMEINFO)
    {
        SDK_ASSERT(FALSE);
    }

    if (gameInfoLength)
    {
        p_param->userGameInfo = gameInfo;
    }
    p_param->userGameInfoLength = gameInfoLength;
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

    BOOL    ret = FALSE;

    /* Determine how to handle common chance encounters here   
     * userGameInfo has at least 4 bytes. If the first 4 bytes are treated as a u32 value and the most-significant bit is 0, common chance encounters are supported.
     * 
     */

    if (p_desc->gameInfoLength >=
        (char *)&p_desc->gameInfo.ggid - (char *)&p_desc->gameInfo
        + sizeof p_desc->gameInfo.ggid &&
        p_desc->gameInfo.userGameInfoLength >= 4 &&
        (*(u32 *)p_desc->gameInfo.userGameInfo & 1 << 31) == 0)
    {
        ret = TRUE;
    }

    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_get_recv_bitmap

  Description:  Checks the receive history bitmap.

  Arguments:    seq_no: Sequence number

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL pass_data_get_recv_bitmap(WXCProtocolContext * protocol, int seq_no)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    if (seq_no < pass_ctrl->recv_bitmap_index)
    {
        return TRUE;
    }
    if (seq_no >= pass_ctrl->recv_bitmap_index + 32)
    {
        return FALSE;
    }
    if (pass_ctrl->recv_bitmap & 1 << seq_no % 32)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_get_next_count

  Description:  Gets the next sequence number you will require from the other party.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static u16 pass_data_get_next_count(WXCProtocolContext * protocol)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pc = &wxc_work->pass_ctrl;

    int     count;

    if (pc->recv_bitmap_index >= wxc_work->my_pass_data.total_count)
    {
        return REQUEST_DONE;           /* All received */
    }
    count = pc->pre_send_count;
    for (;;)
    {
        count++;
        if (count >= wxc_work->my_pass_data.total_count || count >= pc->recv_bitmap_index + 32)
        {
            count = pc->recv_bitmap_index;
        }
        if (!pass_data_get_recv_bitmap(protocol, count))
        {
            pc->pre_send_count = count;
            return (u16)count;
        }
        if (count == pc->pre_send_count)
        {
            /* Control should never reach here */
            OS_TPrintf("Error ! %d %d %d %08X\n", pc->pre_send_count, pc->recv_bitmap_index,
                       wxc_work->my_pass_data.total_count, pc->recv_bitmap);
            return REQUEST_DONE;       /* All received */
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_copy_to_buffer

  Description:  Copies from the PassBuffer structure to the WM receive buffer.

  Arguments:    pb: PassBuffer structure
                buf: WM send buffer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_copy_to_buffer(PassBuffer * pb, u8 *buf)
{
    int     i;

    *buf++ = (u8)(((pb->req_count) >> 8) & 0xff);       /* HI */
    *buf++ = (u8)(pb->req_count & 0xff);        /* LO */

    *buf++ = (u8)(((pb->res_count) >> 8) & 0xff);       /* HI */
    *buf++ = (u8)(pb->res_count & 0xff);        /* LO */

    for (i = 0; i < PASS_BUFFER_SIZE; i++)
    {
        *buf++ = pb->buf[i];
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_DataToBuf

  Description:  Copies from the user send buffer to the PassBuffer structure.

  Arguments:    seq_no: Sequence number
                pb: PassBuffer structure
                buf: User send buffer

  Returns:      None.    {TRUE, TRUE, FALSE, TRUE},

 *---------------------------------------------------------------------------*/
static void pass_DataToBuf(WXCProtocolContext * protocol, int seq_no, PassBuffer * pb,
                           PassData * pd)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    const u8 *src;
    u8     *dest;

    pb->res_count = (u16)seq_no;

    if (seq_no != REQUEST_DONE && seq_no != REQUEST_NONE && seq_no != REQUEST_BYE)
    {
        src = pd->user_send_data + (seq_no * wxc_work->send_unit);
        dest = pb->buf;
        if (seq_no == wxc_work->my_pass_data.total_count - 1)
        {
            int     mod = wxc_work->my_pass_data.size % wxc_work->send_unit;
            if (mod)
            {
                MI_CpuCopy8(src, dest, (u32)mod);
            }
            else
            {
                MI_CpuCopy8(src, dest, wxc_work->send_unit);
            }
        }
        else
        {
            MI_CpuCopy8(src, dest, wxc_work->send_unit);
        }
    }
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
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    /* WPB will operate differently depending whether one is communicating with the parent as a child or communicating with the child as a parent */
    if (WXC_IsParentMode())
    {
        int     send_size = 0;
        int     send_buf_count;
        u8     *send_buf = packet->buffer;

        /* Set request number of own station */
        if (pass_ctrl->recv_done == TRUE)
        {
            if (pass_ctrl->reset_done)
            {
                pass_ctrl->send_buf.req_count = REQUEST_DONE;
            }
            else
            {
                pass_ctrl->send_buf.req_count = REQUEST_BYE;
            }
        }
        else
        {
            pass_ctrl->send_buf.req_count = pass_data_get_next_count(protocol);
            if (pass_ctrl->send_buf.req_count == REQUEST_DONE)
            {
                pass_ctrl->recv_done = TRUE;
            }
        }

        /* Set data to the other party's request */
        if (pass_ctrl->send_done)
        {
            send_buf_count = REQUEST_NONE;
        }
        else
        {
            send_buf_count = pass_ctrl->recv_buf.req_count;
        }
        pass_DataToBuf(protocol, send_buf_count, &(pass_ctrl->send_buf), &wxc_work->my_pass_data);

        /* Copy from send_buf to send buffer */
        pass_copy_to_buffer(&(pass_ctrl->send_buf), send_buf);

#ifdef DEBUG
        OS_TPrintf("parent send->%x req->%x\n", send_buf_count, pass_ctrl->send_buf.req_count);
#endif
        send_size = PASS_PACKET_SIZE;

        /* Specify packet size */
        packet->length = (u16)MATH_ROUNDUP(4 + wxc_work->send_unit, 2);

#ifdef DEBUG
        PrintPassBuffer(packet->buffer, "parent send");
#endif
    }
    else
    {
        int     peer_request;

        /* Set request number of own station */
        if (pass_ctrl->recv_done == TRUE)
        {
            if (pass_ctrl->reset_done)
            {
                pass_ctrl->send_buf.req_count = REQUEST_DONE;
            }
            else
            {
                pass_ctrl->send_buf.req_count = REQUEST_BYE;
            }
        }
        else
        {
            pass_ctrl->send_buf.req_count = pass_data_get_next_count(protocol);
            if (pass_ctrl->send_buf.req_count == REQUEST_DONE)
            {
                pass_ctrl->recv_done = TRUE;
            }
        }

        /* Set data matching the other party's request number */
        peer_request = (int)(pass_ctrl->recv_buf.req_count);
        pass_DataToBuf(protocol, peer_request, &(pass_ctrl->send_buf), &wxc_work->my_pass_data);

        /* Copy from send_buf to send buffer */
        pass_copy_to_buffer(&(pass_ctrl->send_buf), packet->buffer);

#ifdef DEBUG
        OS_TPrintf("child send->%x req->%x\n", peer_request, pass_ctrl.send_buf.req_count);
#endif
        /* Specify packet size */
        packet->length = (u16)MATH_ROUNDUP(4 + wxc_work->send_unit, 2);
#ifdef DEBUG
        PrintPassBuffer(packet->buffer, "child send");
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_copy_to_structure

  Description:  Copies from the WM receive buffer to the PassBuffer structure.

  Arguments:    buf: WM receive buffer
                pb: PassBuffer structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_copy_to_structure(u8 *buf, PassBuffer * pb)
{
    int     i;

    pb->req_count = (u16)(((u32)(*buf++)) << 8);        /* HI */
    pb->req_count += (u16)(*buf++);    /* LO */

    pb->res_count = (u16)(((u32)(*buf++)) << 8);        /* HI */
    pb->res_count += (u16)(*buf++);    /* LO */

    for (i = 0; i < PASS_BUFFER_SIZE; i++)
    {
        pb->buf[i] = *buf++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         pass_data_set_recv_bitmap

  Description:  Sets the receive history bitmap.

  Arguments:    aid: AID (because receive buffer is managed for each AID)
                seq_no: Sequence number

  Returns:      BOOL: FALSE/already checked.
 *---------------------------------------------------------------------------*/
static BOOL pass_data_set_recv_bitmap(WXCProtocolContext * protocol, int seq_no)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wxc_work->pass_ctrl;

    if (seq_no < pass_ctrl->recv_bitmap_index)
    {
        return FALSE;
    }
    if (seq_no >= pass_ctrl->recv_bitmap_index + 32)
    {
        return FALSE;
    }
    if (pass_ctrl->recv_bitmap & 1 << seq_no % 32)
    {
        return FALSE;
    }
    pass_ctrl->recv_bitmap |= 1 << seq_no % 32;
    while (pass_ctrl->recv_bitmap & 1 << pass_ctrl->recv_bitmap_index % 32)
    {
        pass_ctrl->recv_bitmap &= ~(1 << pass_ctrl->recv_bitmap_index++ % 32);
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         pass_BufToData

  Description:  Copies data from the PassBuffer structure to the user receive buffer.

  Arguments:    pb: PassBuffer structure
                buf: User receive buffer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void pass_BufToData(WXCProtocolContext * protocol, PassBuffer * pb, PassCtrl * pctrl)
{
    WXCImplWorkWpb *wxc_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    int     res_count;
    u8     *src, *dest;

    res_count = (int)pb->res_count;
    src = pb->buf;

    if (pctrl->user_recv_buffer == NULL)
    {
        return;
    }

    dest = pctrl->user_recv_buffer + (res_count * PASS_BUFFER_SIZE);
    if (res_count == wxc_work->my_pass_data.total_count - 1)
    {
        int     mod = wxc_work->my_pass_data.size % wxc_work->recv_unit;
        if (mod)
        {
            MI_CpuCopy8(src, dest, (u32)mod);
        }
        else
        {
            MI_CpuCopy8(src, dest, wxc_work->recv_unit);
        }
    }
    else
    {
        MI_CpuCopy8(src, dest, wxc_work->recv_unit);
    }
}

static void disconnect_callback(PassCtrl * pass_ctrl)
{
    if (!(pass_ctrl->reset_done == FALSE && pass_ctrl->recv_done))
    {
        pass_ctrl->reset_done = FALSE;
        pass_ctrl->recv_done = TRUE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;
    PassCtrl *pass_ctrl = &wpb_work->pass_ctrl;

    /* WPB will operate differently depending whether one is communicating with the parent as a child or communicating with the child as a parent */
    if (WXC_IsParentMode())
    {
        if (packet->buffer == NULL || packet->length == 0)
        {
            return FALSE;              /* Continue communication */
        }

        PrintPassBuffer(packet->buffer, "parent recv");

        /* First, copy data from receive buffer to pass_recv_buf */
        pass_copy_to_structure(((u8 *)packet->buffer), &(pass_ctrl->recv_buf));
        if (pass_ctrl->recv_buf.req_count == REQUEST_BYE)
        {
#ifdef DEBUG
            OS_TPrintf("parent : get REQUEST_BYE\n");
#endif
            wpb_work->executing = FALSE;        /* Cut off communication */
            return TRUE;
        }
        if (pass_ctrl->reset_done == TRUE)
        {
            if (pass_ctrl->recv_done == FALSE)
            {
#ifdef DEBUG
                OS_TPrintf("parent recv->%x\n", pass_ctrl->recv_buf.res_count);
#endif
                if (pass_ctrl->recv_buf.res_count < wpb_work->my_pass_data.total_count)
                {
                    /* Insert a check in the receive history */
                    if (TRUE == pass_data_set_recv_bitmap(protocol, pass_ctrl->recv_buf.res_count))
                    {
                        /* Save the received data */
                        pass_BufToData(protocol, &(pass_ctrl->recv_buf), pass_ctrl);
                    }
                }
            }
            else
            {
                if (pass_ctrl->recv_buf.req_count == REQUEST_DONE)
                {
                    pass_ctrl->send_done = TRUE;
                }
                if (pass_ctrl->send_done == TRUE)
                {

                    /* When data transmission ends this side as well: */
                    if (pass_ctrl->hardware_buffer_count < (HARDWARE_BUFFER_DUMMY_COUNT * 2))
                    {
                        /* Wait until execution arrives here four times */
                        pass_ctrl->hardware_buffer_count++;
                        return FALSE;  /* Continue communication */
                    }

                    pass_ctrl->reset_done = FALSE;

                    /* End */
                    return TRUE;
                }
            }
        }
        return FALSE;                  /* Continue communication */
    }
    else
    {
        PrintPassBuffer(packet->buffer, "child recv");

        /* Protocol for communicating with the child device as a parent */
        if (packet->buffer == NULL || packet->length == 0)
        {
            return FALSE;              /* Continue communication */
        }

#ifdef DEBUG
        OS_TPrintf("child recv->%x\n", pass_ctrl->recv_buf.res_count);
#endif

        /* First, copy data from receive buffer to pass_recv_buf */
        pass_copy_to_structure(((u8 *)packet->buffer), &(pass_ctrl->recv_buf));
        if (pass_ctrl->recv_buf.req_count == REQUEST_BYE)
        {
#ifdef DEBUG
            OS_TPrintf("child: get REQUEST_BYE\n");
#endif
            wpb_work->executing = FALSE;        /* Cut off communication */
            return TRUE;
        }
        if (pass_ctrl->reset_done == TRUE)
        {
            if (packet->length < PASS_PACKET_SIZE)
            {
                return FALSE;          /* Continue communication */
            }
            if (pass_ctrl->recv_done == FALSE)
            {
                if (pass_ctrl->recv_buf.res_count < wpb_work->my_pass_data.total_count)
                {
                    /* Insert a check in the receive history */
                    if (TRUE == pass_data_set_recv_bitmap(protocol, pass_ctrl->recv_buf.res_count))
                    {
                        pass_BufToData(protocol, &(pass_ctrl->recv_buf), pass_ctrl);
                    }
                }
            }
            else
            {
                if (pass_ctrl->recv_buf.req_count == REQUEST_DONE)
                {
                    pass_ctrl->send_done = TRUE;
                }
                if (pass_ctrl->send_done == TRUE)
                {

                    /* When data transmission ends this station as well: */
                    if (pass_ctrl->hardware_buffer_count < HARDWARE_BUFFER_DUMMY_COUNT)
                    {
                        /* Wait until execution arrives here twice
                           pass_ctrl->hardware_buffer_count++;
                           return FALSE;        /* Continue communication */
                    }

                    pass_ctrl->reset_done = FALSE;

                    /* End */
                    return TRUE;
                }
            }
        }
        return FALSE;                  /* Continue communication */
    }
    return FALSE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* 4 is the WPB packet header length.
     * Apply it to this protocol (WPB is 512-byte)
     * Be careful to not rewrite the ParentParam value!!
     */
    wpb_work->send_unit = (u16)(send_max - 4);  /* Should be 512 - 4 = 508 */
    wpb_work->recv_unit = (u16)(recv_max - 4);  /* Should be 512 - 4 = 508 */

    /* Initialize the data receive management information */
    MI_CpuClear32(wpb_work->recv_bitmap_buf, sizeof(wpb_work->recv_bitmap_buf));

    /* Initialize WPB relationship */
    wpb_work->my_pass_data.total_count = 0;
    wpb_work->my_pass_data.size = 0;
    wpb_work->my_pass_data.user_send_data = NULL;

    wpb_work->executing = TRUE;
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
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    /* Initialize data buffer */
    pass_ResetData(WXC_GetCurrentBlockImpl(protocol)->impl_work);

    /* Also connect the common protocol members' send and receive buffers */
    protocol->send.buffer = (void *)send_buf;
    protocol->send.length = (u16)send_size;
    protocol->send.checksum = MATH_CalcChecksum8(send_buf, send_size);
    protocol->recv.buffer = recv_buf;
    protocol->recv.buffer_max = (u16)recv_max;
    MI_CpuClear32(wpb_work->recv_bitmap_buf, sizeof(wpb_work->recv_bitmap_buf));

    /* Configure WPB side buffer */
    wpb_work->my_pass_data.total_count =
        (int)(recv_max / PASS_BUFFER_SIZE) + ((recv_max % PASS_BUFFER_SIZE) ? 1 : 0);
    wpb_work->my_pass_data.size = (int)send_size;
    wpb_work->my_pass_data.user_send_data = send_buf;

    wpb_work->pass_ctrl.user_recv_buffer = recv_buf;

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_IsExecuting

  Description:  Checks whether data is currently being exchanged.

  Arguments:    None.

  Returns:      Return TRUE if data is currently being exchanged.
 *---------------------------------------------------------------------------*/
BOOL WXCi_IsExecuting(WXCProtocolContext * protocol)
{
    WXCImplWorkWpb *wpb_work = WXC_GetCurrentBlockImpl(protocol)->impl_work;

    return wpb_work->executing;
}
