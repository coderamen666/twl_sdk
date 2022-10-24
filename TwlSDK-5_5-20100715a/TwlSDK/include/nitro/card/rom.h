/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     rom.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-05#$
  $Rev: 9518 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_ROM_H_
#define NITRO_CARD_ROM_H_


#include <nitro/card/types.h>

#include <nitro/mi/dma.h>
#include <nitro/mi/exMemory.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomHeader

  Description:  Gets ROM header information from the Game Card that is currently inserted.

  Arguments:    None.

  Returns:      Pointer to a CARDRomHeader structure.
 *---------------------------------------------------------------------------*/
const u8 *CARD_GetRomHeader(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeader

  Description:  Gets ROM header information for the currently running program.

  Arguments:    None.

  Returns:      Pointer to a CARDRomHeader structure.
 *---------------------------------------------------------------------------*/
const CARDRomHeader *CARD_GetOwnRomHeader(void);

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeaderTWL

  Description:  Gets TWL-ROM header information for the currently running program.

  Arguments:    None.

  Returns:      Pointer to a CARDRomHeaderTWL structure.
 *---------------------------------------------------------------------------*/
const CARDRomHeaderTWL *CARD_GetOwnRomHeaderTWL(void);

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionFNT

  Description:  Gets FNT region information for the ROM header.

  Arguments:    None.

  Returns:      Pointer to FNT region information for the ROM header.
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFNT(void)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return &header->fnt;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionFAT

  Description:  Gets FAT region information for the ROM header.

  Arguments:    None.

  Returns:      Pointer to FAT region information for the ROM header.
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFAT(void)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return &header->fat;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionOVT

  Description:  Gets OVT region information for the ROM header.

  Arguments:    None.

  Returns:      Pointer to OVT region information for the ROM header.
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionOVT(MIProcessor target)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return (target == MI_PROCESSOR_ARM9) ? &header->main_ovt : &header->sub_ovt;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockRom

  Description:  Locks the card bus for ROM access.

  Arguments:    lock id

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_LockRom(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockRom

  Description:  Unlocks a locked card bus.

  Arguments:    lock id which is used by CARD_LockRom()

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_UnlockRom(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitRomAsync

  Description:  Determines whether a ROM access function has completed.

  Arguments:    None.

  Returns:      TRUE if the ROM access function has completed.
 *---------------------------------------------------------------------------*/
BOOL    CARD_TryWaitRomAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitRomAsync

  Description:  Waits until a ROM access function has completed.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_WaitRomAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRom

  Description:  basic function of ROM read

  Arguments:    dma        DMA channel to use
                src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Completion callback argument (ignored if not used)
                is_async   If set to asynchronous mode: TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadRom(u32 dma, const void *src, void *dst, u32 len,
                      MIDmaCallback callback, void *arg, BOOL is_async);

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRom Async

  Description:  Asynchronous ROM read

  Arguments:    dma        DMA channel to use
                src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size
                callback   Completion callback (NULL if not used)
                arg        Completion callback argument (ignored if not used)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadRomAsync(u32 dma, const void *src, void *dst, u32 len,
                                  MIDmaCallback callback, void *arg)
{
    CARDi_ReadRom(dma, src, dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRom

  Description:  Synchronous ROM read.

  Arguments:    dma        DMA channel to use
                src        Transfer source offset
                dst        Transfer destination memory address
                len        Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadRom(u32 dma, const void *src, void *dst, u32 len)
{
    CARDi_ReadRom(dma, src, dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushThreshold

  Description:  Gets the threshold value for determining whether to entirely or partially invalidate the cache.

  Arguments:    icache: Pointer to the threshold value for invalidating the instruction cache.
                                  This is ignored if it is NULL.
                dcache: Pointer to the threshold value for invalidating the data cache.
                                  This is ignored if it is NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_GetCacheFlushThreshold(u32 *icache, u32 *dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushThreshold

  Description:  Sets the threshold value for determining whether to entirely or partially invalidate the cache.

  Arguments:    icache: The threshold value for invalidating the instruction cache.
                dcache: The threshold value for invalidating the data cache.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_SetCacheFlushThreshold(u32 icache, u32 dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushFlag

  Description:  Gets the flag that configures cache invalidation to be automatic or manual.

  Arguments:    icache: Pointer to the flag for automatically invalidating the instruction cache.
                                  This is ignored if it is NULL.
                dcache: Pointer to the flag for automatically invalidating the data cache.
                                  This is ignored if it is NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void     CARD_GetCacheFlushFlag(BOOL *icache, BOOL *dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushFlag

  Description:  Configures cache invalidation to be automatic or manual.
                By default, this is FALSE for the instruction cache and TRUE for the data cache.

  Arguments:    icache: TRUE to enable automatic invalidation of the instruction cache.
                dcache: TRUE to enable automatic invalidation of the data cache.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void     CARD_SetCacheFlushFlag(BOOL icache, BOOL dcache);


/*---------------------------------------------------------------------------*
 * Internal Functions
 *---------------------------------------------------------------------------*/

u32     CARDi_ReadRomID(void);
void    CARDi_RefreshRom(u32 warn_mask);
BOOL    CARDi_IsTwlRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetOwnSignature

  Description:  Gets this system's signature data for DS Download Play.

  Arguments:    None.

  Returns:      This system's signature data for DS Download Play.
 *---------------------------------------------------------------------------*/
const u8* CARDi_GetOwnSignature(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetOwnSignature

  Description:  Sets this system's signature data for DS Download Play.
                Call this from a higher-level library when starting without a Game Card.

  Arguments:    Signature data for DS Download Play.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetOwnSignature(const void *signature);


#ifdef __cplusplus
} /* extern "C" */
#endif


/* NITRO_CARD_ROM_H_ */
#endif
