/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-2
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
#include "dispfunc.h"


/*****************************************************************************/
/* Variables */

/* If the relay station is in relay mode: TRUE */
BOOL    station_is_relay;

/* Data buffer for exchange */
u8      send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* The original owner of the above send data */
u8      send_data_owner[6];

/* Number of successful exchanges */
static int data_exchange_count = 0;


/*****************************************************************************/
/* Functions */

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
  Name:         user_callback

  Description:  Data exchange complete callback.

  Arguments:    stat: Notification status (normally WXC_STATE_EXCHANGE_DONE)
                arg: Notification argument (receive data buffer)

  Returns:      The calculated value.
 *---------------------------------------------------------------------------*/
static void user_callback(WXCStateCode stat, void *arg)
{
#pragma unused(stat)

    /* Receive data information is given to the argument */
    const WXCBlockDataFormat *recv = (const WXCBlockDataFormat *)arg;

    ++data_exchange_count;

    /*
     * If the relay station is in relay mode, data that should be sent is re-registered with data that was just received
     * 
     */
    if (station_is_relay)
    {
        /* Saves the MAC address of the receiving party */
        const WXCUserInfo *info = WXC_GetUserInfo((u16)((WXC_GetOwnAid() == 0) ? 1 : 0));
        MI_CpuCopy8(info->macAddress, send_data_owner, 6);
        /* Re-registers exchange data */
        MI_CpuCopy8(recv->buffer, send_data, sizeof(send_data));
        MI_CpuClear32(recv->buffer, DATA_SIZE);
        WXC_SetInitialData(GGID_ORG_1, send_data, DATA_SIZE, recv_data, DATA_SIZE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         system_callback

  Description:  WXC library system callback.

  Arguments:    stat: Notification status
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
            void   *system_work = arg;
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

/*---------------------------------------------------------------------------*
  Name:         User_Init

  Description:  User-side initialization operation for WXC library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void User_Init(void)
{
    /* Initializes the WXC library internal status */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* In relay station mode, it can only become a child device */
    WXC_SetChildMode(TRUE);

    /*
     * Register the initial data block.
     * The following is for cases other than when data is received in relay mode.
     * Registered data is not updated.
     */
    OS_GetMacAddress(send_data_owner);
    (void)OS_SPrintf((char *)send_data, "data %02X:%02X:%02X:%02X:%02X:%02X",
                     send_data_owner[0], send_data_owner[1], send_data_owner[2],
                     send_data_owner[3], send_data_owner[4], send_data_owner[5]);
    send_data[DATA_SIZE - 1] = CalcHash(send_data);
    WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);

    /* WXC library wireless startup */
    WXC_Start();
}
