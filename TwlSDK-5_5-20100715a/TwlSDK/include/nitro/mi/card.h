/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     card.h

  Copyright 2004-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_CARD_H_
#define NITRO_MI_CARD_H_


#ifdef __cplusplus
extern "C" {
#endif

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz
#ifdef __DUMMY________
#include <nitro/os.h>
#include <nitro/card.h>


/******************************************************************************
 * CARD access
 *
 *	These are simple CARD API wrappers for internal Lock/Unlock operations.
 *	Future plans call for eliminating them in accordance with the shift to the CARD API.
 *
 ******************************************************************************/


// Use the LockID for the MI-CARD function to allocate/release CARD.
void    MIi_LockCard(void);
void    MIi_UnlockCard(void);

// Read the card ID. (Sync)
static inline u32 MI_ReadCardID(void)
{
    u32     ret;
    MIi_LockCard();
    ret = CARDi_ReadRomID();
    MIi_UnlockCard();
    return ret;
}

// Read card. (Sync)
static inline void MIi_ReadCard(u32 dmaNo, const void *src, void *dst, u32 size)
{
    MIi_LockCard();
    /*
     * This was changed to use interrupts for both synchronous and asynchronous processing to guarantee performance as much as possible in the low-level CARD_ReadRom.
     * 
     * In some cases this function is used while interrupts are prohibited, however, so synchronous MI functions were changed to unconditionally use CPU transfers.
     * 
     * (without multithreading, the efficiency is the same)
     */
    (void)dmaNo;
    CARD_ReadRom((MI_DMA_MAX_NUM + 1), src, dst, size);
    MIi_UnlockCard();
}

// Read card data. (Async)
void    MIi_ReadCardAsync(u32 dmaNo, const void *srcp, void *dstp, u32 size,
                          MIDmaCallback callback, void *arg);

// Confirm completion of card data asynchronous read.
static inline BOOL MIi_TryWaitCard(void)
{
    return CARD_TryWaitRomAsync();
}

// Wait for card data asynchronous read to complete.
static inline void MIi_WaitCard(void)
{
    CARD_WaitRomAsync();
}

#endif
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz


#ifdef __cplusplus
} /* extern "C" */
#endif


/* NITRO_MI_CARD_H_ */
#endif
