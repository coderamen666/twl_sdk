/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_entropy.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/gx.h>

//----------------------------------------------------------------------

extern vu64 OSi_TickCounter;

/*---------------------------------------------------------------------------*
  Name:         OS_GetLowEntropyData

  Description:  Gets low entropy data array that changes according to the system status.

  Arguments:    buffer: Pointer to the array that holds the data string

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_GetLowEntropyData(u32 buffer[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)])
{
    // RTC work area in the System Work Memory
    const OSSystemWork *work = OS_GetSystemWork();
    const u8 *macAddress =
        (u8 *)((u32)(work->nvramUserInfo) + ((sizeof(NVRAMConfig) + 3) & ~0x00000003));

    buffer[0] = (u32)((GX_GetVCount() << 16) | OS_GetTickLo());
    buffer[1] = (u32)(*(u16 *)(macAddress + 4) << 16) ^ (u32)(OSi_TickCounter);
    buffer[2] = (u32)(OSi_TickCounter >> 32) ^ *(u32 *)macAddress ^ work->vblankCount;
#ifdef reg_G3X_GXSTAT
    buffer[2] ^= reg_G3X_GXSTAT;
#endif
    buffer[3] = *(u32 *)(&work->real_time_clock[0]);
    buffer[4] = *(u32 *)(&work->real_time_clock[4]);
    buffer[5] = (((u32)work->mic_sampling_data) << 16) ^ work->mic_last_address;
    buffer[6] = (u32)((*(u16 *)(&work->touch_panel[0]) << 16) | *(u16 *)(&work->touch_panel[2]));
    buffer[7] = (u32)((work->wm_rssi_pool << 16) | (reg_PAD_KEYINPUT | *(vu16 *)HW_BUTTON_XY_BUF));
}
