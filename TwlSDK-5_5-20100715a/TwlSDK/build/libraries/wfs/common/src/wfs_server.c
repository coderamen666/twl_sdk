/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WFS - libraries
  File:     wfs_server.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/


#include <nitro/wfs/server.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WFSi_NotifySegmentEvent

  Description:  Notifies the caller of a WFS_EVENT_SERVER_SEGMENT_REQUEST event.

  Arguments:    context:          The WFSServerContext structure.
                argument         The event argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_NotifySegmentEvent(WFSServerContext *context, void *argument)
{
    if (context->thread_work)
    {
        (*context->thread_hook)(context->thread_work, argument);
    }
    else if (context->callback)
    {
        (*context->callback)(context, WFS_EVENT_SERVER_SEGMENT_REQUEST, argument);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_WBTCallback

  Description:  Server-side WBT event notification.

  Arguments:    userdata         The WFSServerContext structure.
                uc               The WBT event argument.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_WBTCallback(void *userdata, WBTCommand *uc)
{
    WFSServerContext *work = (WFSServerContext *)userdata;
    const int aid = (int)MATH_CTZ(uc->peer_bmp);

    switch (uc->command)
    {

    default:
        WFS_DEBUG_OUTPUT(("invalid WBT callback (command = %d)", uc->command));
        break;

    case WBT_CMD_REQ_USER_DATA:
        /* Confirm whether the message has been sent to everyone */
        if (!uc->target_bmp)
        {
            work->msg_busy = FALSE;
        }
        break;

    case WBT_CMD_SYSTEM_CALLBACK:
        /* Indication */

        switch (uc->event)
        {

        default:
            WFS_DEBUG_OUTPUT(("unknown WBT system callback (event = %d)", uc->event));
            break;

        case WBT_CMD_REQ_SYNC:
        case WBT_CMD_REQ_GET_BLOCK_DONE:
            break;

        case WBT_CMD_REQ_USER_DATA:
            /* Message receipt notification */
            {
                /* Save the received message */
                const WFSMessageFormat *const msg = (const WFSMessageFormat *)uc->user_data.data;
                work->recv_msg[aid] = *msg;

                switch (msg->type)
                {

                case WFS_MSG_LOCK_REQ:
                    /* Read lock request */
                    {
                        const u32 offset = MI_LEToH32(msg->arg1);
                        const u32 length = MI_LEToH32_BITFIELD(24, msg->arg2);
                        const int packet = (int)((msg->packet_hi << 8) | msg->packet_lo);

                        WFS_DEBUG_OUTPUT(("WBT-MSG(LOCK):recv [offset=0x%08X, length=0x%08X] (AID=%d, packet=%d)",
                                         offset, length, aid, packet));
                        /* Deny response when the packet size is changing */
                        if (work->is_changing ||
                            (packet != WBT_GetParentPacketLength(work->wbt) + WBT_PACKET_SIZE_MIN))
                        {
                            work->is_changing = TRUE;
                            work->deny_bitmap |= (1 << aid);
                        }
                        /* If there is a match with an existing lock region, reference it */
                        else
                        {
                            WFSLockInfo *file = NULL;
                            int     index;
                            for (index = 0; (1 << index) <= work->use_bitmap; ++index)
                            {
                                if(((1 << index) & work->use_bitmap) != 0)
                                {
                                    if ((work->list[index].offset == offset) &&
                                        (work->list[index].length == length))
                                    {
                                        file = &work->list[index];
                                        ++file->ref;
                                        break;
                                    }

                                }

                            }
                            /* If a new lock region, allocate from the available list */
                            if (!file)
                            {
                                index = (int)MATH_CTZ((u32)~work->use_bitmap);
                                if (index < WFS_LOCK_HANDLE_MAX)
                                {
                                    PLATFORM_ENTER_CRITICALSECTION();
                                    work->use_bitmap |= (1 << index);
                                    file = &work->list[index];
                                    file->ref = 1;
                                    file->offset = offset;
                                    file->length = length;
                                    /* Transfer to the registered list as it is a new file */
                                    (void)WBT_RegisterBlockInfo(work->wbt, &file->info,
                                                                (u32)(WFS_LOCKED_BLOCK_INDEX + index),
                                                                NULL, NULL, (int)file->length);
                                    file->ack_seq = 0;
                                    PLATFORM_LEAVE_CRITICALSECTION();
                                    /* Request to read just the header in advance */
                                    {
                                        WFSSegmentBuffer    segment[1];
                                        segment->offset = file->offset;
                                        segment->length = (u32)WBT_GetParentPacketLength(work->wbt);
                                        segment->buffer = NULL;
                                        WFSi_NotifySegmentEvent(work, segment);
                                    }
                                }
                                else
                                {
                                    OS_TPanic("internal-error (no available lock handles)");
                                }
                            }
                            work->ack_bitmap |= (1 << aid);
                            work->recv_msg[aid].arg1 = MI_HToLE32((u32)(WFS_LOCKED_BLOCK_INDEX + index));
                        }
                        work->busy_bitmap |= (1 << aid);
                    }
                    break;

                case WFS_MSG_UNLOCK_REQ:
                    /* CLOSEFILE request */
                    {
                        PLATFORM_ENTER_CRITICALSECTION();
                        u32     id = MI_LEToH32(msg->arg1);
                        u32     index = id - WFS_LOCKED_BLOCK_INDEX;
                        if (index < WFS_LOCK_HANDLE_MAX)
                        {
                            WFSLockInfo *file = &work->list[index];
                            /* Deallocate process when all references are gone */
                            if (--file->ref <= 0)
                            {
                                (void)WBT_UnregisterBlockInfo(work->wbt, id);
                                work->use_bitmap &= ~(1 << index);
                            }
                        }
                        work->ack_bitmap |= (1 << aid);
                        PLATFORM_LEAVE_CRITICALSECTION();
                        WFS_DEBUG_OUTPUT(("WBT-MSG(UNLOCK):recv [id=0x%08X] (AID=%d)", id, aid));
                    }
                    break;

                }
            }
            break;

        case WBT_CMD_PREPARE_SEND_DATA:
            /* GETBLOCK buffer preparation request */
            {
                WBTPrepareSendDataCallback *const p_prep = &uc->prepare_send_data;
                u32     id = p_prep->block_id;
                p_prep->data_ptr = NULL;
                /* Response when a valid file */
                id -= WFS_LOCKED_BLOCK_INDEX;
                if (id < WFS_LOCK_HANDLE_MAX)
                {
                    WFSLockInfo *file = &work->list[id];
                    /* Current response to the previous request, next response for current request */
                    WFSSegmentBuffer    segment[1];
                    const u32 length = p_prep->own_packet_size;
                    const u32 current = file->ack_seq;
                    const u32 next = (u32)p_prep->block_seq_no;
                    file->ack_seq = next;
                    /* Preparation for next time */
                    segment->offset = file->offset + length * next;
                    segment->length = MATH_MIN(length, file->length - length * next);
                    segment->buffer = NULL;
                    WFSi_NotifySegmentEvent(work, segment);
                    /* Current request */
                    segment->offset = file->offset + length * current;
                    segment->length = MATH_MIN(length, file->length - length * current);
                    segment->buffer = work->cache_hit_buf;
                    WFSi_NotifySegmentEvent(work, segment);
                    if (segment->buffer != NULL)
                    {
                        p_prep->block_seq_no = (s32)current;
                        p_prep->data_ptr = segment->buffer;
                    }
                }
            }
            break;

        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallServerConnectHook

  Description:  Server-side connection notification.

  Arguments:    context:          The WFSServerContext structure.
                peer:             Information for the connected client.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallServerConnectHook(WFSServerContext *context, const WFSPeerInfo *peer)
{
    /*
     * Because it is not necessarily true that all clients are using WFS, we ignore connection notifications here and instead treat PacketRecv as a connection notification
     * 
     */
    (void)context;
    (void)peer;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallServerDisconnectHook

  Description:  Disconnect notification on the server side.

  Arguments:    context:          The WFSServerContext structure.
                peer:             Information for the disconnected client.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallServerDisconnectHook(WFSServerContext *context, const WFSPeerInfo *peer)
{
    const int bit = (1 << peer->aid);
    context->all_bitmap &= ~bit;
    (void)WBT_CancelCommand(context->wbt, bit);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallServerPacketSendHook

  Description:  Notification of timing when it is possible to send packets on the server side.

  Arguments:    context:          The WFSServerContext structure.
                packet:           Send packet settings.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallServerPacketSendHook(WFSServerContext *context, WFSPacketBuffer *packet)
{
    /* Restart the non-response process when it is possible to send a message */
    if (!context->msg_busy)
    {
        /* Show the current valid communications state in each type of bitmap */
        context->ack_bitmap &= context->all_bitmap;
        context->sync_bitmap &= context->all_bitmap;
        context->busy_bitmap &= context->all_bitmap;
        context->deny_bitmap &= context->all_bitmap;
        /* Wait to change packet size until all processes being received are complete */
        if (context->is_changing && !context->use_bitmap)
        {
            /* Change packet size */
            context->is_changing = FALSE;
            (void)WBT_SetPacketLength(context->wbt, context->new_packet, WBT_PACKET_SIZE_MIN);
            /* Overall denial response for requests with the old size */
            if (context->deny_bitmap)
            {
                WFS_DEBUG_OUTPUT(("WBT-MSG(LOCK):deny [packet-length renewal] (BITMAP=%d)", context->deny_bitmap));
                (void)WFS_SendMessageLOCK_ACK(context->wbt, WFSi_WBTCallback, context->deny_bitmap, 0);
                context->msg_busy = TRUE;
                context->deny_bitmap = 0;
            }
        }
        /* Corresponds as-is to the requests received before the packet size change */
        else if (context->ack_bitmap)
        {
            int     bitmap = context->ack_bitmap;
            WFSMessageFormat *msg = NULL;
            int     i;
            const int sync = context->sync_bitmap;
            const BOOL is_sync = (sync && ((bitmap & sync) == sync));
            /* Respond all at once if there is a uniform child device group that received the synchronous designation */
            if (is_sync)
            {
                bitmap = sync;
            }
            /* Otherwise, respond as normal */
            else
            {
                bitmap &= ~sync;
            }
            /* Search for relevant child devices that can respond */
            for (i = 0;; ++i)
            {
                const int bit = (1 << i);
                if (bit > bitmap)
                {
                    break;
                }
                if ((bit & bitmap) != 0)
                {
                    /* Search for child devices that can respond in order from the lowest aid */
                    if (!msg)
                    {
                        msg = &context->recv_msg[i];
                    }
                    /* Send all at once if same type of response */
                    else if ((msg->type == context->recv_msg[i].type) &&
                             (msg->arg1 == context->recv_msg[i].arg1))
                    {
                    }
                    /* If not, then hold this time for the child device */
                    else
                    {
                        bitmap &= ~bit;
                    }
                }
            }
            /*
             * NOTE:
             *   When an application-side bug occurs such that there are differing request contents even though synchronization has been specified, it is not possible to determine (1) whether some synchronization contents are simply out of synch, (2) and if so, which request should be responded to first in order to recover, or (3) whether there was in fact a fatal difference in the request contents to begin with. Since this cannot be judged, recovery is performed by automatically revoking the synchronization specification while continually issuing a warning.
             *    
             *    
             *    
             *    
             *    
             *    
             *    
             */
            if (is_sync && (bitmap != sync))
            {
                context->sync_bitmap = 0;
                OS_TWarning("[WFS] specified synchronous-access failed! "
                            "(then synchronous-setting was reset)");
            }
            /* Send the response selected this time */
            if (msg)
            {
                switch (msg->type)
                {
                case WFS_MSG_LOCK_REQ:
                    (void)WFS_SendMessageLOCK_ACK(context->wbt, WFSi_WBTCallback, bitmap,
                                                  MI_LEToH32(msg->arg1));
                    break;
                case WFS_MSG_UNLOCK_REQ:
                    (void)WFS_SendMessageUNLOCK_ACK(context->wbt, WFSi_WBTCallback, bitmap,
                                                    MI_LEToH32(msg->arg1));
                    context->busy_bitmap &= ~bitmap;
                    break;
                }
                context->msg_busy = TRUE;
                context->ack_bitmap &= ~bitmap;
            }
        }
    }

    /* Call WBT after updating all the latest statuses */
    packet->length = WBT_CallPacketSendHook(context->wbt, packet->buffer, packet->length, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_CallServerPacketRecvHook

  Description:  Server-side packet receipt notification.

  Arguments:    context:          The WFSServerContext structure.
                packet:           Sender packet information.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_CallServerPacketRecvHook(WFSServerContext *context, const WFSPacketBuffer *packet)
{
    int aid = (int)MATH_CTZ((u32)packet->bitmap);
    const void *buffer = packet->buffer;
    int length = packet->length;
    /*
     * Detect a connection using actual packet receipt from a client to prevent impact on unrelated clients from unneeded port communication from the parent.
     * 
     */
    context->all_bitmap |= (1 << aid);
    WBT_CallPacketRecvHook(context->wbt, aid, buffer, length);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_InitServer

  Description:  Initializes the WFS server context.

  Arguments:    context:          The WFSServerContext structure.
                userdata:         Any user-defined value associated with the context.
                callback:         The system event notification callback.
                                 Specify NULL if not needed.
                allocator:        The allocator used internally.
                packet:           The parent's initial packet size.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_InitServer(WFSServerContext *context,
                    void *userdata, WFSEventCallback callback,
                    MIAllocator *allocator, int packet)
{
    /* Initialize the basic settings */
    MI_CpuClear8(context, sizeof(*context));
    context->userdata = userdata;
    context->callback = callback;
    context->allocator = allocator;

    /* Initialize the internal status */
    context->new_packet = packet;
    context->table->buffer = NULL;
    context->table->length = 0;
    context->sync_bitmap = 0;
    context->ack_bitmap = 0;
    context->msg_busy = FALSE;
    context->all_bitmap = 1;
    context->busy_bitmap = 0;
    context->is_changing = FALSE;
    context->deny_bitmap = 0;
    context->use_bitmap = 0;
    context->thread_work = NULL;
    context->thread_hook = NULL;

    /* Initialize WBT */
    WBT_InitContext(context->wbt, context, WFSi_WBTCallback);
    WBT_AddCommandPool(context->wbt, context->wbt_list,
                       sizeof(context->wbt_list) / sizeof(*context->wbt_list));
    WBT_StartParent(context->wbt, packet, WBT_PACKET_SIZE_MIN);
}

/*---------------------------------------------------------------------------*
  Name:         WFS_EndServer

  Description:  Deallocates the WFS server context.

  Arguments:    context:          The WFSServerContext structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_EndServer(WFSServerContext *context)
{
    if (context->thread_work)
    {
        (*context->thread_hook)(context->thread_work, NULL);
        MI_CallFree(context->allocator, context->thread_work);
        context->thread_work = NULL;
    }
    WBT_ResetContext(context->wbt, NULL);
    if (context->table->buffer)
    {
        MI_CallFree(context->allocator, context->table->buffer);
        context->table->buffer = NULL;
        context->table->length = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_RegisterServerTable

  Description:  Loads the ROM file table from the device and registers it to the server.

  Arguments:    context:          The WFSServerContext structure.
                device:           The device storing the NTR binary.
                fatbase:          The offset within the device wherein the NTR binary is located.
                overlay: The device-internal offset where the NTR binary containing the overlay to be merged is located.
                                 
                                 (If not merging multiple ROM file tables, this value is identical to "fatbase")
                                  

  Returns:      TRUE when the ROM file table is correctly loaded and registered.
 *---------------------------------------------------------------------------*/
BOOL WFS_RegisterServerTable(WFSServerContext *context,
                             MIDevice *device, u32 fatbase, u32 overlay)
{
    BOOL    retval = FALSE;
    /* Multiple file tables cannot be registered. */
    if (context->table->buffer)
    {
        OS_TWarning("table is already registered.\n");
    }
    /* Load the ROM file table from the device */
    else if (WFS_LoadTable(context->table, context->allocator, device, fatbase, overlay))
    {
        /* Register the ROM file table */
        (void)WBT_RegisterBlockInfo(context->wbt, context->table_info,
                                    WFS_TABLE_BLOCK_INDEX, NULL,
                                    context->table->buffer,
                                    (int)context->table->length);
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_GetServerPacketLength

  Description:  Obtain the size of packets sent by the server.

  Arguments:    context:          The WFSServerContext structure.

  Returns:      The currently specified packet size.
 *---------------------------------------------------------------------------*/
int WFS_GetServerPacketLength(const WFSServerContext *context)
{
    return context->new_packet;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_SetServerPacketLength

  Description:  Sets the size of the server send packet.

  Arguments:    context:          The WFSServerContext structure.
                length:           The packet size to be set.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_SetServerPacketLength(WFSServerContext *context, int length)
{
    SDK_ASSERT(length >= WBT_PACKET_SIZE_MIN);
    {
        PLATFORM_ENTER_CRITICALSECTION();
        context->new_packet = length;
        context->is_changing = TRUE;
        PLATFORM_LEAVE_CRITICALSECTION();
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_SetServerSync

  Description:  Sets the clients that synchronously access the server.
                This function achieves efficient transmission rates that utilities unique characteristics of the WBT library; this is done by synchronizing responses to clients that are all clearly guaranteed to access the same files in precisely the same order.
                
                
                However, be cautious of the fact that if the synchronization start timing is not logically safe, responses will become out-of-sync, causing deadlocks to occur.
                

  Arguments:    context:          The WFSServerContext structure.
                bitmap:           The AID bitmap for the client to be synchronized.
                                 Synchronization will not occur when 0 (the default value) is specified.
                                 The last bit is always ignored.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WFS_SetServerSync(WFSServerContext *context, int bitmap)
{
    PLATFORM_ENTER_CRITICALSECTION();
    context->sync_bitmap = (bitmap & ~1);
    PLATFORM_LEAVE_CRITICALSECTION();
}


/*---------------------------------------------------------------------------*
  $Log: wfs_server.c,v $
  Revision 1.6  2007/10/04 05:36:31  yosizaki
  Fix related to WFS_EndServer.

  Revision 1.5  2007/06/14 13:15:27  yosizaki
  Added hook to a thread-proc.

  Revision 1.4  2007/06/11 10:23:32  yosizaki
  Minor fixes.

  Revision 1.3  2007/06/11 06:40:00  yosizaki
  Added WFS_GetServerPacketLength().

  Revision 1.2  2007/04/17 00:01:06  yosizaki
  Renamed some structures.

  Revision 1.1  2007/04/13 04:12:37  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
