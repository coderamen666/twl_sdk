/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - libraries
  File:     wbt_api.c

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

/*---------------------------------------------------------------------------*
	Variable Definitions
 *---------------------------------------------------------------------------*/

static BOOL wbt_initialize_flag = FALSE;

static WBTContext wbti_command_work[1];

/* 2-level command queue */
static WBTCommandList cmd_q[2];


/*---------------------------------------------------------------------------*
	Function Definitions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WBT_PrintBTList

  Description:  Displays the block information list with OS_TPrintf.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_PrintBTList(void)
{
    WBTBlockInfoList *list = wbti_command_work->list;
    for (; list != NULL; list = list->next)
    {
        OS_TPrintf("BTList id = %d\n", list->data_info.id);
        OS_TPrintf("  data size %d\n", list->data_info.block_size);
        OS_TPrintf("  uid %s\n", list->data_info.user_id);
        OS_TPrintf("  info ptr = %p\n", &(list->data_info));
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_AidbitmapToAid

  Description:  Converts the AID bitmap to AID (looking only at the least significant bit).

  Arguments:    abmp: AID bitmap

  Returns:      int: AID
 *---------------------------------------------------------------------------*/
int WBT_AidbitmapToAid(WBTAidBitmap abmp)
{
    return abmp ? (int)MATH_CTZ(abmp) : -1;
}

/*---------------------------------------------------------------------------*
  Name:         WBT_InitParent

  Description:  Block transfer initialization function for the parent.

  Arguments:    send_packet_size: Send packet size
                recv_packet_size: Receive packet size
                callback: Callback function that is used when the requests from the other party is generated

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_InitParent(int send_packet_size, int recv_packet_size, WBTCallback callback)
{
    PLATFORM_ENTER_CRITICALSECTION();
    if (!wbt_initialize_flag)
    {
        wbt_initialize_flag = TRUE;
        WBT_InitContext(wbti_command_work, NULL, NULL);
        wbti_command_work->system_cmd.callback = callback;
        /* Initialize the command pool */
        MI_CpuFill8(cmd_q, 0x00, sizeof(cmd_q));
        WBT_AddCommandPool(wbti_command_work, cmd_q, sizeof(cmd_q) / sizeof(*cmd_q));
        WBT_StartParent(wbti_command_work, send_packet_size, recv_packet_size);
    }
    PLATFORM_LEAVE_CRITICALSECTION();
}

/*---------------------------------------------------------------------------*
  Name:         WBT_InitChild

  Description:  Block transfer initialization function for a child.

  Arguments:    callback: Callback function that is used when the requests from the other party is generated

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_InitChild(WBTCallback callback)
{
    PLATFORM_ENTER_CRITICALSECTION();
    if (!wbt_initialize_flag)
    {
        wbt_initialize_flag = TRUE;
        WBT_InitContext(wbti_command_work, NULL, NULL);
        wbti_command_work->system_cmd.callback = callback;
        /* Initialize the command pool */
        MI_CpuFill8(cmd_q, 0x00, sizeof(cmd_q));
        WBT_AddCommandPool(wbti_command_work, cmd_q, sizeof(cmd_q) / sizeof(*cmd_q));
    }
    PLATFORM_LEAVE_CRITICALSECTION();
}

/*---------------------------------------------------------------------------*
  Name:         WBT_End

  Description:  Parent and child common block transfer end function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_End(void)
{
    PLATFORM_ENTER_CRITICALSECTION();
    if (wbt_initialize_flag)
    {
        wbt_initialize_flag = FALSE;
        wbti_command_work->system_cmd.callback = NULL;
        WBT_ResetContext(wbti_command_work, NULL);
    }
    PLATFORM_LEAVE_CRITICALSECTION();
}

/*---------------------------------------------------------------------------*
  Name:         WBT_SetOwnAid

  Description:  Sets AID of self

  Arguments:    aid: AID of self

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_SetOwnAid(int aid)
{
    WBTContext *const work = wbti_command_work;
    if (WBT_GetAid(work) == -1)
    {
        WBT_StartChild(work, aid);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetOwnAid

  Description:  Gets AID of self

  Arguments:    None.

  Returns:      int: AID of self
 *---------------------------------------------------------------------------*/
int WBT_GetOwnAid(void)
{
    const WBTContext *const work = wbti_command_work;
    return WBT_GetAid(work);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CalcPacketbitmapSize

  Description:  Calculates the size of the buffer used for recording packet reception number.
                (For a child, it must be called after synchronizing with the parent.)

  Arguments:    block_size: Receiving block size

  Returns:      int: Buffer size for recording packet reception number (bytes)
 *---------------------------------------------------------------------------*/
int WBT_CalcPacketbitmapSize(int block_size)
{
    return WBT_GetBitmapLength(wbti_command_work, block_size);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetCurrentDownloadProgress

  Description:  Looks at the progress of the current block reception.

  Arguments:    block_id: Block ID of block that is being received
                aid: AID of the send target(s)
                *current_count: Packet count when reception was completed
                *total_count: Total packet count

  Returns:      BOOL: Success or failure.
 *---------------------------------------------------------------------------*/
BOOL WBT_GetCurrentDownloadProgress(u32 block_id, int aid, int *current_count, int *total_count)
{
    WBT_GetDownloadProgress(wbti_command_work, block_id, aid, current_count, total_count);
    return (*total_count != 0);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_SetPacketSize

  Description:  Changes the sending/receiving packet size of the parent (use it to change after calling WBT_InitParent).

  Arguments:    send_packet_size: Send packet size
                recv_packet_size: Receive packet size

  Returns:      BOOL: FALSE: Size change failed.
                      TRUE:  Size change successful.
 *---------------------------------------------------------------------------*/
BOOL WBT_SetPacketSize(int send_packet_size, int recv_packet_size)
{
    return WBT_SetPacketLength(wbti_command_work, send_packet_size, recv_packet_size);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_NumOfRegisteredBlock

  Description:  Returns the number of blocks that have been registered.

  Arguments:    None.

  Returns:      WBTBlockNumEntry: Number of blocks
 *---------------------------------------------------------------------------*/
int WBT_NumOfRegisteredBlock(void)
{
    return WBT_GetRegisteredCount(wbti_command_work);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_MpParentSendHook

  Description:  Creates send data for the parent.

  Arguments:    sendbuf: Send buffer
                send_size: Send buffer size

  Returns:      int: Block transfer send size
 *---------------------------------------------------------------------------*/
int WBT_MpParentSendHook(void *sendbuf, int send_size)
{
    SDK_ASSERT(wbt_initialize_flag);
    return WBT_CallPacketSendHook(wbti_command_work, sendbuf, send_size, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_MpChildSendHook

  Description:  Creates send data for a child.

  Arguments:    sendbuf: Send buffer
                send_size: Send buffer size

  Returns:      int: Block transfer send size
 *---------------------------------------------------------------------------*/
int WBT_MpChildSendHook(void *sendbuf, int send_size)
{
    SDK_ASSERT(wbt_initialize_flag);
    return WBT_CallPacketSendHook(wbti_command_work, sendbuf, send_size, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_MpParentRecvHook

  Description:  Analyzes the receive data of the parent.

  Arguments:    recv_buf: Receive buffer
                recv_size: Receive buffer size
                aid: AID of the recipient

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_MpParentRecvHook(const void *buf, int size, int aid)
{
    SDK_ASSERT(wbt_initialize_flag);
    WBT_CallPacketRecvHook(wbti_command_work, aid, buf, size);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_MpChildRecvHook

  Description:  Parses the receive data for the child.

  Arguments:    recv_buf: Receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WBT_MpChildRecvHook(const void *buf, int size)
{
    SDK_ASSERT(wbt_initialize_flag);
    WBT_CallPacketRecvHook(wbti_command_work, 0, buf, size);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_RegisterBlock

  Description:  Registers the sendable (deliverable) block.

  Arguments:    block_info_list: Structure for registering block information
                block_id: Block ID
                user_id: User-defined information
                data_ptr: Pointer to the data (when it is NULL, the callback is used every time there is a request from the other party. Users can set the data pointer in the callback functions)
                                  
                                  
                data_size: Data size
                permission_bmp: Delivery permission bitmap ("plans" to permit with 0)

  Returns:      BOOL: TRUE: Registration successful.
                      FALSE: block_id is already registered.
 *---------------------------------------------------------------------------*/
BOOL
WBT_RegisterBlock(WBTBlockInfoList *block_info_list, WBTBlockId block_id, const void *user_id,
                  const void *data_ptr, int data_size, WBTAidBitmap permission_bmp)
{
    (void)permission_bmp;
    SDK_ASSERT(wbt_initialize_flag);
    
    // Check block_id
    if ( block_id <= WBT_BLOCK_ID_MIN )
    {
        OS_TWarning("block_id must be bigger than WBT_BLOCK_ID_MIN.");
        return FALSE;
    }
    
    // Check the total number of registered data blocks
    if ( WBT_NumOfRegisteredBlock() >= WBT_NUM_MAX_BLOCK_INFO_ID )
    {
        OS_TWarning("Number of registered data blocks is full.");
        return FALSE;
    }
    return WBT_RegisterBlockInfo(wbti_command_work, block_info_list, block_id, user_id,
                          data_ptr, data_size);
}


/*---------------------------------------------------------------------------*
  Name:         WBT_UnregisterBlock

  Description:  Removes from the block delivery registration.

  Arguments:    block_id: Block ID to stop the delivery

  Returns:      WBTBlockInfoList: Structure for registering block information
 *---------------------------------------------------------------------------*/
WBTBlockInfoList *WBT_UnregisterBlock(WBTBlockId block_id)
{
    SDK_ASSERT(wbt_initialize_flag);
    return WBT_UnregisterBlockInfo(wbti_command_work, block_id);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_RequstSync

  Description:  Synchronizes with the other party (must be called when starting the block transfer).

  Arguments:    target: The other party to synchronize (specify with AID bitmap)
                callback: Callback function that is called after the synchronization

  Returns:      BOOL: FALSE: Previous command is running.
                      TRUE: Command issue successful.
 *---------------------------------------------------------------------------*/
BOOL WBT_RequestSync(WBTAidBitmap target, WBTCallback callback)
{
    WBTCommandList *list = WBT_AllocCommandList(wbti_command_work);
    if (list)
    {
        list->command.callback = callback;
        WBT_CreateCommandSYNC(wbti_command_work, list);
        WBT_PostCommand(wbti_command_work, list, target, NULL);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetBlockInfo

  Description:  Gets the block information.

  Arguments:    target: The other party to synchronize (specify with AID bitmap)
                block_info_no: Block information number
                block_info_table: Pointer table to the buffer that stores the obtained block information
                callback: Callback function that is called after the synchronization

  Returns:      BOOL: FALSE: Previous command is running.
                      TRUE: Command issue successful.
 *---------------------------------------------------------------------------*/
BOOL
WBT_GetBlockInfo(WBTAidBitmap target, int block_info_no, WBTBlockInfoTable *block_info_table,
                 WBTCallback callback)
{
    WBTCommandList *list = WBT_AllocCommandList(wbti_command_work);
    if (list)
    {
        list->command.callback = callback;
        WBT_CreateCommandINFO(wbti_command_work, list, block_info_no, block_info_table);
        WBT_PostCommand(wbti_command_work, list, target, NULL);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_GetBlock

  Description:  Gets the block.
                
  Arguments:    target: The other party to synchronize (specify with AID bitmap)
                block_id: Block ID
                recv_buf_table: Pointer table to the buffer that stores the received block
                recv_size: Received block size
                p_bmp_table: Pointer table to the buffer for recording packet receipt number
                callback: Callback function that is used after the block is obtained

  Returns:      BOOL: FALSE: Previous command is running.
                      TRUE: Command issue successful.
 *---------------------------------------------------------------------------*/
BOOL
WBT_GetBlock(WBTAidBitmap target, WBTBlockId block_id, WBTRecvBufTable *recv_buf_table,
             u32 recv_size, WBTPacketBitmapTable *p_bmp_table, WBTCallback callback)
{
    WBTCommandList *list = WBT_AllocCommandList(wbti_command_work);
    if (list)
    {
        list->command.callback = callback;
        WBT_CreateCommandGET(wbti_command_work, list, block_id, recv_size, recv_buf_table, p_bmp_table);
        WBT_PostCommand(wbti_command_work, list, target, NULL);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_PutUserData

  Description:  Sends data that is smaller than 9 bytes to the other party.
                
  Arguments:    target: The other party (specify with AID bitmap)
                user_data: Pointer to the data you want to send
                size: Data size
                callback: Callback function

  Returns:      BOOL: FALSE: Previous command is running.
                      TRUE: Command issue successful.
 *---------------------------------------------------------------------------*/
BOOL WBT_PutUserData(WBTAidBitmap target, const void *user_data, int size, WBTCallback callback)
{
    WBTCommandList *list = WBT_AllocCommandList(wbti_command_work);
    if (list)
    {
        list->command.callback = callback;
        WBT_CreateCommandMSG(wbti_command_work, list, user_data, (u32)size);
        WBT_PostCommand(wbti_command_work, list, target, NULL);
    }
    return (list != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         WBT_CancelCurrentCommand

  Description:  Cancels the currently issuing WBT command.
                
  Arguments:    target: The other party (specify with AID bitmap)

  Returns:      BOOL: FALSE: No command to cancel
                      TRUE: Cancellation succeeded.
 *---------------------------------------------------------------------------*/
BOOL WBT_CancelCurrentCommand(WBTAidBitmap cancel_target)
{
    SDK_ASSERT(wbt_initialize_flag);
    return (WBT_CancelCommand(wbti_command_work, cancel_target) != 0);
}


/*---------------------------------------------------------------------------*
  $Log: wbt_api.c,v $
  Revision 1.2  2007/04/10 23:50:11  yosizaki
  Enabled WBT_AidbitmapToAid().

  Revision 1.1  2007/04/10 08:19:45  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
