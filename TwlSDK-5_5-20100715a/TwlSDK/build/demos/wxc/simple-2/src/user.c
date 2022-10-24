/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-2
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

/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x53)
#define GGID_ORG_2                            SDK_MAKEGGID_SYSTEM(0x54)

/* Send/receive data size for testing */
#define DATA_SIZE (1024 * 20)


/*****************************************************************************/
/* Variables */

/* Number of successful exchanges */
static int data_exchange_count = 0;

/* Data buffer for testing */
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

static u8 send_data2[DATA_SIZE];
static u8 recv_data2[DATA_SIZE];

extern int passby_endflg;
extern BOOL passby_ggid[MENU_MAX];     // GGID that does chance encounter communications
extern BOOL send_comp_flg;

static int register_count;

static u32 delete_ggid;

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
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

    const u8 *dst_mac = data + sizeof(OSTick);

    ++data_exchange_count;

    /* Displays debug receive data */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        BgPrintf((s16)1, (s16)5, WHITE, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        //BgPrintf((s16)1, (s16)6, WHITE, "%s", data);
        BgPrintf((s16)1, (s16)6, WHITE, "  (from %02X:%02X:%02X:%02X:%02X:%02X)\n\n",
					dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    }
    else
    {
        BgPrintf((s16)1, (s16)5, WHITE, "sum NG 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
    }

	delete_ggid = 0;

    // Set to close after a single communication (unless it's set to send all data)
    if(send_comp_flg == TRUE)
    {
		if( WXC_IsParentMode() == TRUE)
        {
            const WMParentParam *param;
            
            // Get parent (self) information
            param = WXC_GetParentParam();
            /* Get the GGID of the registered data to delete */
	    	delete_ggid = param->ggid;
        }
        else
        {
            const WMBssDesc *bss;
            
            // Get parent (target) MAC address
            bss = WXC_GetParentBssDesc();
            /* Get the GGID of the registered data to delete */
        	delete_ggid = bss->gameInfo.ggid;
        }
        
		register_count--;
        
        if(register_count <= 0)
		{
			if( WXC_IsParentMode() == TRUE)
            {
                passby_endflg = 1;
            }
            else
            {
                passby_endflg = 2;
            }
		}
		else
		{
            // To run WXC_UnregisterData, call WXC_Stop and set the state to WXC_STATE_READY
            WXC_Stop();
        }
	}
	else
	{
        if( WXC_IsParentMode() == TRUE)
        {
            passby_endflg = 1;
        }
        else
        {
            passby_endflg = 2;
        }
    }
    MI_CpuClear32(data, DATA_SIZE);
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
        // If, rather than unregistering data, you only want to temporarily stop exchanging it, you may also use WXC_SetInitialData during a user_callback for the same results
        // 
        if(delete_ggid != 0)
            WXC_UnregisterData(delete_ggid);
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

    case WXC_STATE_EXCHANGE_DONE:
        /*
         * Data exchange complete (not currently issued)
         */
        break;
    }
}


void User_Init(void)
{
	register_count = 0;
	delete_ggid = 0;
	
    /* Initializes the internal status of the library */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /*
     * Register a data block.
     * Currently, the send/receive data size is the same for this and other systems.
     */
    // Register if flag is set
    if(passby_ggid[0] == TRUE)
    {
        CreateData(send_data);
        send_data[DATA_SIZE - 2] = (u8)GGID_ORG_1;
        send_data[DATA_SIZE - 1] = CalcHash(send_data);
        WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
        
        register_count++;
    }
    
    /*
     * Register one more data block as test.
     * Under current specifications, time-sharing is used to select each data block and data is exchanged with the child that has the same GGID.
     * 
     */
    // Register if flag is set
    
    if(passby_ggid[1] == TRUE)
    {
        CreateData(send_data2);
        send_data2[DATA_SIZE - 2] = (u8)GGID_ORG_2;
        send_data2[DATA_SIZE - 1] = CalcHash(send_data2);
        WXC_RegisterCommonData(GGID_ORG_2, user_callback, send_data2, DATA_SIZE, recv_data2, DATA_SIZE);
        
        register_count++;
    }
}
