/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - include
  File:     context.h

  Copyright 2006-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef	NITRO_WBT_CONTEXT_H_
#define	NITRO_WBT_CONTEXT_H_


/*===========================================================================*/

#include <nitro/platform.h>
#include <nitro/misc.h>
#include <nitro/math/math.h>
#include <nitro/mi/memory.h>
#include <nitro/wbt.h>


#ifdef	__cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */


/*****************************************************************************
 * Wireless communication format
 *****************************************************************************/

/* REQ_SYNC command argument format structure */
typedef struct WBTPacketRequestSyncFormat
{
    PLATFORM_LE16 peer_packet;
    PLATFORM_LE16 own_packet;
}
PLATFORM_STRUCT_PADDING_FOOTER WBTPacketRequestSyncFormat;

/* RES_SYNC command argument format structure */
typedef struct WBTPacketResponseSyncFormat
{
    PLATFORM_LE16 block_total;
    PLATFORM_LE16 peer_packet;
    PLATFORM_LE16 own_packet;
}
PLATFORM_STRUCT_PADDING_FOOTER WBTPacketResponseSyncFormat;

/* REQ_USERDATA command argument format structure */
typedef struct WBTPacketRequestUserDataFormat
{
    PLATFORM_LE8 length;
    PLATFORM_LE8 buffer[WBT_SIZE_USER_DATA];
}
WBTPacketRequestUserDataFormat;

/* REQ_GETBLOCK_DONE command argument format structure */
typedef struct WBTPacketRequestGetBlockDoneFormat
{
    PLATFORM_LE32 id;
}
WBTPacketRequestGetBlockDoneFormat;

/* RES_GETBLOCK_DONE command argument format structure */
typedef struct WBTPacketResponseGetBlockDoneFormat
{
    PLATFORM_LE32 id;
}
WBTPacketResponseGetBlockDoneFormat;

/* REQ_GETBLOCK command argument format structure */
typedef struct WBTPacketRequestGetBlockFormat
{
    PLATFORM_LE32 id;
    PLATFORM_LE32 index;
}
WBTPacketRequestGetBlockFormat;

/* RES_GETBLOCK command argument format structure */
typedef struct WBTPacketResponseGetBlockFormat
{
    PLATFORM_LE32 id;
    PLATFORM_LE32 index;
}
WBTPacketResponseGetBlockFormat;

/* Packet header format structure */
typedef struct WBTPacketHeaderFormat
{
    PLATFORM_LE8 command;
    PLATFORM_LE16 bitmap;
    PLATFORM_LE8 counter;
}
PLATFORM_STRUCT_PADDING_FOOTER WBTPacketHeaderFormat;

/* Packet format structure */
typedef struct WBTPacketFormat
{
    /* Packet header */
    WBTPacketHeaderFormat header;
    /* Existence of the next argument depends on the command */
    union
    {
        u8      argument[10];
        WBTPacketRequestSyncFormat req_sync;
        WBTPacketResponseSyncFormat res_sync;
        WBTPacketRequestUserDataFormat req_userdata;
        WBTPacketRequestGetBlockDoneFormat req_getblock_done;
        WBTPacketResponseGetBlockDoneFormat res_getblock_done;
        WBTPacketRequestGetBlockFormat req_getblock;
        WBTPacketResponseGetBlockFormat res_getblock;
        u8      for_compiler[10];
    } PLATFORM_STRUCT_PADDING_FOOTER /* unnamed */ ;
    /* Existence of the next variable-length argument depends on the command */
}
PLATFORM_STRUCT_PADDING_FOOTER WBTPacketFormat;

PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketHeaderFormat) == 4);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketRequestSyncFormat) == 4);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketResponseSyncFormat) == 6);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketRequestUserDataFormat) == 10);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketRequestGetBlockDoneFormat) == 4);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketResponseGetBlockDoneFormat) == 4);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketRequestGetBlockFormat) == 8);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketResponseGetBlockFormat) == 8);
PLATFORM_COMPILER_ASSERT(sizeof(WBTPacketFormat) == 14);


/*****************************************************************************
 * Local Structures
 *****************************************************************************/

struct WBTContext;
struct WBTCommandList;

/* Command callback prototype */
typedef void (*WBTEventCallback)(void*, WBTCommand*);

/* Command list structure */
typedef struct WBTCommandList
{
    struct WBTCommandList  *next;
    WBTCommand              command;
    WBTEventCallback        callback;
}
WBTCommandList;


/* Latest receipt status to be maintained for each AID by the parent */
typedef struct WBTRecvToken
{
    u8      token_command;
    u8      token_peer_cmd_counter;
    u8      last_peer_cmd_counter;

    u8      dummy[1];

    /*
     * These can only be replaced by WBT_CMD_REQ_GET_BLOCK* functions.
     * Accessed when there is a response.
     */
    u32     token_block_id;
    s32     token_block_seq_no;
}
WBTRecvToken;

/* The different work variables to be maintained for each AID by the parent */
typedef struct WBTPacketBitmap
{
    s32     length;
    void   *buffer;
    s32     count;
    s32     total;
    u32    *bitmap;
    s32     current;
}
WBTPacketBitmap;

/* Command management structure used by wbt_data.c */
typedef struct WBTContext
{
    /* The processing command list and the available command pool */
    WBTCommandList *command;
    WBTCommandList *command_pool;

    /* Arbitrary user-defined value */
    void               *userdata;
    WBTEventCallback    callback;

    /*
     * System callback buffer
     * This is only used temporarily, but it is maintained in a context because it has 156 bytes.
     */
    WBTCommand system_cmd;

    /* The status of each AID */
    struct
    {
        WBTRecvToken recv_token;
        WBTPacketBitmap pkt_bmp;
    }
    peer_param[16];

    /* Own communication status */
    int     my_aid;                    /* Own AID */
    s16     peer_data_packet_size;     /* Size of peer packet data */
    s16     my_data_packet_size;       /* Size of own packet data */
    WBTBlockInfoList *list;            /* Registered data block list */
    u8      my_command_counter;        /* User command issue counter */
    u8      padding[3];
    int     last_target_aid;

    /* Block number send history */
    u32     last_block_id;
    s32     last_seq_no_1;
    s32     last_seq_no_2;

    /* Bitmap receiving the current request */
    int     req_bitmap;

    /* GetBlockInfo bitmap */
    u32     binfo_bitmap[16][MATH_ROUNDUP(sizeof(WBTBlockInfo), sizeof(u32)) / sizeof(u32)];
}
WBTContext;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WBT_InitContext

  Description:  Initializes the WBT structure.

  Arguments:    work: WBTContext structure
                userdata: Any user-defined value
                callback: The system callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WBT_InitContext(WBTContext *work, void *userdata, WBTEventCallback callback);

/*---------------------------------------------------------------------------*
  Name:         WBT_StartParent

  Description:  Starts the WBT as a parent.

  Arguments:    work: WBTContext structure
                own: Own MP send packet size
                peer: Peer MP send packet size

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void    WBT_StartParent(WBTContext *work, int own, int peer)
{
    SDK_MIN_ASSERT(own, WBT_PACKET_SIZE_MIN);
    SDK_MIN_ASSERT(peer, WBT_PACKET_SIZE_MIN);
    work->my_aid = 0;
    work->my_data_packet_size = (s16)(own - WBT_PACKET_SIZE_MIN);
    work->peer_data_packet_size = (s16)(peer - WBT_PACKET_SIZE_MIN);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_StartChild

  Description:  Starts the WBT as a child.

  Arguments:    work: WBTContext structure
                aid: Own AID

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void    WBT_StartChild(WBTContext *work, int aid)
{
    work->my_data_packet_size = 0;
    work->peer_data_packet_size = 0;
    work->my_aid = aid;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_ResetContext

  Description:  Reinitializes WBT.

  Arguments:    work: WBTContext structure
                callback: System callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WBT_ResetContext(WBTContext *work, WBTEventCallback callback);

/*---------------------------------------------------------------------------*
  Name:         WBT_CallPacketSendHook

  Description:  Hook function for generating the send packet data.

  Arguments:    work: WBTContext structure
                buffer: Buffer to store the data
                length: Buffer size

  Returns:      The generated packet size.
 *---------------------------------------------------------------------------*/
int     WBT_CallPacketSendHook(WBTContext *work, void *buffer, int length, BOOL is_parent);

/*---------------------------------------------------------------------------*
  Name:         WBT_CallPacketRecvHook

  Description:  Parses the receive packet data.

  Arguments:    work: WBTContext structure
                aid: Data sender's AID
                buffer: Received data buffer
                length: Length of the received data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WBT_CallPacketRecvHook(WBTContext *work, int aid, const void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         WBT_GetUserData

  Description:  Gets the user-defined value associated with the context.

  Arguments:    work: WBTContext structure

  Returns:      Either the currently processing command or NULL.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void   *WBT_GetUserData(const WBTContext *work)
{
    return work->userdata;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetAid

  Description:  Gets the specified AID value.

  Arguments:    work: WBTContext structure

  Returns:      The set AID value.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBT_GetAid(const WBTContext *work)
{
    return work->my_aid;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetOwnPacketLength

  Description:  Gets the current own send packet size.

  Arguments:    work: WBTContext structure

  Returns:      The current own send packet size.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBT_GetOwnPacketLength(const WBTContext *work)
{
    return work->my_data_packet_size;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetPeerPacketLength

  Description:  Gets the current peer send packet size.

  Arguments:    work: WBTContext structure

  Returns:      The current peer send packet size.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBT_GetPeerPacketLength(const WBTContext *work)
{
    return work->peer_data_packet_size;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetParentPacketLength

  Description:  Gets the current parent send packet size.

  Arguments:    work: WBTContext structure

  Returns:      The current parent send packet size.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
int WBT_GetParentPacketLength(const WBTContext *work)
{
    return (work->my_aid == 0) ? work->my_data_packet_size : work->peer_data_packet_size;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetRegisteredCount

  Description:  Gets the total number of registered data blocks.

  Arguments:    work: WBTContext structure

  Returns:      The total number of registered data blocks.
 *---------------------------------------------------------------------------*/
int     WBT_GetRegisteredCount(const WBTContext * work);

/*---------------------------------------------------------------------------*
  Name:         WBT_GetCurrentCommandList

  Description:  Gets the currently processing command.

  Arguments:    work: WBTContext structure

  Returns:      Either the currently processing command or NULL.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
WBTCommandList *WBT_GetCurrentCommandList(const WBTContext *work)
{
    return work->command;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetCurrentCommand

  Description:  Gets the currently processing command.

  Arguments:    work: WBTContext structure

  Returns:      Either the currently processing command or NULL.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
WBTCommand *WBT_GetCurrentCommand(const WBTContext *work)
{
    WBTCommandList *list = work->command;
    return list ? &list->command : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetBitmapLength

  Description:  Gets the bitmap buffer size needed for the block transfer control.

  Arguments:    work: WBTContext structure
                length: Maximum block size to be transferred

  Returns:      Size of the necessary bitmap buffer
 *---------------------------------------------------------------------------*/
int WBT_GetBitmapLength(const WBTContext *work, int length);

/*---------------------------------------------------------------------------*
  Name:         WBT_AddCommandPool

  Description:  Adds a new list to the command pool.

  Arguments:    work: WBTContext structure
                list: Array of command list structures
                count: Number of array elements

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void    WBT_AddCommandPool(WBTContext *work, WBTCommandList *list, int count)
{
    while (--count >= 0)
    {
        list->next = work->command_pool;
        work->command_pool = list++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_AllocCommandList

  Description:  Newly allocates one list from the command pool.

  Arguments:    work: WBTContext structure

  Returns:      Either the newly allocated command list or NULL.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
WBTCommandList *WBT_AllocCommandList(WBTContext *work)
{
    WBTCommandList *list = work->command_pool;
    if (list)
    {
        work->command_pool = list->next;
        list->next = NULL;
    }
    return list;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_SetPacketLength

  Description:  Changes the packet size.
                Can only be used with the parent.

  Arguments:    work: WBTContext structure
                own: Own MP send packet size
                peer: Peer MP send packet size

  Returns:      Returns TRUE if the setting succeeds.
 *---------------------------------------------------------------------------*/
BOOL    WBT_SetPacketLength(WBTContext *work, int own, int peer);

/*---------------------------------------------------------------------------*
  Name:         WBT_CreateCommandSYNC

  Description:  Generates the "SYNC" command information.

  Arguments:    work: WBTContext structure
                list: List prepared to store command information

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void WBT_CreateCommandSYNC(WBTContext *work, WBTCommandList *list)
{
    (void)work;
    list->command.command = WBT_CMD_REQ_SYNC;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CreateCommandINFO

  Description:  Sets the "INFO" command information to the list.

  Arguments:    work: WBTContext structure
                list: List prepared to store command information
                index: Index from the start of the registration list indicating the block information to get
                                  
                buffer_table: WBTBlockInfoTable of pointers that store the gotten block information
                                  

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void WBT_CreateCommandINFO(WBTContext *work, WBTCommandList *list,
                           int index, const WBTBlockInfoTable *buffer_table)
{
    WBTGetBlockCallback *arg = &list->command.get;
    arg->block_id = (u32)index;
    arg->recv_data_size = sizeof(WBTBlockInfo);
    {
        int     i;
        for (i = 0; i < 16; ++i)
        {
            arg->pkt_bmp_table.packet_bitmap[i] = work->binfo_bitmap[i];
            arg->recv_buf_table.recv_buf[i] = (u8 *)buffer_table->block_info[i];
        }
    }
    list->command.command = WBT_CMD_REQ_GET_BLOCKINFO;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CreateCommandGET

  Description:  Sets the "GET" command information to the list.

  Arguments:    work: WBTContext structure
                list: List prepared to store command information
                id: Block ID to be obtained
                length: Length of the obtained block data
                buffer_table: Table of WBTRecvBufTable pointers that store the obtained block data
                                  
                bitmap_table: Table of buffers that manage the receive status necessary to control internal block transfers
                                  

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void WBT_CreateCommandGET(WBTContext *work, WBTCommandList * list,
                          u32 id, u32 length,
                          const WBTRecvBufTable *buffer_table,
                          WBTPacketBitmapTable *bitmap_table)
{
    WBTGetBlockCallback *arg = &list->command.get;
    (void)work;
    arg->block_id = id;
    arg->recv_data_size = length;
    arg->recv_buf_table = *buffer_table;
    arg->pkt_bmp_table = *bitmap_table;
    list->command.command = WBT_CMD_REQ_GET_BLOCK;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CreateCommandMSG

  Description:  Sets the "MSG" command information to the list.

  Arguments:    work: WBTContext structure
                list: List prepared to store command information
                buffer: Buffer where the send data was stored
                        The buffer's content is only referenced in this function.
                length: Send data length
                        Must be less than WBT_SIZE_USER_DATA.

  Returns:      None.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
void    WBT_CreateCommandMSG(WBTContext *work, WBTCommandList *list,
                             const void *buffer, u32 length)
{
    WBTRecvUserDataCallback *arg = &list->command.user_data;
    (void)work;
    SDK_MINMAX_ASSERT(length, 0, WBT_SIZE_USER_DATA);
    arg->size = (u8)length;
    MI_CpuCopy8(buffer, arg->data, length);
    list->command.command = WBT_CMD_REQ_USER_DATA;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommand

  Description:  Issues a command and adds it to the command queue.

  Arguments:    work: WBTContext structure
                list: Structure where the command information was stored.
                      Managed in the library until the command is completed.
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WBT_PostCommand(WBTContext *work, WBTCommandList *list, u16 bitmap,
                        WBTEventCallback callback);

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommandSYNC

  Description:  Issues the "SYNC" command information.

  Arguments:    context: WBTContext structure
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used

  Returns:      TRUE if there is an available command list and the command issue succeeded.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
BOOL    WBT_PostCommandSYNC(WBTContext *context, int bitmap, WBTEventCallback callback)
{
    WBTCommandList *list = WBT_AllocCommandList(context);
    if (list)
    {
        WBT_CreateCommandSYNC(context, list);
        WBT_PostCommand(context, list, (u16)bitmap, callback);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommandINFO

  Description:  Issues the "INFO" command information.

  Arguments:    context: WBTContext structure
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used
                index: Index from the start of the registration list indicating the block information to get
                                  
                buffer_table: WBTBlockInfoTable of pointers that store the gotten block information
                                  

  Returns:      TRUE if there is an available command list and the command issue succeeded.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
BOOL    WBT_PostCommandINFO(WBTContext *context, int bitmap, WBTEventCallback callback,
                            int index, const WBTBlockInfoTable *buffer_table)
{
    WBTCommandList *list = WBT_AllocCommandList(context);
    if (list)
    {
        WBT_CreateCommandINFO(context, list, index, buffer_table);
        WBT_PostCommand(context, list, (u16)bitmap, callback);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommandGET

  Description:  Issues the "GET" command information.

  Arguments:    context: WBTContext structure
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used
                id: Block ID to be obtained
                length: Length of the obtained block data
                buffer_table: Table of WBTRecvBufTable pointers that store the obtained block data
                                  
                bitmap_table: Table of buffers that manage the receive status necessary to control internal block transfers
                                  

  Returns:      TRUE if there is an available command list and the command issue succeeded.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
BOOL    WBT_PostCommandGET(WBTContext *context, int bitmap, WBTEventCallback callback,
                           u32 id, u32 length, const WBTRecvBufTable *buffer_table,
                           WBTPacketBitmapTable *bitmap_table)
{
    WBTCommandList *list = WBT_AllocCommandList(context);
    if (list)
    {
        WBT_CreateCommandGET(context, list, id, length, buffer_table, bitmap_table);
        WBT_PostCommand(context, list, (u16)bitmap, callback);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_PostCommandMSG

  Description:  Issues the "MSG" command information.

  Arguments:    context: WBTContext structure
                bitmap: AID bitmap corresponding to the command issue
                callback: Command completion callback. NULL if not used
                buffer: Buffer where the send data was stored.
                        The buffer's content is only referenced in this function.
                length: Send data length.
                        Must be less than WBT_SIZE_USER_DATA.

  Returns:      TRUE if there is an available command list and the command issue succeeded.
 *---------------------------------------------------------------------------*/
PLATFORM_ATTRIBUTE_INLINE
BOOL    WBT_PostCommandMSG(WBTContext *context, int bitmap, WBTEventCallback callback,
                           const void *buffer, u32 length)
{
    WBTCommandList *list = WBT_AllocCommandList(context);
    if (list)
    {
        WBT_CreateCommandMSG(context, list, buffer, length);
        WBT_PostCommand(context, list, (u16)bitmap, callback);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CancelCommand

  Description:  Cancels the currently processing command.

  Arguments:    work: WBTContext structure
                bitmap: Peer that will cancel the command

  Returns:      The bitmap that indicates the peer that will actually cancel the command.
 *---------------------------------------------------------------------------*/
int     WBT_CancelCommand(WBTContext * work, int bitmap);

/*---------------------------------------------------------------------------*
  Name:         WBT_GetDownloadProgress

  Description:  Gets the block transfer progress status.

  Arguments:    work: WBTContext structure
                id: Receive block ID
                aid: Recipient's AID
                current: Where the received packets are stored
                total: Where the total number of packets is stored

  Returns:      None.
                Returns 0 for both current and total if there is not block transfer status.
 *---------------------------------------------------------------------------*/
void    WBT_GetDownloadProgress(const WBTContext * work, u32 id, int aid, int *current, int *total);

/*---------------------------------------------------------------------------*
  Name:         WBT_RegisterBlockInfo

  Description:  Newly registers data blocks.

  Arguments:    work: WBTContext structure
                list: List structure used for registration.
                      Used by the library until deallocated with Unregister.
                id: ID associated with the data block
                userinfo: User-defined information associated with the data block.
                          This pointer's target is only referenced in this function.
                          NULL can be designated here if unnecessary.
                buffer: Buffer where the block data was stored.
                        When NULL is specified, the WBT_CMD_PREPARE_SEND_DATA callback sends notification from the library if needed.
                                  
                length: Size of the block data.
                        This value must be correctly specified even when NULL is specified for 'buffer'.
                                  

  Returns:      FALSE if 'id' has already been registered or is smaller than WBT_BLOCK_ID_MIN
 *---------------------------------------------------------------------------*/
BOOL    WBT_RegisterBlockInfo(WBTContext * work, WBTBlockInfoList *list, u32 id,
                              const void *userinfo, const void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         WBT_UnregisterBlockInfo

  Description:  Deallocates registered data blocks.

  Arguments:    work: WBTContext structure
                id: ID associated with the data block to be deallocated

  Returns:      Either the deallocated list structure or NULL.
 *---------------------------------------------------------------------------*/
WBTBlockInfoList *WBT_UnregisterBlockInfo(WBTContext * work, u32 id);


/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* NITRO_WBT_CONTEXT_H_ */

/*---------------------------------------------------------------------------*

  $Log: context.h,v $
  Revision 1.2  2007/08/22 05:22:32  yosizaki
  Fixed dependencies.

  Revision 1.1  2007/04/10 08:20:14  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
