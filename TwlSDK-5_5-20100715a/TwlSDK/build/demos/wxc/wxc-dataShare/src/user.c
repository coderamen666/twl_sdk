/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
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
#include "disp.h"
#include "gmain.h"
#include "wh.h"

static int data_exchange_count = 0;

/**** Debug Data Buffer ****/
#define DATA_SIZE 1024*20
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x52)
/*---------------------------------------------------------------------------*
    External Variable Definitions
 *---------------------------------------------------------------------------*/
extern int passby_endflg;
extern WMBssDesc bssdesc;
extern WMParentParam parentparam;
extern u8 macAddress[6];

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

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)arg;
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

    ++data_exchange_count;

    /* Displays debug receive data */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        OS_TPrintf("sum OK 0x%02X %6d %6d sec\n", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        OS_TPrintf("%s\n", data);
    }
    else
    {
        OS_TPrintf("sum NG 0x%02X sum 0x0%02X %6d sec", sum, data[DATA_SIZE - 1],
               OS_TicksToSeconds(OS_GetTick()));
    }
    MI_CpuClear32(data, DATA_SIZE);
    
    // End when communication is performed once
    
    if( WXC_IsParentMode() == TRUE)
    {
        const WMParentParam *param;
        
        passby_endflg = 1;
        
        // Get parent information in advance
        param = WXC_GetParentParam();
        MI_CpuCopyFast( param, &parentparam, sizeof(WMParentParam) );        
    }
    else
    {
        const WMBssDesc *bss;
        
        passby_endflg = 2;
        // Get parent's MAC address
        bss = WXC_GetParentBssDesc();
        MI_CpuCopyFast( bss, &bssdesc, sizeof(WMBssDesc) ); 
    }
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
            MI_CpuCopy16( user->macAddress, macAddress, sizeof(u8)*6 ); 
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
    u8      MacAddress[6];
    
    
    /* Initializes the internal status of the library */
    WXC_Init(OS_Alloc(WXC_WORK_SIZE), system_callback, 2);

    /* Set current situation data */
    OS_GetMacAddress(MacAddress);
    (void)OS_SPrintf((char *)send_data, "data %02X:%02X:%02X:%02X:%02X:%02X",
                  MacAddress[0], MacAddress[1], MacAddress[2],
                  MacAddress[3], MacAddress[4], MacAddress[5]);

    send_data[DATA_SIZE - 1] = CalcHash(send_data);

    /* Register own station data */
    /* Target station data receive buffer. Data size is same as own station data size */
    WXC_RegisterCommonData(GGID_ORG_1, user_callback, send_data, DATA_SIZE, recv_data, DATA_SIZE);
    
    /* Library wireless startup */
    WXC_Start();
}
