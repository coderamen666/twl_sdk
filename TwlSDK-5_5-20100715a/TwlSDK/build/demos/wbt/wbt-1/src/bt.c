/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - demos - wbt-1
  File:     bt.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8576 $
  $Author: nishimoto_takashi $
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include <nitro/wbt.h>
#include "wh.h"
#include "bt.h"
#include "text.h"

#define NOT_USE_ALLOC

#ifndef	SDK_NO_MESSAGE
#define	__MY_LINE__		__LINE__
#else
#define	__MY_LINE__		0
#endif

/* Reception block control data for child device */
static int request_block_num;
static int block_info_num = 0;
static OSTick start_tick;
static int bt_loop_count = 0;
static int bt_running = 0;

/* Child device receive buffers */
static WBTBlockInfoTable block_info_table;
static WBTRecvBufTable recv_buf_table;
static WBTPacketBitmapTable recv_buf_packet_bmp_table;
static WBTAidBitmap tbmp = 1;          /* Request partner (parent) */

/* User data send buffer for child device */
static u8 user_data[WBT_SIZE_USER_DATA];

#ifdef NOT_USE_ALLOC
WBTBlockInfo block_info[WBT_NUM_OF_AID][NUM_OF_BT_LIST];
u8      recv_buf[WBT_NUM_OF_AID][BT_DATA_SIZE];
u32    
    recv_pkt_bmp_buf[WBT_NUM_OF_AID][WBT_PACKET_BITMAP_SIZE(BT_DATA_SIZE, BT_PARENT_PACKET_SIZE)];
#endif

/* Block data for parent device */
static WBTBlockInfoList bt_list[NUM_OF_BT_LIST];
static u8 bt_data[NUM_OF_BT_LIST][BT_DATA_SIZE];

static u8 user_char_id[NUM_OF_BT_LIST][WBT_USER_ID_LEN] = {
    "BT text information area 0",
    "BT text information area 1",
    "BT text information area 2",
#if 0
    "BT text information area 3",
    "BT text information area 4"
#endif
};


const char *command_str[] = {
    "REQ_NONE",
    "REQ_WAIT",
    "REQ_SYNC",
    "RES_SYNC",
    "REQ_GET_BLOCK",
    "RES_GET_BLOCK",
    "REQ_GET_BLOCKINFO",
    "RES_GET_BLOCKINFO",
    "REQ_GET_BLOCK_DONE",
    "RES_GET_BLOCK_DONE",
    "REQ_USER_DATA",
    "RES_USER_DATA",
    "SYSTEM_CALLBACK",
    "PREPARE_SEND_DATA",
    "REQ_ERROR",
    "RES_ERROR",
    "CANCEL"
};




static int strlen(char *str)
{
    int     i = 0;
    while (1)
    {
        if (*str != '\0')
        {
            str++;
            i++;
        }
        else
        {
            break;
        }
    }
    return i;
}


/* Block registration function for parent device */
void bt_register_blocks(void)
{
    int     i;
    char   *end_string = "This is BlockTransfer test data contents end\n";
    char   *ptr;
    int     offset;

    for (i = 0; i < NUM_OF_BT_LIST; i++)
    {
        offset = BT_DATA_SIZE - (strlen(end_string) + 1);
        *(s32 *)(&(bt_data[i][0])) = offset;
        (void)OS_SPrintf((char *)&(bt_data[i][4]),
                         "This is BlockTransfer test data contents start %d\n", i);

        ptr = (char *)(&(bt_data[i][0]) + offset);
        (void)OS_SPrintf((char *)ptr, "%s", end_string);

        (void)WBT_RegisterBlock(&(bt_list[i]), (u32)(10000 + i) /* IDs of 1000 or less are not allowed */ ,
                                user_char_id[i], &(bt_data[i][0]), BT_DATA_SIZE, 0);
    }
}


/* Block transmission end function for child device */
void bt_stop(void)
{
    bt_running = 0;
    (void)WBT_CancelCurrentCommand(0xffff);
}


/* Block transmission start function for child device */
void bt_start(void)
{
    OSIntrMode enabled;
    static int init_flag = FALSE;
    int     i;

    enabled = OS_DisableInterrupts();

    if (bt_running)
    {
        (void)OS_RestoreInterrupts(enabled);
        return;
    }
    bt_running = 1;

#ifdef NOT_USE_ALLOC

    if (init_flag == FALSE)
    {
        init_flag = TRUE;
        for (i = 0; i < WBT_NUM_OF_AID; i++)
        {
            block_info_table.block_info[i] = &(block_info[i][0]);
            recv_buf_table.recv_buf[i] = &(recv_buf[i][0]);
            recv_buf_packet_bmp_table.packet_bitmap[i] = &(recv_pkt_bmp_buf[i][0]);
        }
    }

#else

    mfprintf(tc[2], "child bt start\n");

    if (init_flag == FALSE)

    {
        init_flag = TRUE;
        /* Initialize child device receive buffers */
        for (i = 0; i < WBT_NUM_OF_AID; i++)
        {
            block_info_table.block_info[i] = NULL;
            recv_buf_table.recv_buf[i] = NULL;
            recv_buf_packet_bmp_table.packet_bitmap[i] = NULL;
        }
    }

    for (i = 0; i < WBT_NUM_OF_AID; i++)
    {
        if (block_info_table.block_info[i] != NULL)
        {
            OS_Free(block_info_table.block_info[i]);
            block_info_table.block_info[i] = NULL;
        }
        if (recv_buf_table.recv_buf[i] != NULL)
        {
            OS_Free(recv_buf_table.recv_buf[i]);
            recv_buf_table.recv_buf[i] = NULL;
        }
        if (recv_buf_packet_bmp_table.packet_bitmap[i] != NULL)
        {
            OS_Free(recv_buf_packet_bmp_table.packet_bitmap[i]);
            recv_buf_packet_bmp_table.packet_bitmap[i] = NULL;
        }
    }
#endif

    (void)OS_RestoreInterrupts(enabled);

    (void)WBT_RequestSync(tbmp,        /* Partner that is requesting Sync (multiple only possible for parent) */
                          bt_callback  /* Callback when ending */
        );
}


/* Callback function for block transmission */
void bt_callback(void *arg)
{

    WBTCommand *uc = (WBTCommand *)arg;
    int     peer_aid = WBT_AidbitmapToAid(uc->peer_bmp);        /* AID of communications partner station */
    // mfprintf(tc[2],"aid = %d\n", peer_aid);

    switch (uc->event)
    {
    case WBT_CMD_RES_SYNC:
        /* WBT_RequestSync end */
        request_block_num = 0;
        block_info_num = uc->sync.num_of_list;  /* Number of blocks held by partner */
        mfprintf(tc[2], "blockinfo num = %d my_packet_size = %d peer_packet_size = %d\n", block_info_num, uc->sync.my_packet_size,      /* Partner's send data size */
                 uc->sync.peer_packet_size      /* Send data size here */
            );
#ifndef NOT_USE_ALLOC
        mfprintf(tc[2], "info buf alloc %d\n", peer_aid);
        /* Block information table initialization */
        block_info_table.block_info[peer_aid] = (WBTBlockInfo *)OS_Alloc(sizeof(WBTBlockInfo));
#endif

        if (uc->target_bmp == 0)       /* Has there been a response from all partner stations? */
        {
            (void)OS_SPrintf((char *)user_data, " %5d\n", bt_loop_count);

            /* Send user data */
            if (FALSE == WBT_PutUserData(tbmp,  /* Partner that sends user data (multiple only possible for parent) */
                                         user_data,     /* User data buffer */
                                         WBT_SIZE_USER_DATA,    /* User data size of 12 or lower */
                                         bt_callback    /* Callback when ending */
                ))
            {
                mfprintf(tc[2], "command invoke error %d\n", __MY_LINE__);
            }
        }
        break;
    case WBT_CMD_RES_USER_DATA:

        if (uc->target_bmp == 0)       /* Has there been a response from all partner stations? */
        {
            /* Block list request */
            if (FALSE == WBT_GetBlockInfo(tbmp, /* Partner that makes block list request (multiple only possible for parent) */
                                          request_block_num /* Block list number */ ,
                                          &block_info_table,    /* Block information table */
                                          bt_callback   /* Callback when ending */
                ))
            {
                mfprintf(tc[2], "command invoke error %d\n", __MY_LINE__);
            }
        }
        break;
    case WBT_CMD_RES_GET_BLOCKINFO:

        /* End WBT_GetBlockInfo */

        mfprintf(tc[2], "blockinfo %d done\n", uc->get.block_id);       /* Obtained block list ID */
        mfprintf(tc[2], " info id = %d\n", block_info_table.block_info[peer_aid]->id);  /* Block ID */
        mfprintf(tc[2], " info block size = %d\n", block_info_table.block_info[peer_aid]->block_size);  /* Block size */
        mfprintf(tc[2], " info = %s\n", block_info_table.block_info[peer_aid]->user_id);        /* Block user definition information */

#ifndef NOT_USE_ALLOC
        /* Initialize receive buffer table */
        recv_buf_table.recv_buf[peer_aid] =
            (u8 *)OS_Alloc((u32)block_info_table.block_info[peer_aid]->block_size);
        mfprintf(tc[2], "recv buf alloc %d\n", peer_aid);

        /* Initialize buffer table for registration of packet reception number */
        recv_buf_packet_bmp_table.packet_bitmap[peer_aid] =
            (u32 *)
            OS_Alloc((u32)
                     WBT_CalcPacketbitmapSize(block_info_table.block_info[peer_aid]->block_size));

        mfprintf(tc[2], "recv pkt bmp size = %d\n",
                 WBT_CalcPacketbitmapSize(block_info_table.block_info[peer_aid]->block_size));
#endif

        if (uc->target_bmp == 0)       /* Has there been a response from all partner stations? */
        {

            /* Block reception request */
            if (FALSE == WBT_GetBlock(tbmp,     /* Partner that makes the block reception request (multiple only possible for parent) */
                                      block_info_table.block_info[peer_aid]->id /* Block ID */ ,
                                      &recv_buf_table,  /* Receive buffer table */
                                      (u32)block_info_table.block_info[peer_aid]->block_size,   /* Block size */
                                      &recv_buf_packet_bmp_table,       /* Buffer table for registration of packet reception number */
                                      bt_callback       /* Callback when ending */
                ))
            {
                mfprintf(tc[2], "command invoke error %d\n", __MY_LINE__);
            }
            else
            {
                start_tick = OS_GetTick();      /* Start time measurement */
            }

        }

        break;
    case WBT_CMD_RES_GET_BLOCK:
        /* End WBT_GetBlock */

        mfprintf(tc[2], "get block %d done\n", uc->get.block_id);       /* Received block ID */
        mfprintf(tc[2], " time %d msec\n", OS_TicksToMilliSeconds(OS_GetTick() - start_tick));

        mfprintf(tc[2], " %s\n", &(recv_buf_table.recv_buf[peer_aid][4]));      /* Received block contents */
        {
            u32     offset;
            offset = *(u32 *)&(recv_buf_table.recv_buf[peer_aid][0]);
            mfprintf(tc[2], " %s\n", (char *)(&(recv_buf_table.recv_buf[peer_aid][offset])));
        }

#ifndef NOT_USE_ALLOC
        /* Reception buffer table deallocation */
        mfprintf(tc[2], "recv buf free %d\n", peer_aid);
        OS_Free(recv_buf_table.recv_buf[peer_aid]);
        recv_buf_table.recv_buf[peer_aid] = NULL;

        /* Deallocation of buffer table for recording of the packet reception number */
        OS_Free(recv_buf_packet_bmp_table.packet_bitmap[peer_aid]);
        recv_buf_packet_bmp_table.packet_bitmap[peer_aid] = NULL;

        OS_Free(block_info_table.block_info[peer_aid]);
        block_info_table.block_info[peer_aid] = NULL;

        {
            mfprintf(tc[2], "info buf alloc %d\n", peer_aid);
            /* Block information table initialization */
            block_info_table.block_info[peer_aid] = (WBTBlockInfo *)OS_Alloc(sizeof(WBTBlockInfo));
        }
#endif

        if (uc->target_bmp == 0)
        {                              /* Has there been a response from all requested partner stations? */

            request_block_num++;

            if (request_block_num < block_info_num)
            {

                /* Block list request */
                if (FALSE == WBT_GetBlockInfo(tbmp, request_block_num,  /* Block list number */
                                              &block_info_table,        /* Block information table */
                                              bt_callback       /* Callback when ending */
                    ))
                {
                    mfprintf(tc[2], "command invoke error %d\n", __MY_LINE__);
                }
            }
            else
            {
                request_block_num = 0;

                bt_loop_count++;
                if (bt_loop_count > 99999)
                {
                    bt_loop_count = 0;
                }

                (void)OS_SPrintf((char *)user_data, " %05d\n", bt_loop_count);

                /* Send user data */
                if (FALSE == WBT_PutUserData(tbmp,      /* Partner that sends user data (multiple only possible for parent) */
                                             user_data, /* User data buffer */
                                             WBT_SIZE_USER_DATA,        /* User data size of 12 or lower */
                                             bt_callback        /* Callback when ending */
                    ))
                {
                    mfprintf(tc[2], "command invoke error %d\n", __MY_LINE__);
                }
            }
        }
        break;
    case WBT_CMD_REQ_NONE:
        mfprintf(tc[2], "WBT user none\n");
        break;
    case WBT_CMD_REQ_USER_DATA:
        mfprintf(tc[2], "get user data = %s\n", uc->user_data.data);
        break;
    case WBT_CMD_REQ_GET_BLOCK_DONE:
        mfprintf(tc[2], "get peer getblockdone %d done from %d\n", uc->blockdone.block_id,
                 peer_aid);
        break;
    case WBT_CMD_REQ_SYNC:
        mfprintf(tc[2], "get peer sync from %d\n", peer_aid);
        break;
    case WBT_CMD_RES_ERROR:
        mfprintf(tc[2], "get req error %d from %d\n", peer_aid, uc->result);
        break;
    case WBT_CMD_REQ_ERROR:
        mfprintf(tc[2], "get res error %d from %d\n", peer_aid, uc->result);
        break;
    case WBT_CMD_CANCEL:
        mfprintf(tc[2], "get canncel [%s] command from %d\n", command_str[uc->command], peer_aid);
        break;
    default:
        mfprintf(tc[2], "WBT callback unknown %d\n", uc->event);
        break;
    }
}
