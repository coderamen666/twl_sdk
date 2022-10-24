/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - unregister-1
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
#include "font.h"
#include "text.h"


/*****************************************************************************/
/* Constants */

/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x55)

/* Send/receive data size for testing */
#define DATA_SIZE (1024 * 20)


/*****************************************************************************/
/* Variables */

/* Number of successful exchanges */
static int data_exchange_count = 0;

/* Data buffer for testing */
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];


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

  Arguments:    buf: Data storage target.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CreateData(void *buf)
{
	*(OSTick*)buf = OS_GetTick();
	OS_GetMacAddress((u8*)buf + sizeof(OSTick));
}

/*---------------------------------------------------------------------------*
  Name:         user_callback

  Description:  Data exchange complete callback

  Arguments:    stat: Notification status (normally WXC_STATE_EXCHANGE_DONE)
                arg: Notification argument (receive data buffer)

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static void user_callback(WXCStateCode stat, void *arg)
{
#pragma unused(stat)

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)arg;
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

	SDK_ASSERT(stat == WXC_STATE_EXCHANGE_DONE);
    ++data_exchange_count;

    /* Displays debug receive data */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        mprintf("sum OK 0x%02X %6d %6d sec\n", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));

		/* Determines whether received data is its own or re-sent */
		{
			u8 mac[6];
			const u8 *dst_mac = data + sizeof(OSTick);
			int i;

			OS_GetMacAddress(mac);
			for ( i = 0 ; (i < sizeof(mac)) && (dst_mac[i] == mac[i]) ; ++i )
			{
			}
			/* If it is its own sent data, it regenerates data */
			if(i >= sizeof(mac))
			{
				mprintf("  (own data before %6d sec)\n",
					OS_TicksToSeconds(OS_GetTick() - *(OSTick*)data));
				CreateData(send_data);
			}
			/* If it is data from a new sender, it forwards the data as is */
			else
			{
				mprintf("  (from %02X:%02X:%02X:%02X:%02X:%02X)\n\n",
					dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
				MI_CpuCopy8(data, send_data, sizeof(send_data));
			}
			/*
			 * Pause to unregister data and then register it again.
			 * Even though the current library allows WXC_SetInitialData() to be used for equivalent processing here without pausing wireless features, we are employing the method shown here anyway because this sample shows how to use WXC_UnregisterData().
			 * 
			 * 
			 * 
			 */
			WXC_Stop();
		}
    }
    else
    {
        mprintf("sum NG 0x%02X sum 0x0%02X %6d sec\n", sum, data[DATA_SIZE - 1],
                OS_TicksToSeconds(OS_GetTick()));
    }

    MI_CpuClear32(data, DATA_SIZE);
}

/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC library system callback.

  Arguments:    stat: Notification status
                arg: Notification argument (the meaning changes based on the notification status)

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static void system_callback(WXCStateCode state, void *arg)
{
	switch (state)
	{
	case WXC_STATE_READY:
		/*
		 * Issued from a WXC_Init function call or when WXC_Stop completes.
		 * arg is always NULL.
		 */

		/* If there is currently registered data, it is cancelled here */
		if (data_exchange_count > 0)
		{
			WXC_UnregisterData(GGID_ORG_1);
		}
		/* Newly registers data for the next exchange */
		send_data[DATA_SIZE - 2] = (u8)GGID_ORG_1;
		send_data[DATA_SIZE - 1] = CalcHash(send_data);
		WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
		/* Library wireless startup */
		WXC_Start();
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

	case WXC_STATE_CONNECTED:
		/*
		 * Issued when a new connection is made.
		 * arg is a pointer to the WXCUserInfo structure that shows the connection target.
		 */
		{
			const WXCUserInfo * user = (const WXCUserInfo *)arg;
			OS_TPrintf("connected(%2d):%02X:%02X:%02X:%02X:%02X:%02X\n",
				user->aid,
				user->macAddress[0], user->macAddress[1], user->macAddress[2],
				user->macAddress[3], user->macAddress[4], user->macAddress[5]);
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
    /* Initializes data for the next exchange */
    data_exchange_count = 0;
    CreateData(send_data);
    /* Initializes the internal status of the library */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);
}
