/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-1
  File:     user.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

#include <nitro/wxc.h>
#include "user.h"
#include "common.h"
#include "dispfunc.h"


/*****************************************************************************/
/* Constants */

/* Send/receive data size for testing */
#define DATA_SIZE (1024 * 20)

/* GGID list compatible with relay stations */
static u32 ggid_list[] =
{
    GGID_ORG_1,
    GGID_ORG_2,
	0,
} ;


/*****************************************************************************/
/* Variables */

/* Number of successful exchanges */
static int data_exchange_count = 0;

/* Send/receive data buffer */
static u8 send_data[2][DATA_SIZE];
static u8 recv_data[2][DATA_SIZE];


/*---------------------------------------------------------------------------*
  Name:         CalcHash

  Description:  Hash calculation for a simple check.

  Arguments:    src: Calculation target buffer

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static u8 CalcHash(const u8 *src)
{
    int     i;
    u8      sum = 0;
    for (i = 0; i < DATA_SIZE - 1; i++)
    {
        sum ^= src[i];
    }
    return sum;
}

/*---------------------------------------------------------------------------*
  Name:         CreateData

  Description:  Creates send data.

  Arguments:    buf: Data storage target

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CreateData(void *buf)
{
	*(OSTick*)buf = OS_GetTick();
	OS_GetMacAddress((u8*)buf + sizeof(OSTick));
}


/*---------------------------------------------------------------------------*
  Name:         user_callback

  Description:  Data exchange complete callback.

  Arguments:    stat: Notification status (normally WXC_STATE_EXCHANGE_DONE)
                arg: Notification argument (receive data buffer)

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static void user_callback(WXCStateCode stat, void *arg)
{
#pragma unused(stat)

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)arg;
    u8     *recv = (u8 *)block->buffer;

    ++data_exchange_count;

    /* Display the received data */
	{
	    const u8 *dst_mac = recv + sizeof(OSTick);
		u8      sum = CalcHash(recv);
		if (sum == recv[DATA_SIZE - 1])
		{
			BgPrintf((s16)1, (s16)5, WHITE, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
					OS_TicksToSeconds(OS_GetTick()));
			BgPrintf((s16)1, (s16)6, WHITE, "  (from %02X:%02X:%02X:%02X:%02X:%02X)\n\n",
						dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
		}
		else
		{
			BgPrintf((s16)1, (s16)5, WHITE, "sum NG 0x%02X %6d %6d sec", sum, data_exchange_count,
					OS_TicksToSeconds(OS_GetTick()));
		}
	}

    /* Configures the next data buffer for exchange */
    {
		/* Manages the buffers independently for each corresponding GGID */
		const u32 ggid = WXC_GetCurrentGgid();
		int index = 0;
		for ( index = 0 ; (ggid_list[index]|WXC_GGID_COMMON_BIT) != ggid ; ++index )
		{
		}
		/*
		 * The data received this time can either be used during the next submission (relay mode) or read and discarded here (distribution mode)
		 * 
		 * 
		 * 
		 */
		if (keep_flg)
		{
			CreateData(send_data[index]);
		}
		else
		{
		    MI_CpuCopy8(recv, send_data[index], sizeof(send_data[index]));
		}
		/*
		 * Re-configure data
		 * This is the same as specifying NULL for the send data in WXC_RegisterCommonData() and then using WXC_AddData() when there is a CONNECTED notification.
		 * 
		 */
		send_data[index][DATA_SIZE - 2] = (u8)ggid;
		send_data[index][DATA_SIZE - 1] = CalcHash(send_data[index]);
		MI_CpuClear32(recv, DATA_SIZE);
		WXC_SetInitialData(ggid, send_data[index], DATA_SIZE, recv_data[index], DATA_SIZE);
    }

}


/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC library system callback.

  Arguments:    stat: Notification status (normally WXC_STATE_EXCHANGE_DONE)
                arg: Notification argument (receive data buffer)

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static void system_callback(WXCStateCode state, void *arg)
{
    switch (state)
    {
    case WXC_STATE_READY:
        /*
         * Issued from a WXC_Init function call.
         * arg is always NULL.
         */
        break;

    case WXC_STATE_ACTIVE:
        /*
         * Issued from a WXC_Start function call.
         * arg is always NULL.
         */
        break;

    case WXC_STATE_ENDING:
        /*
         * Issued from a WXC_End function call.
         * arg is always NULL.
         */
        break;

    case WXC_STATE_END:
        /*
         * Issued upon completion of the shutdown processing run by the WXC_End function.
         * arg is internal work memory allocated by the WXC_Init function.
         * At this point, work memory is deallocated by the user.
         */
        {
            void *system_work = arg;
            OS_Free(system_work);
        }
        break;

    case WXC_STATE_EXCHANGE_DONE:
        /*
         * Data exchange complete (not currently issued)
         */
        break;
    }
}


void User_Init(void)
{
	int	index;

    /* Initializes the internal status of the library */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* In relay station mode, it can only become a child device */
    WXC_SetChildMode(TRUE);

    /* Registers the first data block of each GGID */
    for ( index = 0 ; ggid_list[index] ; ++index )
    {
        if(passby_ggid[index])
        {
            CreateData(send_data[index]);
            send_data[index][DATA_SIZE - 2] = (u8)ggid_list[index];
            send_data[index][DATA_SIZE - 1] = CalcHash(send_data[index]);
            WXC_RegisterCommonData(ggid_list[index], user_callback, send_data[index], DATA_SIZE, recv_data[index], DATA_SIZE);
        }
    }

    /* Library wireless startup */
    WXC_Start();
}
