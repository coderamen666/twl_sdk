/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - libraries
  File:     wbt_context.c

  Copyright 2006-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include	<nitro.h>
#include	<nitro/wbt.h>
#include	<nitro/wbt/context.h>


/*---------------------------------------------------------------------------*/
/* Macros */

// #define PRINTF_DEBUG 1
// #define PRINTF_DEBUG_L1 1
// #define PRINTF_DEBUG_L2 1

#if defined(PRINTF_DEBUG)
#define WBT_DEBUG_OUTPUT0       OS_TPrintf
#else
#define WBT_DEBUG_OUTPUT0(...)  (void)0
#endif

#if defined(PRINTF_DEBUG_L1)
#define WBT_DEBUG_OUTPUT1       OS_TPrintf
#else
#define WBT_DEBUG_OUTPUT1(...)  (void)0
#endif

#if defined(PRINTF_DEBUG_L2)
#define WBT_DEBUG_OUTPUT2       OS_TPrintf
#else
#define WBT_DEBUG_OUTPUT2(...)  (void)0
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

/* *INDENT-OFF* */
#define WBT_CMD_0   WBT_CMD_REQ_NONE
#define WBT_COMMAND_REQ(type, pair_type, argument)                  \
    {                                                               \
        (u16) WBT_CMD_REQ_ ## type, (u16) WBT_CMD_ ## pair_type,    \
        sizeof(WBTPacketHeaderFormat) + argument,                   \
        TRUE, FALSE                                                 \
    }
#define WBT_COMMAND_RES(type, pair_type, argument)                  \
    {                                                               \
        (u16) WBT_CMD_RES_ ## type, (u16) WBT_CMD_ ## pair_type,    \
        sizeof(WBTPacketHeaderFormat) + argument,                   \
        FALSE, TRUE                                                 \
    }
#define WBT_COMMAND_MSG(type)                                       \
    {                                                               \
        (u16) WBT_CMD_ ## type, 0, 0, FALSE, FALSE                  \
    }
static const struct
{
    u32     type:8;         /* One's own command type */
    u32     pair_type:8;    /* Command type that is a pair */
    u32     packet;         /* Command packet size (minimum length) */
    u32     is_req:1;       /* TRUE if WBT_CMD_REQ_* */
    u32     is_res:1;       /* TRUE if WBT_CMD_RES_* */
}
WBTi_CommandTable[] =
{
    WBT_COMMAND_MSG(REQ_NONE),
    WBT_COMMAND_REQ(WAIT,           0,                  0),
    WBT_COMMAND_REQ(SYNC,           RES_SYNC,           sizeof(WBTPacketRequestSyncFormat)),
    WBT_COMMAND_RES(SYNC,           REQ_SYNC,           sizeof(WBTPacketResponseSyncFormat)),
    WBT_COMMAND_REQ(GET_BLOCK,      RES_GET_BLOCK,      sizeof(WBTPacketRequestGetBlockFormat)),
    WBT_COMMAND_RES(GET_BLOCK,      REQ_GET_BLOCK,      sizeof(WBTPacketResponseGetBlockFormat)),
    WBT_COMMAND_REQ(GET_BLOCKINFO,  RES_GET_BLOCKINFO,  sizeof(WBTPacketRequestGetBlockFormat)),
    WBT_COMMAND_RES(GET_BLOCKINFO,  REQ_GET_BLOCKINFO,  sizeof(WBTPacketResponseGetBlockFormat)),
    WBT_COMMAND_REQ(GET_BLOCK_DONE, RES_GET_BLOCK_DONE, sizeof(WBTPacketRequestGetBlockDoneFormat)),
    WBT_COMMAND_RES(GET_BLOCK_DONE, REQ_GET_BLOCK_DONE, sizeof(WBTPacketResponseGetBlockDoneFormat)),
    WBT_COMMAND_REQ(USER_DATA,      RES_USER_DATA,      sizeof(WBTPacketRequestUserDataFormat)),
    WBT_COMMAND_RES(USER_DATA,      REQ_USER_DATA,      0),
    WBT_COMMAND_MSG(SYSTEM_CALLBACK),
    WBT_COMMAND_MSG(PREPARE_SEND_DATA),
    WBT_COMMAND_MSG(REQ_ERROR),
    WBT_COMMAND_MSG(RES_ERROR),
    WBT_COMMAND_MSG(CANCEL),
};
enum { WBT_COMMAND_MAX = sizeof(WBTi_CommandTable) / sizeof(*WBTi_CommandTable) };
#undef WBT_CMD_0
#undef WBT_COMMAND_REQ
#undef WBT_COMMAND_RES
#undef WBT_COMMAND_MSG
/* *INDENT-ON* */


/*---------------------------------------------------------------------------*/
/* Functions */

SDK_INLINE int div32(int a)
{
    return (a >> 5);
}

SDK_INLINE int mod32(int a)
{
    return (a & 0x1f);
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_CopySafeMemory

  Description:  Transfers memory or clears it to zero.

  Arguments:    src: Transfer source buffer or NULL
                dst: Transfer destination buffer.
                     If src is NULL, clear to zero.
                len: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE void WBTi_CopySafeMemory(const void *src, void *dst, u32 len)
{
    if (!src)
    {
        MI_CpuFill8(dst, 0x00, len);
    }
    else
    {
        MI_CpuCopy8(src, dst, len);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_GetFirstIterationAID

  Description:  Preparations for performing a looping search with the previous communications target as the starting point.

  Arguments:    context: WBTContext structure

  Returns:      AID of the search's starting position.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBTi_GetFirstIterationAID(WBTContext *context)
{
    const int   mask = context->req_bitmap;
    /* Avoid an infinite loop (search from the front if no previous communications target exists) */
    if (((1 << context->last_target_aid) & mask) == 0)
    {
        context->last_target_aid = 31 - (int)MATH_CLZ((u32)mask);
    }
    return context->last_target_aid;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_GetNextIterationAID

  Description:  Gets the AID that corresponds to the next position in a looping search.

  Arguments:    aid: Current AID
                mask: Bitmap of all search targets

  Returns:      The next AID.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBTi_GetNextIterationAID(int aid, int mask)
{
    ++aid;
    if ((1 << aid) > mask)
    {
        aid = (int)MATH_CTZ((u32)mask);
    }
    return aid;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_InitBitmap

  Description:  Initializes the bitmap structure.

  Arguments:    pkt_bmp: Bitmap structure
                length: Byte size of the data length
                bits: Bitmap buffer
                buffer: Data buffer

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WBTi_InitBitmap(WBTContext * work, WBTPacketBitmap * pkt_bmp, s32 length, u32 *bits, u8 *buffer,
                            int packet)
{
    SDK_ASSERT(packet > 0);
    pkt_bmp->length = length;
    pkt_bmp->buffer = buffer;
    pkt_bmp->total = (length + packet - 1) / packet;
    pkt_bmp->count = 0;
    pkt_bmp->bitmap = bits;
    MI_CpuFill8(bits, 0x00, (u32)WBT_GetBitmapLength(work, length));
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_MergeBitmapIndex

  Description:  If the specified index in the bitmap has not yet been received, stores it.

  Arguments:    pkt_bmp: Bitmap structure
                index: Packet index
                packet: Per-packet size
                src: Packet data

  Returns:      If not yet received, store it and return TRUE.
 *---------------------------------------------------------------------------*/
static BOOL WBTi_MergeBitmapIndex(WBTPacketBitmap * pkt_bmp, int index, u32 packet, const void *src)
{
    BOOL    retval = FALSE;
    u32     pos = (u32)div32(index);
    u32     bit = (u32)mod32(index);
    if ((pkt_bmp->bitmap[pos] & (1 << bit)) == 0)
    {
        u8     *dst = pkt_bmp->buffer;
        const u32 offset = index * packet;
        const u32 total = (u32)pkt_bmp->length;
        u32     length = (u32)MATH_MIN(packet, total - offset);
        MI_CpuCopy8(src, dst + offset, length);
        pkt_bmp->bitmap[pos] |= (1 << bit);
        pkt_bmp->count += 1;
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_FindBitmapIndex

  Description:  Searches for a not-yet-received index in the bitmap.

  Arguments:    pkt_bmp: Bitmap structure

  Returns:      The unreceived index or -1.
 *---------------------------------------------------------------------------*/
static s32 WBTi_FindBitmapIndex(WBTPacketBitmap * pkt_bmp)
{
    int     last_num;
    int     bit_num;
    u32    *bmp;
    int     num;

    num = pkt_bmp->current + 1;
    if (num >= pkt_bmp->total)
    {
        num = 0;
    }
    last_num = num;
    bmp = pkt_bmp->bitmap + div32(num);
    bit_num = mod32(num);

    for (;;)
    {
        /* If the specified index has not yet been received, finish searching */
        if ((*bmp & (u32)((u32)1 << bit_num)) == 0)
        {
            break;
        }
        /* Otherwise, move on to the next index */
        else
        {
            /* Loop to the beginning once the last index point is reached */
            if (++num >= pkt_bmp->total)
            {
                num = 0;
                bit_num = 0;
                bmp = pkt_bmp->bitmap;
            }
            /* The bitmap for managing receptions is controlled in 32-bit increments */
            else if (++bit_num >= 32)
            {
                bit_num = 0;
                ++bmp;
            }
            /* All packets have been received */
            if (num == last_num)
            {
                num = -1;
                break;
            }
        }
    }
    return num;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_GetPacketBuffer

  Description:  Gets the buffer of the packet data represented by the index in the specified block.
                

  Arguments:    work: WBT structure
                id: Requested block ID
                index: Requested index

  Returns:      Pointer to the block data.
 *---------------------------------------------------------------------------*/
static u8 *WBTi_GetPacketBuffer(WBTContext * work, u32 id, s32 index)
{
    u8     *ptr = NULL;
    WBTBlockInfoList *list = work->list;
    int     count = 0;
    for (; list != NULL; list = list->next)
    {
        if (id < WBT_NUM_MAX_BLOCK_INFO_ID)
        {
            if (id == count)
            {
                /*
                 * CAUTION!
                 * Don't pass the internal management structure unchanged. Decide endianness here.
                 * (However, this is not a permanent measure, just a temporary workaround.)
                 */
                static WBTBlockInfo tmp;
                tmp = list->data_info;
                tmp.id = MI_HToLE32(tmp.id);
                tmp.block_size = (s32)MI_HToLE32(tmp.block_size);
                ptr = (u8 *)&tmp;
//                ptr = (u8 *)&list->data_info;
                break;
            }
        }
        else
        {
            if (id == list->data_info.id)
            {
                ptr = (u8 *)list->data_ptr;
                break;
            }
        }
        ++count;
    }
    if (ptr)
    {
        ptr += index * work->my_data_packet_size;
    }
    return ptr;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_SwitchNextCommand

  Description:  Called when there is an update to the command list.
                  - When a new command is issued.
                  - When the current command has completed.

  Arguments:    work: WBT structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WBTi_SwitchNextCommand(WBTContext * work)
{
    BOOL    retval = FALSE;

    /* Process if the updated command is valid */
    WBTCommand *current = WBT_GetCurrentCommand(work);
    if (current)
    {
        /* Increment the command counter (avoid 0) */
        if (++work->my_command_counter == 0)
        {
            ++work->my_command_counter;
        }
        current->my_cmd_counter = work->my_command_counter;

        /* Initialization processing for each command */
        switch (current->command)
        {

        case WBT_CMD_REQ_GET_BLOCK:
        case WBT_CMD_REQ_GET_BLOCKINFO:
            /* Initialize the bitmap */
            {
                int     aid;
                for (aid = 0; aid < 16; ++aid)
                {
                    if ((current->target_bmp & (1 << aid)) != 0)
                    {
                        WBTPacketBitmap *pkt_bmp = &work->peer_param[aid].pkt_bmp;
                        WBTi_InitBitmap(work, pkt_bmp, (int)current->get.recv_data_size,
                                        current->get.pkt_bmp_table.packet_bitmap[aid],
                                        current->get.recv_buf_table.recv_buf[aid],
                                        work->peer_data_packet_size);
                        pkt_bmp->current = 0;
                    }
                }
            }
            break;

        case WBT_CMD_REQ_SYNC:
        case WBT_CMD_REQ_USER_DATA:
            /* Unnecessary initialization commands */
            break;

        default:
            /* Commands that should not be possible */
            current->command = WBT_CMD_REQ_NONE;
            break;

        }
        retval = (current->command != WBT_CMD_REQ_NONE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_NotifySystemCallback

  Description:  System callback notification.

  Arguments:    work: WBT structure
                event: Event type
                aid: In the case of a requested receive event, the other party's AID
                result: Processing result

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WBTi_NotifySystemCallback(WBTContext * work, WBTCommandType event, int aid,
                                      WBTResult result)
{
    BOOL    retval = TRUE;
    WBTRecvToken *peer = &work->peer_param[aid].recv_token;

    /* For anything other than WBT_CMD_PREPARE_SEND_DATA, only notify once */
    if ((event != WBT_CMD_PREPARE_SEND_DATA) &&
        (peer->token_peer_cmd_counter == peer->last_peer_cmd_counter))
    {
        retval = FALSE;
    }
    /*
     * Because the GetBlockInfo function uses the GetBlock function in the library, it is not necessary (and is not allowed) to notify the application of completion of this internal processing
     * 
     */
    else if ((event == WBT_CMD_REQ_GET_BLOCK_DONE) &&
             (peer->token_block_id < WBT_NUM_MAX_BLOCK_INFO_ID))
    {
        retval = FALSE;
    }
    /* If necessary, generate a system callback */
    if (retval)
    {
        WBTCommand *cmd = &work->system_cmd;
        peer->last_peer_cmd_counter = peer->token_peer_cmd_counter;
        cmd->peer_cmd_counter = peer->token_peer_cmd_counter;   /* For debugging */
        cmd->result = result;
        cmd->event = event;
        cmd->command = WBT_CMD_SYSTEM_CALLBACK;
        cmd->peer_bmp = (u16)(1 << aid);
        /* New specifications */
        if (work->callback)
        {
            work->callback(work->userdata, cmd);
        }
        /* Old specifications */
        else if (cmd->callback)
        {
            (*cmd->callback) (cmd);
        }
    }
}


/*****************************************************************************
 * Packet Transmission
 *****************************************************************************/

PLATFORM_ATTRIBUTE_INLINE
    WBTPacketFormat * WBTi_MakeCommandHeader(void *dst, u8 cmd, int target, u8 counter)
{
    WBTPacketFormat *format = (WBTPacketFormat *) dst;
    MI_StoreLE8(&format->header.command, cmd);
    MI_StoreLE16(&format->header.bitmap, (u16)target);
    MI_StoreLE8(&format->header.counter, counter);
    return format;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_TryCreateResponse

  Description:  Generates a response if a request is currently being received.

  Arguments:    work: WBT structure
                aid: Other party's AID
                buf: Buffer for creating a send command
                size: Buffer size
                command: Command that verifies the response
                foroce_blockinfo: TRUE if there is a response, even if the specified ID does not exist

  Returns:      The packet length of the response, if it could be generated, and 0 otherwise.
 *---------------------------------------------------------------------------*/
static
    int WBTi_TryCreateResponse(WBTContext * work, int aid, void *buf, int size, int command,
                               BOOL foroce_blockinfo)
{
    int     ret_size = 0;
    WBTRecvToken *token = &work->peer_param[aid].recv_token;
    BOOL    done = FALSE;
    (void)size;
    if (command == WBT_CMD_REQ_SYNC)
    {
        WBTPacketFormat *format = WBTi_MakeCommandHeader(buf, WBT_CMD_RES_SYNC, (1 << aid),
                                                         token->token_peer_cmd_counter);
        /* Be cautious of the fact that the transmission destination is the standard for distinguishing between oneself and others */
        MI_StoreLE16(&format->res_sync.block_total, (u16)WBT_GetRegisteredCount(work));
        MI_StoreLE16(&format->res_sync.peer_packet, (u16)work->my_data_packet_size);
        MI_StoreLE16(&format->res_sync.own_packet, (u16)work->peer_data_packet_size);
        ret_size = (int /* Temporary */ )sizeof(format->header) + sizeof(format->res_sync);
        done = TRUE;
    }
    else if (command == WBT_CMD_REQ_GET_BLOCK_DONE)
    {
        u32     id = token->token_block_id;
        WBTPacketFormat *format =
            WBTi_MakeCommandHeader(buf, WBT_CMD_RES_GET_BLOCK_DONE, (1 << aid),
                                   token->token_peer_cmd_counter);
        MI_StoreLE32(&format->res_getblock_done.id, id);
        ret_size = (int /* Temporary */ )sizeof(format->header) + sizeof(format->res_getblock_done);
        done = TRUE;
        WBT_DEBUG_OUTPUT1("send BlockDone to %d id = %d\n", aid, id);
    }
    else if (command == WBT_CMD_REQ_USER_DATA)
    {
        WBTPacketFormat *format = WBTi_MakeCommandHeader(buf, WBT_CMD_RES_USER_DATA, (1 << aid),
                                                         token->token_peer_cmd_counter);
        ret_size = (int /* Temporary */ )sizeof(format->header);
        done = TRUE;
    }
    else if (command == WBT_CMD_REQ_GET_BLOCK)
    {
        /*
         * Currently, if the requested block does not exist, nothing is done.
         * Establishing a new error and sending notification would also be acceptable.
         */
        u32     id = token->token_block_id;
        /* Even for id < 1000, block information is not returned here */
        WBTBlockInfoList *list = work->list;
        for (; list != NULL; list = list->next)
        {
            if (list->data_info.id == id)
            {
                break;
            }
        }
        if (list)
        {
            /*
             * Calculate the appropriate response index corresponding to the block request.
             * Because it is easy for request contents to be duplicated due to MP communications information delays, manage the response history on the responding side and avoid duplicates in the past two responses.
             * 
             */
            s32     index = token->token_block_seq_no;
            s32     block_seq_no;
            /* Respond with the specified index as is for GetBlockInfo requests */
            if (id >= WBT_NUM_MAX_BLOCK_INFO_ID)
            {
                /*
                 * Search for the specified block.
                 * CAUTION!
                 *     This determination is already made at the one location where this function is called.
                 *     It is therefore impossible for this function to return -1!
                 */
                /* Take the response history into account in the case of blocks that recently responded */
                if (id == work->last_block_id)
                {
                    const int index_total =
                        (list->data_info.block_size + work->my_data_packet_size -
                         1) / work->my_data_packet_size;
                    int     i;
                    /* Search for the most recent indices, which do not exist in the response history */
                    for (i = 0; (i < 3) && ((index == work->last_seq_no_1) ||
                                            (index == work->last_seq_no_2)); ++i)
                    {
                        if (++index >= index_total)
                        {
                            index = 0;
                        }
                    }
                }
                /*
                 * Record the contents of the current response in the response history.
                 * CAUTION!
                 *     In this implementation, the index of an unrelated block from the previous cycle is also unfortunately retained, in last_seq_no_2.
                 *     
                 */
                work->last_block_id = id;
                work->last_seq_no_2 = work->last_seq_no_1;
                work->last_seq_no_1 = index;
            }
            block_seq_no = index;

            {
                u32     packet = work->my_data_packet_size;
                u8     *data_ptr = NULL;
                BOOL    sendable = FALSE;
                if (list->data_ptr)
                {
                    data_ptr = WBTi_GetPacketBuffer(work, id, block_seq_no);
                    WBT_DEBUG_OUTPUT1("send BlockData to %d id = %d seq no = %d pktsize %d\n", aid,
                                      id, index, packet);
                    sendable = TRUE;
                }
                else if (list->block_type == WBT_BLOCK_LIST_TYPE_USER)
                {
                    /* Notification to bring the data packet preparation to the user's attention */
                    WBTCommand *system_cmd = &work->system_cmd;
                    system_cmd->prepare_send_data.block_id = id;
                    system_cmd->prepare_send_data.block_seq_no = block_seq_no;
                    system_cmd->prepare_send_data.own_packet_size = (s16)packet;
                    system_cmd->prepare_send_data.data_ptr = NULL;
                    WBTi_NotifySystemCallback(work, WBT_CMD_PREPARE_SEND_DATA, aid,
                                              WBT_RESULT_SUCCESS);
                    WBT_DEBUG_OUTPUT1("peer req seq no  = %d seq no = %d dataptr = %p\n", index,
                                      block_seq_no, system_cmd->prepare_send_data.data_ptr);
                    if (system_cmd->prepare_send_data.data_ptr != NULL)
                    {
                        id = system_cmd->prepare_send_data.block_id;
                        block_seq_no = system_cmd->prepare_send_data.block_seq_no;
                        data_ptr = system_cmd->prepare_send_data.data_ptr;
                        packet = system_cmd->prepare_send_data.own_packet_size;
                        sendable = TRUE;
                    }
                }
                /* Send if the data has been prepared */
                if (sendable)
                {
                    /* CAUTION! This calculation of the transmission destination bitmap is apparently only provisional processing */
                    u16     target = (u16)((WBT_GetAid(work) == WBT_AID_PARENT) ? 0xFFFE : 0x0001);
                    WBTPacketFormat *format =
                        WBTi_MakeCommandHeader(buf, WBT_CMD_RES_GET_BLOCK, target,
                                               token->token_peer_cmd_counter);
                    MI_StoreLE32(&format->res_getblock.id, id);
                    MI_StoreLE32(&format->res_getblock.index, (u32)block_seq_no);
                    WBTi_CopySafeMemory(data_ptr, &format->res_getblock + 1, (u32)packet);
                    ret_size = (int /* Temporary */ )(sizeof(format->header) +
                                                     sizeof(format->res_getblock) + packet);
                    done = TRUE;
                }
            }
        }
    }
    else if (command == WBT_CMD_REQ_GET_BLOCKINFO)
    {
        u32     id = token->token_block_id;
        s32     index = token->token_block_seq_no;
        int     packet = work->my_data_packet_size;
        u8     *data_ptr = WBTi_GetPacketBuffer(work, id, index);
        WBT_DEBUG_OUTPUT1("send BlockData to %d id = %d seq no = %d pktsize %d\n", aid, id, index,
                          packet);
        /*
         * CAUTION!
         *     It is possible for data_ptr to become NULL, but only if there are no blocks for the specified ID.
         *     In that case, wouldn't it be better to return a callback or always send 0 data?
         *     This is currently being revised in HEAD!
         */
        if (foroce_blockinfo || data_ptr)
        {
            /* CAUTION! This calculation of the transmission destination bitmap is apparently only provisional processing */
            u16     target = (u16)((WBT_GetAid(work) == 0) ? 0xFFFF : 0x0001);
            WBTPacketFormat *format = WBTi_MakeCommandHeader(buf, WBT_CMD_RES_GET_BLOCKINFO, target,
                                                             token->token_peer_cmd_counter);
            MI_StoreLE32(&format->res_getblock.id, id);
            MI_StoreLE32(&format->res_getblock.index, (u32)index);
            WBTi_CopySafeMemory(data_ptr, &format->res_getblock + 1, (u32)packet);
            ret_size = (int /* Temporary */ )(sizeof(format->header) + sizeof(format->res_getblock) +
                                             packet);
            done = TRUE;
        }
    }

    /* Destroy the response content after responding */
    if (done)
    {
        work->req_bitmap &= ~(1 << aid);
    }
    return ret_size;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_CheckRequest

  Description:  Generate one's own command request.

  Arguments:    work: WBT structure
                buffer: Buffer to store the data
                length: Buffer size

  Returns:      The size of the request, if it could be generated, and 0 otherwise.
 *---------------------------------------------------------------------------*/
static int WBTi_CheckRequest(WBTContext * work, void *send_buf, int size)
{
    int     send_size = 0;
    WBTCommand *current = WBT_GetCurrentCommand(work);
    (void)size;
    if (current)
    {
        switch (current->command)
        {
        case WBT_CMD_REQ_SYNC:
            {
                WBTPacketFormat *format =
                    WBTi_MakeCommandHeader(send_buf, WBT_CMD_REQ_SYNC, current->target_bmp,
                                           current->my_cmd_counter);
                /* Be cautious of the fact that the transmission destination is the standard for distinguishing between oneself and others */
                MI_StoreLE16(&format->req_sync.peer_packet, (u16)work->my_data_packet_size);
                MI_StoreLE16(&format->req_sync.own_packet, (u16)work->peer_data_packet_size);
                send_size = (int /* Temporary */ )sizeof(format->header) + sizeof(format->req_sync);
                WBT_DEBUG_OUTPUT0("send ReqSync to 0x%04x cmd counter %d\n",
                                  current->target_bmp, current->my_cmd_counter);
            }
            break;
        case WBT_CMD_REQ_GET_BLOCK:
        case WBT_CMD_REQ_GET_BLOCKINFO:
            {
                int     aid;
                for (aid = 0; aid < 16; ++aid)
                {
                    /* Confirm that this is the partner that should be requested */
                    if ((current->target_bmp & (1 << aid)) != 0)
                    {
                        WBTPacketBitmap *pkt_bmp = &work->peer_param[aid].pkt_bmp;
                        s32     next_seq_no = WBTi_FindBitmapIndex(pkt_bmp);
                        /* BlockDone if reception is complete */
                        if (next_seq_no == -1)
                        {
                            WBTPacketFormat *format =
                                WBTi_MakeCommandHeader(send_buf, WBT_CMD_REQ_GET_BLOCK_DONE,
                                                       (1 << aid), current->my_cmd_counter);
                            MI_StoreLE32(&format->req_getblock_done.id, current->get.block_id);
                            send_size = (int /* Temporary */ )sizeof(format->header) +
                                sizeof(format->req_getblock_done);
                            WBT_DEBUG_OUTPUT0("send ReqBlockDone to %d 0x%04x\n", aid, (1 << aid));
                        }
                        /* Request the next block if reception is not complete */
                        else
                        {
                            WBTPacketFormat *format =
                                WBTi_MakeCommandHeader(send_buf, current->command,
                                                       current->target_bmp,
                                                       current->my_cmd_counter);
                            MI_StoreLE32(&format->req_getblock.id, current->get.block_id);
                            MI_StoreLE32(&format->req_getblock.index, (u32)next_seq_no);
                            send_size = (int /* Temporary */ )sizeof(format->header) +
                                sizeof(format->req_getblock);
                            WBT_DEBUG_OUTPUT1("send ReqBlock id=%d seq=%d\n", current->get.block_id,
                                              next_seq_no);
                        }
                        if (send_size)
                        {
                            break;
                        }
                    }
                }
            }
            break;

        case WBT_CMD_REQ_USER_DATA:
            {
                WBTPacketFormat *format =
                    WBTi_MakeCommandHeader(send_buf, WBT_CMD_REQ_USER_DATA, current->target_bmp,
                                           current->my_cmd_counter);
                MI_StoreLE8(&format->req_userdata.length, current->user_data.size);
                MI_CpuCopy8(current->user_data.data, &format->req_userdata.buffer,
                            WBT_SIZE_USER_DATA);
                send_size =
                    (int /* Temporary */ )sizeof(format->header) + sizeof(format->req_userdata);
            }
            break;

        default:
            WBT_DEBUG_OUTPUT0("Unknown User Command:Error %s %s %d\n", __FILE__, __FUNCTION__,
                              __LINE__);
            break;
        }
    }
    return send_size;
}

/*---------------------------------------------------------------------------*
  Name:         WBTi_CheckBlockResponse

  Description:  Confirm requests in order from each AID, and generate appropriate block responses.

  Arguments:    work: WBT structure
                buffer: Buffer to store the data
                length: Buffer size

  Returns:      The size of the block response, if it could be generated, and 0 otherwise.
 *---------------------------------------------------------------------------*/
static int WBTi_CheckBlockResponse(WBTContext * work, void *buffer, int length)
{
    int     retval = 0;

    /* Determine the currently requested targets in order */
    int     mask = work->req_bitmap;
    if (!retval && (mask != 0))
    {
        int     aid = WBTi_GetFirstIterationAID(work);
        do
        {
            aid = WBTi_GetNextIterationAID(aid, mask);
            if ((work->req_bitmap & (1 << aid)) != 0)
            {
                WBTRecvToken *token = &work->peer_param[aid].recv_token;
                if (WBT_CMD_REQ_GET_BLOCK == token->token_command)
                {
                    /*
                     * Attempt to create a response command.
                     * For execution that has come here, the only things that return 0 are:
                     * -WBT_CMD_REQ_GET_BLOCK
                     *   -- There are no blocks for the specified ID.
                     *   -- Preparations were not done in the PREPARE callback.
                     *  
                     * However, even if there is no response for WBT_CMD_REQ_GET_BLOCK, execution will at least finish.
                     * 
                     */
                    retval =
                        WBTi_TryCreateResponse(work, aid, buffer, length, WBT_CMD_REQ_GET_BLOCK,
                                               FALSE);
                }
                if (retval)
                {
                    work->last_target_aid = aid;
                }
            }
        }
        while (aid != work->last_target_aid);
    }

    if (!retval && (mask != 0))
    {
        int     aid = WBTi_GetFirstIterationAID(work);
        do
        {
            aid = WBTi_GetNextIterationAID(aid, mask);
            if ((work->req_bitmap & (1 << aid)) != 0)
            {
                WBTRecvToken *token = &work->peer_param[aid].recv_token;
                if (WBT_CMD_REQ_GET_BLOCKINFO == token->token_command)
                {
                    retval =
                        WBTi_TryCreateResponse(work, aid, buffer, length, WBT_CMD_REQ_GET_BLOCKINFO,
                                               TRUE);
                }
                if (retval)
                {
                    work->last_target_aid = aid;
                }
            }
        }
        while (aid != work->last_target_aid);
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CallPacketSendHook

  Description:  Hook function for generating the send packet data.

  Arguments:    work: WBT structure
                buffer: Buffer to store the data
                length: Buffer size

  Returns:      The generated packet size.
 *---------------------------------------------------------------------------*/
int WBT_CallPacketSendHook(WBTContext * work, void *buffer, int length, BOOL is_parent)
{
    int     retval = 0;

    if (work->last_target_aid == -1)
    {
        work->last_target_aid = (is_parent ? 1 : 0);
    }

    /* Check for illegal arguments */
    if (!buffer || (length <= 0))
    {
        return 0;
    }

    /* Confirm requests from each AID in order, and generate appropriate responses */
    {
        /*
         * CAUTION!
         *     If the response generation priority is (AID order > command), isn't a single determination reasonable because duplicate commands from the same AID are not accepted?
         *     
         *     
         */
        static const WBTCommandType tbl[] = {
            WBT_CMD_REQ_USER_DATA,
            WBT_CMD_REQ_SYNC,
            WBT_CMD_REQ_GET_BLOCKINFO,
            WBT_CMD_REQ_GET_BLOCK_DONE,
            WBT_CMD_REQ_NONE,
        };
        /* Determine the currently requested targets in order */
        int     mask = work->req_bitmap;
        if (mask != 0)
        {
            int     aid = WBTi_GetFirstIterationAID(work);
            do
            {
                aid = WBTi_GetNextIterationAID(aid, mask);
                if ((mask & (1 << aid)) != 0)
                {
                    /*
                     * Attempt to create a response command.
                     * For execution that has come here, the only things that return 0 are:
                     * - WBT_CMD_REQ_GET_BLOCKINFO
                     *   -- There are no blocks for the specified ID, and "force" is not used.
                     *  
                     */
                    WBTRecvToken *token = &work->peer_param[aid].recv_token;
                    int     i;
                    for (i = 0; !retval && (tbl[i] != WBT_CMD_REQ_NONE); ++i)
                    {
                        /* WBT_CMD_REQ_*** */
                        if (tbl[i] == token->token_command)
                        {
                            retval =
                                WBTi_TryCreateResponse(work, aid, buffer, length, tbl[i], FALSE);
                        }
                    }
                    if (retval)
                    {
                        work->last_target_aid = aid;
                    }
                }
            }
            while (aid != work->last_target_aid);
        }
    }


    if (!retval)
    {
        /*
         * A parent device's command priority is:
         *   (1) General response (unregistered GetBlockInfo is ignored)
         *   (2) Requests from the local host
         *   (3) Block response (unregistered GetBlockInfo is also forcibly responded to)
         *   (4) (Wait)
         */
        if (is_parent)
        {
            /*
             * 0 is returned here in the following cases.
             * - When no command is being issued.
             */
            retval = WBTi_CheckRequest(work, buffer, length);
            if (!retval)
            {
                retval = WBTi_CheckBlockResponse(work, buffer, length);
            }
        }
        /*
         * A child device's command priority is:
         *   (1) General response (unregistered GetBlockInfo is ignored)
         *   (3) Block response (unregistered GetBlockInfo is also forcibly responded to)
         *   (2) Requests from the local host
         *   (4) (Wait)
         */
        else
        {
            retval = WBTi_CheckBlockResponse(work, buffer, length);
            if (!retval)
            {
                /*
                 * 0 is returned here in the following cases.
                 * - When no command is being issued.
                 */
                retval = WBTi_CheckRequest(work, buffer, length);
            }
        }
        if (!retval)
        {
            /* A WAIT command is always sent when the counter is 0 */
            int     mask = (is_parent ? 0xFFFE : 0x0001);
            WBTPacketFormat *format =
                WBTi_MakeCommandHeader(buffer, WBT_CMD_REQ_WAIT, (u16)mask, (WBTCommandCounter)0);
            retval = (int /* Temporary */ )sizeof(format->header);
        }
    }

    return retval;
}


/*****************************************************************************
 * Packet Reception
 *****************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         WBTi_NotifyCompletionCallback

  Description:  Notification of user command completion.

  Arguments:    work: WBT structure
                event: Event type
                aid: AID of the other party that has completed the command

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WBTi_NotifyCompletionCallback(WBTContext * work, WBTCommandType event, int aid)
{
    WBTCommandList *list = WBT_GetCurrentCommandList(work);
    WBTCommand *current = WBT_GetCurrentCommand(work);
    WBTRecvToken *token = &work->peer_param[aid].recv_token;
    /* A notification is only generated once */
    if ((current != NULL) &&
        (current->my_cmd_counter == token->token_peer_cmd_counter))
    {
        int     peer_bmp = (1 << aid);
        current->peer_cmd_counter = token->token_peer_cmd_counter;
        current->peer_bmp = (u16)peer_bmp;      /* For debugging */
        if ((current->target_bmp & peer_bmp) != 0)
        {
            /* This is a completion notification, so it's always "SUCCESS." (Errors are notified to a system callback) */
            current->target_bmp &= ~peer_bmp;
            current->event = event;
            current->result = WBT_RESULT_SUCCESS;
            /* New specifications */
            if (list->callback)
            {
                list->callback(work->userdata, current);
            }
            /* Old specifications */
            else if (current->callback)
            {
                current->callback(current);
            }
        }
        /* Delete the command if all targets have finished responding */
        if (current->target_bmp == 0)
        {
            WBTCommandList *list = work->command;
            work->command = list->next;
            WBT_AddCommandPool(work, list, 1);
            WBTi_SwitchNextCommand(work);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CallPacketRecvHook

  Description:  Parses the receive packet data.

  Arguments:    work: WBT structure
                aid: Data sender's AID
                buffer: Received data buffer
                length: Length of the received data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_CallPacketRecvHook(WBTContext * work, int aid, const void *buffer, int length)
{
    WBTRecvToken *token = &work->peer_param[aid].recv_token;

    /* The previous reception state is always cleared here */
    work->req_bitmap &= ~(1 << aid);

    /* At the very least, the command header should always exist */
    if (buffer && (length >= sizeof(WBTPacketHeaderFormat)))
    {
        const WBTPacketFormat *format = (const WBTPacketFormat *)buffer;

        u8      command;
        u16     bitmap;

        command = MI_LoadLE8(&format->header.command);
        bitmap = MI_LoadLE16(&format->header.bitmap);
        token->token_peer_cmd_counter = MI_LoadLE8(&format->header.counter);
        token->token_command = command;


        /*
         * Perform command processing if the packet is addressed to the local host.
         * Note: Although these are if-else statements, we ultimately want to use a function table.
         */
        if ((WBT_GetAid(work) != -1) && ((bitmap & (1 << WBT_GetAid(work))) != 0))
        {
            /* Ignore unknown, out-of-range commands */
            if (command >= WBT_COMMAND_MAX)
            {
            }
            /* Ignore if the command's packet size does not satisfy the minimum length */
            else if (length < WBTi_CommandTable[command].packet)
            {
            }
            /** Request command */
            else if (WBTi_CommandTable[command].is_req)
            {
                if (command == WBT_CMD_REQ_WAIT)
                {
                }

                else if (command == WBT_CMD_REQ_SYNC)
                {
                    WBTRequestSyncCallback *cb = &work->system_cmd.sync;
                    cb->peer_packet_size = (s16)MI_LoadLE16(&format->req_sync.peer_packet);
                    cb->my_packet_size = (s16)MI_LoadLE16(&format->req_sync.own_packet);
                    cb->num_of_list = 0;        /* In the old specifications, this is the only member not included in a request */
                    /* The child device always complies with the parent device's communication settings */
                    if (WBT_GetAid(work) != 0)
                    {
                        work->my_data_packet_size = cb->my_packet_size;
                        work->peer_data_packet_size = cb->peer_packet_size;
                    }
                    work->req_bitmap |= (1 << aid);
                }
                /* User-defined data request */
                else if (command == WBT_CMD_REQ_USER_DATA)
                {
                    WBTRecvUserDataCallback *cb = &work->system_cmd.user_data;
                    cb->size = MI_LoadLE8(&format->req_userdata.length);
                    if (cb->size > WBT_SIZE_USER_DATA)
                    {
                        cb->size = 0;
                    }
                    MI_CpuCopy8(format->req_userdata.buffer, cb->data, cb->size);
                    work->req_bitmap |= (1 << aid);
                }
                /* Block information request or block request (same format) */
                else if ((command == WBT_CMD_REQ_GET_BLOCK) ||
                         (command == WBT_CMD_REQ_GET_BLOCKINFO))
                {
                    token->token_block_id = MI_LoadLE32(&format->req_getblock.id);
                    token->token_block_seq_no = (s32)MI_LoadLE32(&format->req_getblock.index);
                    work->req_bitmap |= (1 << aid);
                    WBT_DEBUG_OUTPUT1("get req Block from %d id = %d seq no = %d\n", aid, token->token_block_id,
                                      token->token_block_seq_no);
                }
                /* Notification that reception of a block has completed */
                else if (command == WBT_CMD_REQ_GET_BLOCK_DONE)
                {
                    WBTGetBlockDoneCallback *cb = &work->system_cmd.blockdone;
                    cb->block_id = MI_LoadLE32(&format->req_getblock_done.id);
                    // It is also possible for a completion notification to be sent without a request ever being sent. This happens when a child piggy-backs on another child's currently executing request to the same ID
                    //  
                    token->token_block_id = MI_LoadLE32(&format->req_getblock_done.id);
                    work->req_bitmap |= (1 << aid);
                }
            }

            /* Response command */
            else if (WBTi_CommandTable[command].is_res)
            {
                WBTCommand *current = WBT_GetCurrentCommand(work);

                /* Ignore if not currently requesting a command */
                if (!current)
                {
                }
                /*
                 * CAUTION!
                 *     Although we would like to unify the processes which determine whether or not REQ_ and RES_ have been dealt with, GetBlock-related processes do not have a one-to-one correspondence and therefore we have not addressed this yet
                 *     
                 *     (1) WBT_CMD_RES_GET_BLOCK / WBT_CMD_REQ_GET_BLOCK
                 *     (2) WBT_CMD_RES_GET_BLOCKINFO / WBT_CMD_REQ_GET_BLOCKINFO
                 *     (3) WBT_CMD_RES_GET_BLOCK_DONE / WBT_CMD_REQ_GET_BLOCK, WBT_CMD_REQ_GET_BLOCKINFO
                 *     What ought to be done when sending WBT_CMD_REQ_GET_BLOCK_DONE?
                 */
                else
                {
                    /* Synchronous response */
                    if (command == WBT_CMD_RES_SYNC)
                    {
                        if (current->command == WBT_CMD_REQ_SYNC)
                        {
                            current->sync.num_of_list =
                                (s16)MI_LoadLE16(&format->res_sync.block_total);
                            current->sync.peer_packet_size =
                                (s16)MI_LoadLE16(&format->res_sync.peer_packet);
                            current->sync.my_packet_size =
                                (s16)MI_LoadLE16(&format->res_sync.own_packet);

                            /* The child device always complies with the parent device's communication settings */
                            if (WBT_GetAid(work) != 0)
                            {
                                work->my_data_packet_size = current->sync.my_packet_size;
                                work->peer_data_packet_size = current->sync.peer_packet_size;
                            }
                            WBT_DEBUG_OUTPUT0("Get res Sync from %d my %d peer %d\n", aid,
                                              current->sync.my_packet_size,
                                              current->sync.peer_packet_size);
                            WBTi_NotifyCompletionCallback(work, (WBTCommandType)command, aid);
                        }
                    }
                    /* Completion notification */
                    else if (command == WBT_CMD_RES_USER_DATA)
                    {
                        if (current->command == WBT_CMD_REQ_USER_DATA)
                        {
                            WBTi_NotifyCompletionCallback(work, (WBTCommandType)command, aid);
                        }
                    }
                    /* Block response */
                    else if ((command == WBT_CMD_RES_GET_BLOCK) ||
                             (command == WBT_CMD_RES_GET_BLOCKINFO))
                    {
                        if ((current->command == WBT_CMD_REQ_GET_BLOCK) ||
                            (current->command == WBT_CMD_REQ_GET_BLOCKINFO))
                        {
                            u32     id = MI_LoadLE32(&format->res_getblock.id);
                            s32     index = (s32)MI_LoadLE32(&format->res_getblock.index);

                            /* Determine whether or not this matches the requested ID */
                            if (id == current->get.block_id)
                            {
                                /* Determine whether the response is from the request location */
                                if ((current->target_bmp & (1 << aid)) != 0)
                                {
                                    /* Determine whether the index is in range */
                                    WBTPacketBitmap *pkt_bmp = &work->peer_param[aid].pkt_bmp;
                                    if (index >= pkt_bmp->total)
                                    {
                                        WBT_DEBUG_OUTPUT1
                                            ("%s num of seq over seq no = %d total = %d\n",
                                             __FUNCTION__, index, pkt_bmp->total);
                                        /* Save failed */
                                        WBTi_NotifySystemCallback(work, WBT_CMD_RES_ERROR, aid,
                                                                  WBT_RESULT_ERROR_SAVE_FAILURE);
                                    }
                                    else
                                    {
                                        /* If the data is not yet received, store */
                                        const void *src = (const u8 *)format +
                                            sizeof(format->header) + sizeof(format->res_getblock);
                                        u32     packet = (u32)work->peer_data_packet_size;
                                        if (WBTi_MergeBitmapIndex(pkt_bmp, index, packet, src))
                                        {
                                            pkt_bmp->current = index;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    /* Notification that reception of a block has completed */
                    else if (command == WBT_CMD_RES_GET_BLOCK_DONE)
                    {
                        /* Completion notification if the response is the correct one corresponding to the current request */
                        if ((current->command == WBT_CMD_REQ_GET_BLOCK) ||
                            (current->command == WBT_CMD_REQ_GET_BLOCKINFO))
                        {
                            u32     id = MI_LoadLE32(&format->res_getblock_done.id);
                            if (current->get.block_id == id)
                            {
                                WBT_DEBUG_OUTPUT1
                                    ("get block my cmd counter = %d peer cmd counter = %d\n",
                                     current->my_cmd_counter, token->token_peer_cmd_counter);
                                WBTi_NotifyCompletionCallback(work,
                                                              (current->command ==
                                                               WBT_CMD_REQ_GET_BLOCK) ?
                                                              WBT_CMD_RES_GET_BLOCK :
                                                              WBT_CMD_RES_GET_BLOCKINFO, aid);
                            }
                            WBT_DEBUG_OUTPUT0("c usr cmd tbmp 0x%x\n", current->target_bmp);
                        }
                    }
                }
            }

            /* Not supported or an illegal command */
            else
            {
                WBTi_NotifySystemCallback(work, WBT_CMD_RES_ERROR, aid,
                                          WBT_RESULT_ERROR_UNKNOWN_PACKET_COMMAND);
            }

        }

        /* System callback if a request is received */
        if ((work->req_bitmap & (1 << aid)) != 0)
        {
            /* However, do not send notifications for GetBlock* commands, because they are received numerous times */
            if ((command != WBT_CMD_REQ_GET_BLOCK) && (command != WBT_CMD_REQ_GET_BLOCKINFO))
            {
                WBTi_NotifySystemCallback(work, (WBTCommandType)command, aid, WBT_RESULT_SUCCESS);
            }
        }

    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_InitContext

  Description:  Initializes the WBT structure.

  Arguments:    work: WBT structure
                userdata: Any user-defined value
                callback: System callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_InitContext(WBTContext * work, void *userdata, WBTEventCallback callback)
{
    work->userdata = userdata;
    work->callback = callback;

    /* Initialization of transmission history for block transfers */
    work->last_block_id = (u32)-1;
    work->last_seq_no_1 = -1;
    work->last_seq_no_2 = -1;

    /* Initialize command management */
    work->command = NULL;
    work->command_pool = NULL;
    work->my_command_counter = 0;
    work->last_target_aid = -1;
    work->req_bitmap = 0;
    MI_CpuFill8(&work->system_cmd, 0x00, sizeof(work->system_cmd));
    MI_CpuFill8(work->peer_param, 0x00, sizeof(work->peer_param));

    /* Clear the command list */
    WBT_ResetContext(work, callback);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_ResetContext

  Description:  Reinitializes a WBT structure.

  Arguments:    work: WBT structure
                callback: System callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_ResetContext(WBTContext * work, WBTEventCallback callback)
{
    int     i;

    work->my_aid = -1;
    work->peer_data_packet_size = 0;
    work->my_data_packet_size = 0;

    work->list = NULL;
    work->callback = callback;

    /* Destroy all commands */
    while (work->command)
    {
        WBTCommandList *list = work->command;
        work->command = list->next;
        list->command.command = WBT_CMD_REQ_NONE;
    }

    work->system_cmd.command = WBT_CMD_REQ_NONE;
    work->system_cmd.target_bmp = 0;
    work->system_cmd.peer_bmp = 0;

    for (i = 0; i < 16; ++i)
    {
        work->peer_param[i].recv_token.last_peer_cmd_counter = 0;
    }

}

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommand

  Description:  Issues a command and adds it to the command queue.

  Arguments:    work: WBT structure
                cmd: Structure that stores the command information.
                     Managed in the library until the command is completed.
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_PostCommand(WBTContext *work, WBTCommandList *list, u16 bitmap,
                     WBTEventCallback callback)
{
    PLATFORM_ENTER_CRITICALSECTION();
    {
        if (list)
        {
            /* Add to the end of the list */
            WBTCommandList **pp;
            for (pp = &work->command; *pp; pp = &(*pp)->next)
            {
            }
            *pp = list;
            list->next = NULL;
            list->command.target_bmp = bitmap;
            list->callback = callback;
            /* If new command comes during idle, begin processing here */
            if (work->command == list)
            {
                WBTi_SwitchNextCommand(work);
            }
        }
    }
    PLATFORM_LEAVE_CRITICALSECTION();
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CancelCommand

  Description:  Cancels the currently processing command.

  Arguments:    work: WBT structure
                bitmap: Peer that will cancel the command

  Returns:      The bitmap that indicates the peer that will actually cancel the command.
 *---------------------------------------------------------------------------*/
int WBT_CancelCommand(WBTContext * work, int bitmap)
{
    PLATFORM_ENTER_CRITICALSECTION();
    {
        WBTCommandList *list = WBT_GetCurrentCommandList(work);
        WBTCommand *current = WBT_GetCurrentCommand(work);
        if (current)
        {
            int     aid;
            /* Cancellation notification for all AIDs that are currently processing */
            bitmap &= current->target_bmp;
            for (aid = 0;; ++aid)
            {
                int     bit = (1 << aid);
                if (bit > bitmap)
                {
                    break;
                }
                if ((bit & bitmap) == 0)
                {
                    bitmap &= ~bit;
                }
                /* New specifications */
                else if (list->callback)
                {
                    current->event = WBT_CMD_CANCEL;
                    current->target_bmp &= ~bit;
                    current->peer_bmp = (u16)bit;
                    list->callback(work->userdata, current);
                }
                /* Old specifications */
                else if (current->callback)
                {
                    current->event = WBT_CMD_CANCEL;
                    current->target_bmp &= ~bit;
                    current->peer_bmp = (u16)bit;
                    (*current->callback) (current);
                }
            }
            /* Destroy the cancelled command (similar to WBTi_NotifyCompletionCallback) */
            if (current->target_bmp == 0)
            {
                WBTCommandList *list = work->command;
                work->command = list->next;
                WBT_AddCommandPool(work, list, 1);
                WBTi_SwitchNextCommand(work);
            }
        }
    }
    PLATFORM_LEAVE_CRITICALSECTION();
    return bitmap;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetBitmapLength

  Description:  Gets the bitmap buffer size needed for the block transfer control.

  Arguments:    work: WBT structure
                length: Maximum block size to be transferred

  Returns:      Size of the necessary bitmap buffer.
 *---------------------------------------------------------------------------*/
int WBT_GetBitmapLength(const WBTContext *work, int length)
{
    int     packet = work->peer_data_packet_size;
    SDK_ASSERT(packet > 0);
    return (int)(sizeof(u32) * MATH_ROUNDUP(((length + packet - 1) / packet), 32));
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetDownloadProgress

  Description:  Gets the block transfer progress status.

  Arguments:    work: WBT structure
                id: Receive block ID
                aid: Recipient's AID
                count: Storage location for the received packet count
                total: Where the total number of packets is stored

  Returns:      None.
                Returns 0 for both current and total if there is not block transfer status.
 *---------------------------------------------------------------------------*/
void WBT_GetDownloadProgress(const WBTContext *work, u32 id, int aid, int *count, int *total)
{
    const WBTCommand *current = WBT_GetCurrentCommand(work);
    if ((current != NULL) &&
        (current->command == WBT_CMD_REQ_GET_BLOCK) && (current->get.block_id == id))
    {
        const WBTPacketBitmap *pkt_bmp = &work->peer_param[aid].pkt_bmp;
        *count = pkt_bmp->count;
        *total = pkt_bmp->total;
    }
    else
    {
        *count = 0;
        *total = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_SetPacketLength

  Description:  Changes the packet size.
                Can only be used with the parent.

  Arguments:    work: WBT structure
                own: Own MP send packet size
                peer: Peer MP send packet size

  Returns:      Returns TRUE if the setting succeeds.
 *---------------------------------------------------------------------------*/
BOOL WBT_SetPacketLength(WBTContext * work, int own, int peer)
{
    BOOL    retval = FALSE;
    WBTCommand *current;

    SDK_ASSERT(own >= WBT_PACKET_SIZE_MIN);
    SDK_ASSERT(peer >= WBT_PACKET_SIZE_MIN);

    current = WBT_GetCurrentCommand(work);
    /* Cannot change packet size while requesting a block transmission */
    if ((current == NULL) ||
        ((current->command != WBT_CMD_REQ_GET_BLOCK) &&
        (current->command != WBT_CMD_REQ_GET_BLOCKINFO)))
    {
        work->my_data_packet_size = (s16)(own - WBT_PACKET_SIZE_MIN);
        work->peer_data_packet_size = (s16)(peer - WBT_PACKET_SIZE_MIN);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_RegisterBlockInfo

  Description:  Newly registers data blocks.

  Arguments:    work: WBT structure
                list: List structure used for registration.
                      Used by the library until deallocated with Unregister.
                id: ID associated with the data block
                userinfo: User-defined information associated with the data block.
                          This pointer's target is only referenced within this function.
                          A NULL can be designated here if unnecessary.
                buffer: Buffer where the block data was stored.
                        When NULL is specified, the WBT_CMD_PREPARE_SEND_DATA callback sends notification from the library if needed.
                                  
                length: Size of the block data.
                        This value must be correctly specified even when NULL is specified for 'buffer'.
                                  

  Returns:      FALSE if 'id' has already been registered or is smaller than WBT_BLOCK_ID_MIN.
 *---------------------------------------------------------------------------*/
BOOL
WBT_RegisterBlockInfo(WBTContext * work, WBTBlockInfoList *list, u32 id,
                      const void *userinfo, const void *buffer, int length)
{
    PLATFORM_ENTER_CRITICALSECTION();
    {
        WBTBlockInfoList **pp;
        for (pp = &work->list; *pp; pp = &((*pp)->next))
        {
            /* Return FALSE and exit if 'id' has already been registered with 'work' */
            if ((*pp)->data_info.id == id)
            {
                OS_TWarning("block_id is registered already.");
                return FALSE;
            }
        }
        *pp = list;
        list->next = NULL;
        list->data_info.id = id;
        list->data_info.block_size = length;
        WBTi_CopySafeMemory(userinfo, list->data_info.user_id, WBT_USER_ID_LEN);
        list->data_ptr = (void *)buffer;
        /* Currently unused members */
        list->permission_bmp = 0;
        list->block_type = (u16)(buffer ? WBT_BLOCK_LIST_TYPE_COMMON : WBT_BLOCK_LIST_TYPE_USER);
    }
    PLATFORM_LEAVE_CRITICALSECTION();
    
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         WBT_UnregisterBlockInfo

  Description:  Deallocates registered data blocks.

  Arguments:    work: WBT structure
                id: ID associated with the data block to be deallocated

  Returns:      Either the deallocated list structure or NULL.
 *---------------------------------------------------------------------------*/
WBTBlockInfoList *WBT_UnregisterBlockInfo(WBTContext * work, u32 id)
{
    WBTBlockInfoList *retval = NULL;
    {
        PLATFORM_ENTER_CRITICALSECTION();
        WBTBlockInfoList **pp;
        for (pp = &work->list; *pp; pp = &(*pp)->next)
        {
            if ((*pp)->data_info.id == id)
            {
                retval = *pp;
                *pp = (*pp)->next;
                break;
            }
        }
        PLATFORM_LEAVE_CRITICALSECTION();
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetRegisteredCount

  Description:  Gets the total number of registered data blocks.

  Arguments:    work: WBT structure

  Returns:      The total number of registered data blocks.
 *---------------------------------------------------------------------------*/
int WBT_GetRegisteredCount(const WBTContext * work)
{
    int     n = 0;
    {
        PLATFORM_ENTER_CRITICALSECTION();
        WBTBlockInfoList *list = work->list;
        for (list = work->list; list; list = list->next)
        {
            ++n;
        }
        PLATFORM_LEAVE_CRITICALSECTION();
    }
    return n;
}


/*---------------------------------------------------------------------------*
  $Log: wbt_context.c,v $
  Revision 1.5  2007/12/06 01:39:01  yosizaki
  Fixed WBT_CancelCommand.

  Revision 1.4  2007/11/22 02:04:46  yosizaki
  Fixes specific to GETBLOCK_DONE sequence.

  Revision 1.3  2007/07/30 08:50:29  yosizaki
  Fix related to GetBlock response.

  Revision 1.2  2007/05/15 03:17:02  yosizaki
  Fix related to response iterations.

  Revision 1.1  2007/04/10 08:19:45  yosizaki
  Initial upload.
s
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
