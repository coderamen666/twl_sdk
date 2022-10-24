/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_dma.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_utility.h"


/*---------------------------------------------------------------------------*/
/* Constants */

static const CARDDmaInterface CARDiDmaUsingFormer =
{
    MIi_CardDmaCopy32,
    MI_StopDma,
};
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static CARDDmaInterface CARDiDmaUsingNew =
{
    MI_Card_A_NDmaCopy,
    MI_StopNDma,
};
#include <twl/ltdmain_end.h>
#endif

/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetDmaInterface

  Description:  Determines appropriately from the DMA channel, and get either the new or the old DMA command interface.
                

  Arguments:    channel: DMA channel
                          The range from 0 to MI_DMA_MAX_NUM; if MI_DMA_USING_NEW is valid, indicates that it is a new DMA.
                          

  Returns:      Either the corresponding new or old DMA interface
 *---------------------------------------------------------------------------*/
const CARDDmaInterface* CARDi_GetDmaInterface(u32 channel)
{
    const CARDDmaInterface *retval = NULL;
    // Use DMA from the valid channel range.
    BOOL    isNew = ((channel & MI_DMA_USING_NEW) != 0);
    channel &= ~MI_DMA_USING_NEW;
    if (channel <= MI_DMA_MAX_NUM)
    {
        // Existing DMA
        if (!isNew)
        {
            retval = &CARDiDmaUsingFormer;
        }
        // New DMA can be used only when running in TWL environments
        else if (!OS_IsRunOnTwl())
        {
            OS_TPanic("NDMA can use only TWL!");
        }
#ifdef SDK_TWL
        else
        {
            retval = &CARDiDmaUsingNew;
        }
#endif
    }
    return retval;
}

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         CARDi_ICInvalidateSmart

  Description:  Selects either the IC_InvalidateAll or IC_InvalidateRange function based on the threshold value.

  Arguments:    buffer: Buffer to be invalidated
                length: Size to be invalidated
                threshold: Threshold value that switches processes

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ICInvalidateSmart(void *buffer, u32 length, u32 threshold)
{
    if (length >= threshold)
    {
        IC_InvalidateAll();
    }
    else
    {
        IC_InvalidateRange((void *)buffer, length);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_DCInvalidateSmart

  Description:  Selects either the DC_FlushAll or DC_InvalidateRange function based on the threshold value.
                If the buffer is not aligned with the cache line, the Store operation also is automatic only for the beginning or end.
                

  Arguments:    buffer: Buffer to be invalidated
                length: Size to be invalidated
                threshold: Threshold value that switches processes

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_DCInvalidateSmart(void *buffer, u32 length, u32 threshold)
{
    if (length >= threshold)
    {
        DC_FlushAll();
    }
    else
    {
        u32     len = length;
        u32     pos = (u32)buffer;
        u32     mod = (pos & (HW_CACHE_LINE_SIZE - 1));
        if (mod)
        {
            pos -= mod;
            DC_StoreRange((void *)(pos), HW_CACHE_LINE_SIZE);
            DC_StoreRange((void *)(pos + length), HW_CACHE_LINE_SIZE);
            length += HW_CACHE_LINE_SIZE;
        }
        DC_InvalidateRange((void *)pos, length);
        DC_WaitWriteBufferEmpty();
    }
}

#endif // SDK_ARM9
