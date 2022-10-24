/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - libraries
  File:     os_vramExclusive.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include        <nitro/os/ARM9/vramExclusive.h>
#include        <nitro/os/common/interrupt.h>
#include        <nitro/os/common/system.h>


/*---------------------------------------------------------------------------*
    Internal Variable Definitions
 *---------------------------------------------------------------------------*/
static u32 OSi_vramExclusive;
static u16 OSi_vramLockId[OS_VRAM_BANK_KINDS];


/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         OsCountZeroBits

  Description:  Counts and returns the number of sequential zeroes in the 32-bit value starting from the top.

  Arguments:    bitmap: The value to evaluate

  Returns:      u32: The number of 0 bits.
                                                        0x80000000 = 0; 0x00000000 = 32.
 *---------------------------------------------------------------------------*/
#include        <nitro/code32.h>
static asm u32
OsCountZeroBits( u32 bitmap )
{
    clz         r0,             r0
        bx              lr
        }
#include        <nitro/codereset.h>


/*---------------------------------------------------------------------------*
  Name:         OSi_InitVramExclusive

  Description:  Initializes VRAM exclusion processes.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
OSi_InitVramExclusive( void )
{
    s32         i;

    OSi_vramExclusive = 0x0000;
    for( i = 0 ; i < OS_VRAM_BANK_KINDS ; i ++ )
    {
        OSi_vramLockId[ i ] = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OSi_TryLockVram

  Description:  Tries the VRAM exclusive lock.

  Arguments:    bank: ID bitmap of VRAM on which exclusive lock is being tried
                lockId: Any ID that can be used as key for locking

  Returns:      BOOL: Returns TRUE if the lock is successful.
 *---------------------------------------------------------------------------*/
BOOL OSi_TryLockVram(u16 bank, u16 lockId)
{
    u32     workMap;
    s32     zeroBits;
    OSIntrMode enabled = OS_DisableInterrupts();

    // Check whether exclusive lock already set with different ID
    workMap = (u32)(bank & OSi_vramExclusive);
    while (TRUE)
    {
        zeroBits = (s32)(31 - OsCountZeroBits(workMap));
        if (zeroBits < 0)
        {
            break;
        }
        workMap &= ~(0x00000001 << zeroBits);
        if (OSi_vramLockId[zeroBits] != lockId)
        {
            (void)OS_RestoreInterrupts(enabled);
            return FALSE;
        }
    }

    // Lock all VRAM with specified ID
    workMap = (u32)(bank & OS_VRAM_BANK_ID_ALL);
    while (TRUE)
    {
        zeroBits = (s32)(31 - OsCountZeroBits(workMap));
        if (zeroBits < 0)
        {
            break;
        }
        workMap &= ~(0x00000001 << zeroBits);
        OSi_vramLockId[zeroBits] = lockId;
        OSi_vramExclusive |= (0x00000001 << zeroBits);
    }

    (void)OS_RestoreInterrupts(enabled);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_UnlockVram

  Description:  Releases VRAM exclusive lock.

  Arguments:    bank: ID bitmap of VRAM on which exclusive lock is being released
                lockId: Arbitrary ID specified when locked

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_UnlockVram(u16 bank, u16 lockId)
{
    u32     workMap;
    s32     zeroBits;
    OSIntrMode enabled = OS_DisableInterrupts();

    workMap = (u32)(bank & OSi_vramExclusive & OS_VRAM_BANK_ID_ALL);
    while (TRUE)
    {
        zeroBits = (s32)(31 - OsCountZeroBits((u32)workMap));
        if (zeroBits < 0)
        {
            break;
        }
        workMap &= ~(0x00000001 << zeroBits);
        if (OSi_vramLockId[zeroBits] == lockId)
        {
            OSi_vramLockId[zeroBits] = 0;
            OSi_vramExclusive &= ~(0x00000001 << zeroBits);
        }
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
