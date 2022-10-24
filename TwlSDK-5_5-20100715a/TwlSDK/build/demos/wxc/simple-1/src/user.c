/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-1
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
#include "font.h"
#include "user.h"
#include "print.h"

static int data_exchange_count = 0;

/**** Debug Data Buffer ****/
#define DATA_SIZE 1024*20
static u8 send_data[DATA_SIZE];
static u8 recv_data[DATA_SIZE];

/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x51)

extern u16 gScreen[32 * 32];           // Virtual screen
u16 temp_gScreen[32 * 32];           // Virtual screen


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

/* Callback function used when data exchange is completed in the chance encounter communication */
static void user_callback(WXCStateCode stat, void *ptr)
{
#pragma unused(stat)

    const WXCBlockDataFormat * block = (const WXCBlockDataFormat *)ptr;
    u8      sum = 0;
    u8     *data = (u8 *)block->buffer;

    ++data_exchange_count;

    // Move gScreen buffer by one line
    MI_CpuClearFast((void *)temp_gScreen, sizeof(temp_gScreen));
    MI_CpuCopyFast((void *)(&gScreen[(5 * 32)]), (void *)(&temp_gScreen[(7 * 32)]), sizeof(u16)*32*14);
    MI_CpuCopyFast((void *)temp_gScreen, (void *)gScreen, sizeof(gScreen));

    /* Displays debug receive data */
    sum = CalcHash(data);

    if (sum == data[DATA_SIZE - 1])
    {
        PrintString(1, 5, 0x1, "sum OK 0x%02X %6d %6d sec", sum, data_exchange_count,
                OS_TicksToSeconds(OS_GetTick()));
        PrintString(1, 6, 0x1, "%s", data);
    }
    else
    {
        PrintString(1, 5, 0x1, "sum NG 0x%02X sum 0x0%02X %6d sec", sum, data[DATA_SIZE - 1],
                OS_TicksToSeconds(OS_GetTick()));
    }
    MI_CpuClear32(data, DATA_SIZE);
    
    // End when communication is performed once
    (void)WXC_End();
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
