/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WFS - libraries
  File:     wfs_client.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/


#include <nitro/wfs/client.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WFSi_NotifyEvent

  Description:  Notifies for events within the WFS server.

  Arguments:    context: The WFSClientContext structure.
                event: The event type for the notification.
                argument: The event argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
inline static void WFSi_NotifyEvent(WFSClientContext *context,
                                    WFSEventType event, void *argument)
{
    if (context->callback)
    {
        context->callback(context, event, argument);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ReallocBitmap

  Description:  Reallocates a bitmap capable of receiving the specified size.

  Arguments:    context: The WFSClientContext structure.
                length: The anticipated file size to be received.
                                 Specifying a negative value will reallocate with the current value.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ReallocBitmap(WFSClientContext *context, int length)
{
    if (length < 0)
    {
        length = (int)context->max_file_size;
        context->max_file_size = 0;
    }
    if (context->max_file_size < length)
    {
        const int packet = WBT_GetParentPacketLength(context->wbt);
        const u32 newsize = WBT_PACKET_BITMAP_SIZE((u32)length, packet);
        context->max_file_size = (u32)length;
        MI_CallFree(context->allocator, context->recv_pkt_bmp_buf);
        context->recv_pkt_bmp_buf = (u32 *)MI_CallAlloc(context->allocator,
                                    sizeof(u32) * newsize, sizeof(u32));
        if (context->recv_pkt_bmp_buf == NULL)
        {
            OS_TPanic("cannot allocate bitmap buffer %d BYTEs!", sizeof(u32) * newsize);
        }
        context->recv_buf_packet_bmp_table.packet_bitmap[0] = context->recv_pkt_bmp_buf;
    }
    SDK_ASSERT(context->recv_pkt_bmp_buf);
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ReceiveTableSequence

  Description:  The sequence that receives the ROM file table.
                Can only be run once immediately after connecting to the server.

  Arguments:    userdata: The WFSClientContext structure.
                callback: The WBT completion callback argument.
                                 Specify NULL for the call when at the start of the sequence.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ReceiveTableSequence(void *userdata, WBTCommand *callback)
{
    WFSClientContext * const    context = (WFSClientContext *)userdata;
    WBTContext * const          wbt = context->wbt;

    if ((callback == NULL) || (callback->result == WBT_RESULT_SUCCESS))
    {
        BOOL    post = FALSE;
        const int   bitmap = 0x0001;
        /* Sequence start => issue WBT-SYNC() and synchronize the packet size */
        if (callback == NULL)
        {
            WFS_DEBUG_OUTPUT(("WBT-SYNC():post"));
            post = WBT_PostCommandSYNC(wbt, bitmap, WFSi_ReceiveTableSequence);
        }
        /* WBT-SYNC completion => issue WBT-INFO(0) and get the size of the ROM file table */
        else if (callback->event == WBT_CMD_RES_SYNC)
        {
            WFS_DEBUG_OUTPUT(("WBT-SYNC():done [server = %d, client = %d]",
                             callback->sync.peer_packet_size + WBT_PACKET_SIZE_MIN,
                             callback->sync.my_packet_size + WBT_PACKET_SIZE_MIN));
            WFS_DEBUG_OUTPUT(("WBT-INFO(0):post"));
            post = WBT_PostCommandINFO(wbt, bitmap, WFSi_ReceiveTableSequence,
                                       0, &context->block_info_table);
        }
        /* WBT-INFO(0) completion => issue WBT-GET(0x20000) and get the size of the ROM file table */
        else if (callback->event == WBT_CMD_RES_GET_BLOCKINFO)
        {
            const int length = context->block_info_table.block_info[0]->block_size;
            WFS_DEBUG_OUTPUT(("WBT-INFO(0):done [table-length = %d]", length));
            context->table->length = (u32)length;
            context->table->buffer = (u8 *)MI_CallAlloc(context->allocator, (u32)length, 1);
            if (context->table->buffer == NULL)
            {
                OS_TPanic("cannot allocate FAT buffer %d BYTEs!", length);
            }
            WFSi_ReallocBitmap(context, length);
            context->recv_buf_table.recv_buf[0] = context->table->buffer;
            WFS_DEBUG_OUTPUT(("WBT-GET(0x%08X):post", WFS_TABLE_BLOCK_INDEX));
            post = WBT_PostCommandGET(wbt, bitmap, WFSi_ReceiveTableSequence,
                                      WFS_TABLE_BLOCK_INDEX, context->table->length,
                                      &context->recv_buf_table,
                                      &context->recv_buf_packet_bmp_table);
        }
        /* WBT-GET(0x20000) completion => mount preparations complete event notification */
        else if (callback->event == WBT_CMD_RES_GET_BLOCK)
        {
            WFS_DEBUG_OUTPUT(("WBT-GET(0x%08X):done [ready for mount]", WFS_TABLE_BLOCK_INDEX));
            WFS_ParseTable(context->table);
            context->fat_ready = TRUE;
            WFSi_NotifyEvent(context, WFS_EVENT_CLIENT_READY, NULL);
            post = TRUE;    /* As a matter of convenience */
        }
        /* WBT command issue failure (insufficient command queue resulting from an internal WFS problem) */
        if (!post)
        {
            WFS_DEBUG_OUTPUT(("internal-error (failed to post WBT command)"));
        }
    }
    /* Some kind of internal error */
     else
    {
        /* Cancelled during the WFS start up (no need to do anything special here) */
        if (callback->event == WBT_CMD_CANCEL)
        {
        }
        /* An unexpected WBT error (a state management problem occurred as a result of an internal WFS problem) */
        else
        {
            WFS_DEBUG_OUTPUT(("internal-error (unexpected WBT result)"));
            WFS_DEBUG_OUTPUT(("  command = %d", callback->command));
            WFS_DEBUG_OUTPUT(("  event   = %d", callback->event));
            WFS_DEBUG_OUTPUT(("  result  = %d", callback->result));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ReadRomSequence

  Description:  The sequence that receives the server-side ROM image.
                Run every time the FS_ReadFile function is called from the client.

  Arguments:    userdata: The WFSClientContext structure.
                callback: The WBT completion callback argument.
                                 Specify NULL for the call when at the start of the sequence.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ReadRomSequence(void *userdata, WBTCommand *callback)
{
    WFSClientContext * const    context = (WFSClientContext *)userdata;
    WBTContext * const          wbt = context->wbt;

    if ((callback == NULL) || (callback->result == WBT_RESULT_SUCCESS))
    {
        BOOL    post = FALSE;
        const int   bitmap = 0x0001;
        /* Sequence start => issue WBT-MSG(LOCK) and lock the transfer range */
        if (callback == NULL)
        {
            WFS_DEBUG_OUTPUT(("WBT-MSG(LOCK):post"));
            post = WFS_SendMessageLOCK_REQ(wbt, WFSi_ReadRomSequence, bitmap,
                                           context->request_region.offset + context->table->origin,
                                           context->request_region.length);
        }
        /* WBT-MSG() issued (wait for the server response; no need to do anything here) */
        else if (callback->event == WBT_CMD_RES_USER_DATA)
        {
            /* waiting for response from server... */
            post = TRUE;    /* As a matter of convenience */
        }
        else
        {
            const WFSMessageFormat *const msg = (const WFSMessageFormat *)callback->user_data.data;
            /* WBT-MSG(LOCK) response received */
            if ((callback->event == WBT_CMD_REQ_USER_DATA) &&
                (msg->type == WFS_MSG_LOCK_ACK))
            {
                BOOL    accepted = (BOOL)MI_LEToH32(msg->arg2);
                /* Denial => reissue WBT-SYNC() and synchronize the packet size */
                if (!accepted)
                {
                    WFS_DEBUG_OUTPUT(("WBT-MSG(LOCK):failed [packet-length renewal]"));
                    WFS_DEBUG_OUTPUT(("WBT-SYNC():post"));
                    post = WBT_PostCommandSYNC(wbt, bitmap, WFSi_ReadRomSequence);
                }
                /* Permission => reissue WBT-GET(id) and receive blocks */
                else
                {
                    u32     id = MI_LEToH32(msg->arg1);
                    WFS_DEBUG_OUTPUT(("WBT-MSG(LOCK):done [lock-id = 0x%08X]", id));
                    context->block_id = id;
                    context->recv_buf_table.recv_buf[0] = context->request_buffer;
                    WFS_DEBUG_OUTPUT(("WBT-GET(0x%08X):post", id));
                    post = WBT_PostCommandGET(wbt, bitmap, WFSi_ReadRomSequence,
                                              context->block_id, context->request_region.length,
                                              &context->recv_buf_table,
                                              &context->recv_buf_packet_bmp_table);
                }
            }
            /* WBT-SYNC() completion => retry the sequence (will only be a single level recursive call) */
            else if (callback->event == WBT_CMD_RES_SYNC)
            {
                WFS_DEBUG_OUTPUT(("WBT-SYNC():done [server = %d, client = %d]",
                                 callback->sync.peer_packet_size + WBT_PACKET_SIZE_MIN,
                                 callback->sync.my_packet_size + WBT_PACKET_SIZE_MIN));
                WFSi_ReallocBitmap(context, -1);
                WFSi_ReadRomSequence(context, NULL);
                post = TRUE;    /* As a matter of convenience */
            }
            /* WBT-GET(id) completion => issue WBT-MSG(UNLOCK, id) and deallocate the transfer range */
            else if (callback->event == WBT_CMD_RES_GET_BLOCK)
            {
                u32     id = context->block_id;
                WFS_DEBUG_OUTPUT(("WBT-GET(0x%08X):done", id));
                WFS_DEBUG_OUTPUT(("WBT-MSG(UNLOCK,0x%08X):post", id));
                post = WFS_SendMessageUNLOCK_REQ(wbt, WFSi_ReadRomSequence, bitmap, id);
            }
            /* WBT-MSG(UNLOCK, id) response received => read complete event notification */
            else if ((callback->event == WBT_CMD_REQ_USER_DATA) &&
                (msg->type == WFS_MSG_UNLOCK_ACK))
            {
                WFS_DEBUG_OUTPUT(("WBT-MSG(UNLOCK,0x%08X):done [read-operation completed]", context->block_id));
                context->request_buffer = NULL;
                {
                    WFSRequestClientReadDoneCallback callback = context->request_callback;
                    void   *argument = context->request_argument;
                    context->request_callback = NULL;
                    context->request_argument = NULL;
                    if (callback)
                    {
                        (*callback)(context, TRUE, argument);
                    }
                }
                post = TRUE;    /* As a matter of convenience */
            }
        }
        /* WBT command issue failure (insufficient command queue resulting from an internal WFS problem) */
        if (!post)
        {
            WFS_DEBUG_OUTPUT(("internal-error (failed to post WBT command)"));
        }
    }
    /* Some kind of internal error */
     else
    {
        /* Cancelled during the WFS read process  (no need to do anything special here) */
        if (callback->event == WBT_CMD_CANCEL)
        {
        }
        /* An unexpected WBT error (a state management problem occurred as a result of an internal WFS problem) */
        else
        {
            WFS_DEBUG_OUTPUT(("internal-error (unexpected WBT result)"));
            WFS_DEBUG_OUTPUT(("  command = %d", callback->command));
            WFS_DEBUG_OUTPUT(("  event   = %d", callback->event));
            WFS_DEBUG_OUTPUT(("  result  = %d", callback->result));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_WBTSystemCallback

  Description:  The client-side WBT system callback.

  Arguments:    userdata: The WFSServerContext structure.
                callback: The WBT event argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_WBTSystemCallback(void *userdata, WBTCommand *callback)
{
    WFSClientContext * const context = (WFSClientContext *)userdata;
    /* The response from the server is redirected to WFSi_ReadRomSequence */
    if ((callback->event == WBT_CMD_REQ_USER_DATA) &&
        (context->request_buffer))
    {
        WFSi_ReadRomSequence(context, callback);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallClientConnectHook

  Description:  Connection notification on the client side.

  Arguments:    context: The WFSClientContext structure.
                peer: Information for the connected communication peer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallClientConnectHook(WFSClientContext *context, const WFSPeerInfo *peer)
{
    (void)context;
    (void)peer;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallClientDisconnectHook

  Description:  Disconnect notification on the client side.

  Arguments:    context: The WFSClientContext structure.
                peer: Information for the disconnected communication peer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallClientDisconnectHook(WFSClientContext *context, const WFSPeerInfo *peer)
{
    (void)context;
    (void)peer;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallClientPacketSendHook

  Description:  Notification of timing when it is possible to send packets on the client side.

  Arguments:    context: The WFSClientContext structure.
                packet: Send packet settings

  Returns:      The actual packet size.
 *---------------------------------------------------------------------------*/
void WFS_CallClientPacketSendHook(WFSClientContext *context, WFSPacketBuffer *packet)
{
    packet->length = WBT_CallPacketSendHook(context->wbt, packet->buffer, packet->length, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallClientPacketRecvHook

  Description:  Notification of timing when it is possible to receive packets on the client side.

  Arguments:    context: The WFSClientContext structure
                packet: Sender packet information

  Returns:      The actual packet size.
 *---------------------------------------------------------------------------*/
void WFS_CallClientPacketRecvHook(WFSClientContext *context, const WFSPacketBuffer *packet)
{
    int aid = (int)MATH_CTZ((u32)packet->bitmap);
    WBT_CallPacketRecvHook(context->wbt, aid, packet->buffer, packet->length);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_InitClient

  Description:  Initializes the WFS client context.

  Arguments:    context: The WFSClientContext structure
                userdata: Any user-defined value associated with the context
                callback: The system event notification callback
                                 Specify NULL if not needed.
                allocator: The allocator used internally

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_InitClient(WFSClientContext *context,
                    void *userdata, WFSEventCallback callback,
                    MIAllocator *allocator)
{
    int     i;
    context->userdata = userdata;
    context->callback = callback;
    context->allocator = allocator;
    context->fat_ready = FALSE;
    /* Initialize WBT variables */
    for (i = 0; i < WBT_NUM_OF_AID; ++i)
    {
        context->block_info_table.block_info[i] = &context->block_info[i];
        context->recv_buf_table.recv_buf[i] = NULL;
        context->recv_buf_packet_bmp_table.packet_bitmap[i] = NULL;
    }
    context->recv_pkt_bmp_buf = NULL;
    context->max_file_size = 0;
    context->block_id = 0;
    context->request_buffer = NULL;
    context->table->length = 0;
    context->table->buffer = NULL;
    context->unmount_callback = NULL;
    /* Initialize WBT */
    WBT_InitContext(context->wbt, context, WFSi_WBTSystemCallback);
    WBT_AddCommandPool(context->wbt, context->wbt_list,
                       sizeof(context->wbt_list) / sizeof(*context->wbt_list));
}

/*---------------------------------------------------------------------------*
  Name:         WFS_EndClient

  Description:  Deallocates the WFS client context.

  Arguments:    context: The WFSClientContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_EndClient(WFSClientContext *context)
{
    MI_CallFree(context->allocator, context->recv_pkt_bmp_buf);
    WBT_ResetContext(context->wbt, NULL);
    if (context->table->buffer)
    {
        MI_CallFree(context->allocator, context->table->buffer);
        context->table->buffer = NULL;
    }
    if (context->request_callback)
    {
        (*context->request_callback)(context->request_argument, FALSE, context->request_argument);
    }
    if (context->unmount_callback)
    {
        (*context->unmount_callback)(context);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_StartClient

  Description:  Starts the WFS client context communication.

  Arguments:    context: The WFSClientContext structure
                peer: The local host's connection information.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_StartClient(WFSClientContext *context, const WFSPeerInfo *peer)
{
    WBT_StartChild(context->wbt, peer->aid);
    WFSi_ReceiveTableSequence(context, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_RequestClientRead

  Description:  Begins a ROM image read request from the server.
                When complete, a WFS_EVENT_CLIENT_READ notification occurs.

  Arguments:    context: The WFSClientContext structure
                buffer: Memory where the load data is stored
                offset: Starting position for the device load
                length: The load size.
                callback: Load completion callback
                                 NULL if not necessary.
                arg: Argument passed to the load completion callback
  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_RequestClientRead(WFSClientContext *context, void *buffer, u32 offset,
                           u32 length, WFSRequestClientReadDoneCallback callback,
                           void *arg)
{
    if (context->fat_ready)
    {
        context->request_buffer = buffer;
        context->request_region.offset = offset;
        context->request_region.length = length;
        context->request_callback = callback;
        context->request_argument = arg;
        WFSi_ReallocBitmap(context, (int)length);
        WFSi_ReadRomSequence(context, NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetClientReadProgress

  Description:  Gets the progress of the ROM image read request.

  Arguments:    context: The WFSClientContext structure
                current: Variable that gets the number of received packets
                total: Variable that gets the total expected number of packets

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_GetClientReadProgress(WFSClientContext *context,int *current, int *total)
{
    WBT_GetDownloadProgress(context->wbt, context->block_id, 0, current, total);
}


/*---------------------------------------------------------------------------*
  $Log: wfs_client.c,v $
  Revision 1.2  2007/06/11 06:39:24  yosizaki
  Changed WFS_RequestClientRead().

  Revision 1.1  2007/04/13 04:12:37  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
